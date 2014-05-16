#include <algorithm>
#include <cstdarg>
#include "face_collection.h"


face_collection::face_collection()
{
    this->clt_db = NULL;
    this->filepath = std::string();
    this->err_str = std::string();
}
face_collection::~face_collection()
{
    if (this->clt_db != NULL)
        vedis_close(this->clt_db);
}

void face_collection::save()
{
    if (this->clt_db != NULL)
        vedis_commit(this->clt_db);
}

void face_collection::close()
{
    if (this->clt_db != NULL) {
        vedis_close(this->clt_db);
        this->filepath.clear();
    }
}

bool face_collection::handle_rc(int rc, bool print)
{
    this->err_str.clear();
    if (rc == VEDIS_OK) {
        return true;
    } else {
        const char *buf;
        int len;
        vedis_config(this->clt_db, VEDIS_CONFIG_ERR_LOG, &buf, &len);
        if (len > 0) {
            this->err_str.append(buf);
        }
        if (print) {
            std::cerr << "ERROR: " << this->err_str << " " << rc << std::endl;
        }
        return false;
    }
}

bool face_collection::load(const std::string &filepath)
{
    int rc;
    rc = vedis_open(&this->clt_db, filepath.c_str());
    if (!this->handle_rc(rc, true)) return false;
    this->filepath = filepath;
    return true;
}

void face_collection::my_exec_fmt(const char *fmt, ...)
{
    char buf[512];
    int rc;
    std::va_list args;
    va_start(args, fmt);
    std::vsprintf(buf, fmt, args);
    std::fprintf(stderr, "EXEC: %s ", buf);
    rc = vedis_exec(clt_db, buf, -1);
    if (handle_rc(rc, true)) std::fprintf(stderr, "=>OK\n");
    vedis_commit(clt_db);
    va_end(args);
}

void face_collection::my_kv_store(const char *key, const void *data, vedis_int64 size)
{
    enter_func();
    int rc;
    std::fprintf(stderr, "KV S: %s size:%lld ", key, size);
    rc = vedis_kv_store(clt_db, key, -1, data, size);
    if (handle_rc(rc, true)) std::fprintf(stderr, "=>OK\n");
    vedis_commit(clt_db);
    leave_func();
}

void face_collection::my_kv_delete(const char *key)
{
    int rc;
    std::fprintf(stderr, "KV D: %s ", key);
    rc = vedis_kv_delete(clt_db, key, -1);
    if (handle_rc(rc, true)) std::fprintf(stderr, "=>OK\n");
    vedis_commit(clt_db);
}

void face_collection::my_kv_fetch(const char *key, void *buf, vedis_int64 *len)
{
    enter_func();
    int rc;
    std::fprintf(stderr, "KV F: %s ", key);
    rc = vedis_kv_fetch(clt_db, key, -1, buf, len);
    std::fprintf(stderr, "size:%lld ", *len);
    if (handle_rc(rc, true)) std::fprintf(stderr, "=>OK\n");
    vedis_commit(clt_db);
    leave_func();
}

bool face_collection::create(const std::string &name, const std::string &filepath)
{
    int rc;

    rc = vedis_open(&this->clt_db, filepath.c_str());
    if (!this->handle_rc(rc, true)) return false;
    my_exec_fmt("SET %s '%s'", "collection:name", name.c_str());
    my_exec_fmt("SADD %s x", "collection:idset");
    this->filepath = filepath;
    return true;
}

bool face_collection::new_id(int id, const std::string &name)
{
    if (this->id_exist(id)) return false;
    my_exec_fmt("SET id:%d:name '%s'", id, name.c_str());
    my_exec_fmt("SADD id:%d:faceset x", id);
    my_exec_fmt("SADD %s i%d", "collection:idset", id);

    return true;
}

bool face_collection::delete_id(int id)
{
    int rc, i;
    char key[64];

    //first, check if this id is not exist.
    if (!this->id_exist(id)) return false;

    vedis_value *array, *val;
    my_exec_fmt("SMEMBERS id:%d:faceset", id);
    rc = vedis_exec_result(this->clt_db, &array);
    this->handle_rc(rc, true);

    while ((val = vedis_array_next_elem(array)) != NULL) {
        const char *str = vedis_value_to_string(val, NULL);
        if (str[0] != 'f') continue;
        std::sscanf(str, "f%d", &i);
        std::sprintf(key, "id:%d:face:%d:data", id, i);
        my_kv_delete(key);

        int kp_cnt = 0;
        vedis_value *val;

        my_exec_fmt("GET id:%d:face:%d:siftkp:count", id, i);
        rc = vedis_exec_result(this->clt_db, &val);
        this->handle_rc(rc, true);
        const char *str_ = vedis_value_to_string(val, NULL);
        std::sscanf(str_, "%d", &kp_cnt);
        for (int j = 1; j <= kp_cnt; j++) {
            std::sprintf(key, "id:%d:face:%d:siftkp:%d:x", id, i, j);
            my_kv_delete(key);
            std::sprintf(key, "id:%d:face:%d:siftkp:%d:y", id, i, j);
            my_kv_delete(key);
            std::sprintf(key, "id:%d:face:%d:siftkp:%d:sigma", id, i, j);
            my_kv_delete(key);
            std::sprintf(key, "id:%d:face:%d:siftkp:%d:angle", id, i, j);
            my_kv_delete(key);
            std::sprintf(key, "id:%d:face:%d:siftkp:%d:descr", id, i, j);
            my_kv_delete(key);
        }
        my_exec_fmt("DEL id:%d:face:%d:siftkp:trained", id, i);
        my_exec_fmt("DEL id:%d:face:%d:siftkp:count", id, i);
    }

    my_exec_fmt("DEL id:%d:name id:%d:faceset", id, id, id);
    my_exec_fmt("SREM %s i%d", "collection:idset", id);

    return true;

}

std::vector<int> face_collection::all_id()
{
    int rc;
    vedis_value *array, *val;
    std::vector<int> ids;
    my_exec_fmt("SMEMBERS %s", "collection:idset");
    rc = vedis_exec_result(this->clt_db, &array);
    this->handle_rc(rc, true);
    while ((val = vedis_array_next_elem(array)) != NULL) {
        const char *str = vedis_value_to_string(val, NULL);
        int id_;
        if (str[0] != 'i') continue;
        std::sscanf(str, "i%d", &id_);
        ids.push_back(id_);
    }
    return ids;
}

bool face_collection::add_face(int id, int facenum, const cv::Mat &face)
{
    enter_func();
    int rc;
    char key[64];
    std::vector<unsigned char> buf;

    if (!this->id_exist(id)) return false;
    if (this->id_face_exist(id, facenum)) return false;

    my_exec_fmt("SADD id:%d:faceset f%d", id, facenum);
    std::sprintf(key, "id:%d:face:%d:data", id, facenum);
    this->mat2byte(face, buf);
    my_kv_store(key, buf.data(), buf.size());
    my_exec_fmt("SET id:%d:face:%d:siftkp:trained false", id, facenum);
    leave_func();
    return true;
}

bool face_collection::erase_face(int id, int facenum)
{
    int rc;
    char key[64];

    if (!this->id_face_exist(id, facenum)) return false;

    std::sprintf(key, "id:%d:face:%d:data", id, facenum);
    my_kv_delete(key);
    int kp_cnt = 0;
    vedis_value *val;

    my_exec_fmt("GET id:%d:face:%d:siftkp:count", id, facenum);
    rc = vedis_exec_result(this->clt_db, &val);
    this->handle_rc(rc, true);
    const char *str = vedis_value_to_string(val, NULL);
    std::sscanf(str, "%d", &kp_cnt);
    for (int j = 1; j <= kp_cnt; j++) {
        std::sprintf(key, "id:%d:face:%d:siftkp:%d:x", id, facenum, j);
        my_kv_delete(key);
        std::sprintf(key, "id:%d:face:%d:siftkp:%d:y", id, facenum, j);
        my_kv_delete(key);
        std::sprintf(key, "id:%d:face:%d:siftkp:%d:sigma", id, facenum, j);
        my_kv_delete(key);
        std::sprintf(key, "id:%d:face:%d:siftkp:%d:angle", id, facenum, j);
        my_kv_delete(key);
        std::sprintf(key, "id:%d:face:%d:siftkp:%d:descr", id, facenum, j);
        my_kv_delete(key);
    }
    my_exec_fmt("DEL id:%d:face:%d:siftkp:trained", id, facenum);
    my_exec_fmt("DEL id:%d:face:%d:siftkp:count", id, facenum);
    my_exec_fmt("SREM id:%d:faceset f%d", id, facenum);

    return true;
}

bool face_collection::get_face(int id, int facenum, cv::Mat &face)
{
    enter_func();
    int rc;
    char key[64];
    char *buf;
    vedis_int64 len;
    //first, we should check if the face is exist.
    if (!this->id_face_exist(id, facenum)) return false;

    //second, fetch the face data.
    std::sprintf(key, "id:%d:face:%d:data", id, facenum);
    my_kv_fetch(key, NULL, &len);
    buf = new char[len];
    my_kv_fetch(key, buf, &len);
    this->byte2mat(buf, len, face);
    leave_func();
    return true;
}

std::vector<int> face_collection::all_face(int id)
{
    int rc;
    vedis_value *array, *val;
    std::vector<int> ids;
    if (!this->id_exist(id)) return ids;
    my_exec_fmt("SMEMBERS id:%d:faceset", id);
    rc = vedis_exec_result(this->clt_db, &array);
    this->handle_rc(rc, true);
    while ((val = vedis_array_next_elem(array)) != NULL) {
        const char *str = vedis_value_to_string(val, NULL);
        int id_;
        if (str[0] != 'f') continue;
        std::sscanf(str, "f%d", &id_);
        ids.push_back(id_);
    }
    return ids;
}

int face_collection::max_face(int id)
{
    if (!this->id_exist(id)) return -1;
    std::vector<int> faces = this->all_face(id);
    return *(std::max_element(faces.begin(), faces.end()));
}

std::string face_collection::clt_name()
{
    int rc;
    vedis_value *result;
    my_exec_fmt("GET %s", "collection:name");
    rc = vedis_exec_result(this->clt_db, &result);
    this->handle_rc(rc, true);
    return std::string(vedis_value_to_string(result, NULL));
}

bool face_collection::clt_rename(const std::string &name)
{
    int rc;
    my_exec_fmt("SET %s '%s'", "collection:name", name.c_str());
    return true;
}

std::string face_collection::id_name(int id)
{
    int rc;
    vedis_value *result;
    if (!this->id_exist(id))
        return std::string();
    my_exec_fmt("GET id:%d:name", id);
    rc = vedis_exec_result(this->clt_db, &result);
    this->handle_rc(rc, true);
    const char *name = vedis_value_to_string(result, NULL);
    return std::string(name);
}

bool face_collection::id_rename(int id, const std::string &name)
{
    int rc;
    if (!this->id_exist(id)) return false;
    my_exec_fmt("SET id:%d:name '%s'", name.c_str());
    return true;
}

bool face_collection::is_trained(int id, int facenum)
{
    int rc;
    bool trained = false;
    vedis_value *val;
    if (!this->id_face_exist(id, facenum)) return false;

    my_exec_fmt("GET id:%d:face:%d:siftkp:trained", id, facenum);
    rc = vedis_exec_result(this->clt_db, &val);
    this->handle_rc(rc, true);
    const char *str = vedis_value_to_string(val, NULL);
    if (str[0] == 't') trained = true;
    else trained = false;
    return trained;
}

bool face_collection::set_train_data(int id, int facenum, std::vector<sift_keypoint_descr_t> &kpds)
{
    if (!this->id_face_exist(id, facenum)) return false;
    int rc;
    my_exec_fmt("SET id:%d:face:%d:siftkp:trained true", id, facenum);
    my_exec_fmt("SET id:%d:face:%d:siftkp:count %d", id, facenum, kpds.size());

    char key[64];
    for (int i = 1; i <= kpds.size(); i++) {
        std::sprintf(key, "id:%d:face:%d:siftkp:%d:x", id, facenum, i);
        my_kv_store(key, &(kpds.at(i-1).x), sizeof(float));
        std::sprintf(key, "id:%d:face:%d:siftkp:%d:y", id, facenum, i);
        my_kv_store(key, &(kpds.at(i-1).y), sizeof(float));
        std::sprintf(key, "id:%d:face:%d:siftkp:%d:sigma", id, facenum, i);
        my_kv_store(key, &(kpds.at(i-1).sigma), sizeof(float));
        std::sprintf(key, "id:%d:face:%d:siftkp:%d:angle", id, facenum, i);
        my_kv_store(key, &(kpds.at(i-1).angle), sizeof(float));
        std::sprintf(key, "id:%d:face:%d:siftkp:%d:descr", id, facenum, i);
        my_kv_store(key, &(kpds.at(i-1).descr), sizeof(float)*128);
    }
    return true;

}

bool face_collection::get_train_data(int id, int facenum, std::vector<struct sift_keypoint_descr_t> &kpds)
{
    enter_func();
    if (!this->id_face_exist(id, facenum)) return false;
    if (!this->is_trained(id, facenum)) return false;

    int rc;
    char key[128];
    kpds.clear();
    int kp_cnt = 0;
    vedis_value *val;
    vedis_int64 len;

    my_exec_fmt("GET id:%d:face:%d:siftkp:count", id, facenum);
    rc = vedis_exec_result(this->clt_db, &val);
    this->handle_rc(rc, true);
    const char *str = vedis_value_to_string(val, NULL);
    std::sscanf(str, "%d", &kp_cnt);
    std::cerr << "kp_cnt :: " << kp_cnt << std::endl;

    for (int i = 1; i <= kp_cnt; ++i) {
        struct sift_keypoint_descr_t kpd;
        std::sprintf(key, "id:%d:face:%d:siftkp:%d:x", id, facenum, i);
        len = sizeof(float);
        my_kv_fetch(key, &(kpd.x), &len);
        std::sprintf(key, "id:%d:face:%d:siftkp:%d:y", id, facenum, i);
        my_kv_fetch(key, &(kpd.y), &len);
        std::sprintf(key, "id:%d:face:%d:siftkp:%d:sigma", id, facenum, i);
        my_kv_fetch(key, &(kpd.sigma), &len);
        std::sprintf(key, "id:%d:face:%d:siftkp:%d:angle", id, facenum, i);
        my_kv_fetch(key, &(kpd.angle), &len);
        std::sprintf(key, "id:%d:face:%d:siftkp:%d:descr", id, facenum, i);
        len = sizeof(float) * 128;
        my_kv_fetch(key, kpd.descr, &len);
        kpds.push_back(kpd);
    }
    leave_func();
    return true;
}


std::string face_collection::error_string()
{
    return this->err_str;
}
std::string face_collection::file_path()
{
    return this->filepath;
}

void face_collection::byte2mat(const char *buf, int len, cv::Mat &mat) const
{
    std::vector<unsigned char> imgbuf(buf, buf + len / sizeof(char));
    mat = cv::imdecode(imgbuf, CV_LOAD_IMAGE_ANYDEPTH);
}

void face_collection::mat2byte(const cv::Mat &mat, std::vector<unsigned char> &buf) const
{
    std::vector<int> param;
    param.push_back(FC_IMG_PARAM_ID);
    param.push_back(FC_IMG_PARAM_VALUE);
    cv::imencode(FC_IMG_FORMAT, mat, buf, param);
    //error check
    //...
}

bool face_collection::id_exist(int id)
{
    int rc;
    vedis_value *result;
    my_exec_fmt("SISMEMBER %s i%d", "collection:idset", id);

    rc = vedis_exec_result(this->clt_db, &result);
    this->handle_rc(rc, true);
    return vedis_value_to_bool(result);
}

bool face_collection::id_face_exist(int id, int facenum)
{
    int rc;
    vedis_value *result;
    if (!this->id_exist(id)) return false;
    my_exec_fmt("SISMEMBER id:%d:faceset f%d", id, facenum);

    rc = vedis_exec_result(this->clt_db, &result);
    this->handle_rc(rc, true);
    return vedis_value_to_bool(result);
}

