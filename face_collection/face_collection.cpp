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
            std::cerr << this->err_str;
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

bool face_collection::create(const std::string &name, const std::string &filepath)
{
    int rc;

    rc = vedis_open(&this->clt_db, filepath.c_str());
    if (!this->handle_rc(rc, true)) return false;
    rc = vedis_exec_fmt(this->clt_db, "SET %s '%s'", "collection:name", name.c_str());
    if (!this->handle_rc(rc, true)) return false;
    rc = vedis_exec_fmt(this->clt_db, "SADD %s %d", "collection:idset", 0);
    if (!this->handle_rc(rc, true)) return false;
    this->filepath = filepath;
    return true;
}

bool face_collection::new_id(int id, const std::string &name)
{
    int rc;
    //first, check if this id is already exist.
    if (this->id_exist(id)) return false;

    //second, create basic structure
    rc = vedis_exec_fmt(this->clt_db, "SADD %s %d", "collection:idset", id);

    rc = vedis_exec_fmt(this->clt_db, "SET id:%d:name '%s'", id, name.c_str());

    rc = vedis_exec_fmt(this->clt_db, "SADD id:%d:faceset %d", id, 0);

    //no use:
    //rc = vedis_exec_fmt(this->clt_db, "HSET id:%d:faces %d %s", id, 0, "");

    return true;
}

bool face_collection::delete_id(int id)
{
    int rc;

    //first, check if this id is not exist.
    if (!this->id_exist(id)) return false;

    rc = vedis_exec_fmt(this->clt_db, "DEL id:%d:name id:%d:faces id:%d:faceset", id, id, id);
    rc = vedis_exec_fmt(this->clt_db, "SREM %s %d", "collection:idset", id);

    return true;

}

bool face_collection::add_face(int id, const cv::Mat &face, int *facenum)
{
    char bitmap[sizeof(fcint_t)];
    unsigned i;
    fcint_t idfcnt, fcnt;

    //first, check if there are any empty places.
    if ((idfcnt = this->read_id_facecnt(id)) >= FC_ID_FACE_MAX)
        return false;
    //second, find a empty place.
    this->read_id_face_bitmap(id, bitmap);
    for (i = FC_ID_FACE_MIN; i <= FC_ID_FACE_MAX; ++i) {
        if (this->bitmap_get(bitmap, i) == 0)
            break;
    }
    if (i > FC_ID_FACE_MAX) //impossible
        return false;
    *facenum = i;
    this->write_id_face(id, *facenum, face);
    this->write_id_face_bitmap_1(id, *facenum);
    idfcnt++;
    this->write_id_facecnt(id, idfcnt);
    fcnt = this->read_info_facecnt();
    fcnt++;
    this->write_info_facecnt(fcnt);
    return true;
}

bool face_collection::erase_face(int id, int facenum)
{
    int rc;
    char key[32];
    fcint_t zero = 0;
    fcint_t idfcnt, fcnt;

    if (!this->id_face_exist(id, facenum)) return false;
    std::sprintf(key, "id:%d:face:%d:data", id, facenum);
    rc = unqlite_kv_store(this->clt_db, key, -1, &zero, sizeof(fcint_t));
    this->handle_rc(rc, true);
    this->write_id_face_bitmap_0(id, facenum);

    //update info
    idfcnt = this->read_id_facecnt(id);
    fcnt = this->read_info_facecnt();
    idfcnt--;
    fcnt--;
    this->write_id_facecnt(id, idfcnt);
    this->write_info_facecnt(fcnt);
    return true;
}

bool face_collection::clt_name(std::string &name)
{
    name = this->read_info_name();
    return true;
}

bool face_collection::clt_rename(const std::string &name)
{
    this->write_info_name(name);
    return true;
}

bool face_collection::id_name(int id, std::string &name)
{
    name = this->read_id_name(id);
    return true;
}

bool face_collection::id_rename(int id, const std::string &name)
{
    this->write_id_name(id, name);
    return true;
}

bool face_collection::clt_face_cnt(int *cnt)
{
    *cnt = this->read_info_facecnt();
    return true;
}

bool face_collection::id_face_cnt(int id, int *cnt)
{
    *cnt = this->read_id_facecnt(id);
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

void face_collection::byte2mat(const char *buf, fcint_t len, cv::Mat &mat) const
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



bool face_collection::id_exist(fcint_t id)
{
    int rc;
    vedis_value *result;

    rc = vedis_exec_fmt(this->clt_db, "SISMEMBER %s %d", "collection:idset", id);
    //handle rc...

    vedis_exec_result(this->clt_db, &result);
    return vedis_value_to_bool(result);
}

bool face_collection::id_face_exist(fcint_t id, fcint_t facenum)
{
    int rc;
    vedis_value *result;
    if (!this->id_exist(id)) return false;
    rc = vedis_exec_fmt(this->clt_db, "SISMEMBER id:%d:faceset %d", id, facenum);
    //handle rc...

    vedis_exec_result(this->clt_db, &result);
    return vedis_value_to_bool(result);
}

std::string face_collection::read_id_name(fcint_t id)
{
    int rc;
    vedis_value *result;

    if (!this->id_exist(id))
        return std::string;
    rc = vedis_exec_fmt(this->clt_db, "GET id:%d:name", id);
    //handle rc...

    vedis_exec_result(this->clt_db, &result);

    return std::string(vedis_value_to_string(result));
}

void face_collection::write_id_name(fcint_t id, const std::string &name)
{
    int rc;
    if (!this->id_exist(id))
        return;

    rc = vedis_exec_fmt(this->clt_db, "SET id:%d:name '%s'", id, name.c_str());
    //handle rc...
}

void face_collection::read_id_face(fcint_t id, fcint_t n, cv::Mat &face)
{
    int rc;
    char key[32];
    char *buf;
    unqlite_int64 len;
    //first, we should check if the face number n is exist.
    //...
    //second, fetch the face data.
    std::sprintf(key, "id:%d:face:%d:data", id, n);
    rc = unqlite_kv_fetch(this->clt_db, key, -1, NULL, &len);
    this->handle_rc(rc, true);
    buf = new char[len];
    rc = unqlite_kv_fetch(this->clt_db, key, -1, buf, &len);
    this->handle_rc(rc, true);
    this->byte2mat(buf, len, face);
}

void face_collection::write_id_face(fcint_t id, fcint_t n, const cv::Mat &face)
{
    int rc;
    char key[32];
    std::vector<unsigned char> buf;
    std::sprintf(key, "id:%d:face:%d:data", id, n);
    this->mat2byte(face, buf);
    rc = unqlite_kv_store(this->clt_db, key, -1, buf.data(), buf.size());
    this->handle_rc(rc, true);
    this->write_id_face_bitmap_1(id, n);
}
