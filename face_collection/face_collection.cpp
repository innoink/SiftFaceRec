#include "face_collection.h"

face_collection::face_collection()
{
    this->clt_db = NULL;
    this->err_str = std::string();
}
face_collection::~face_collection()
{
    if (this->clt_db != NULL)
        unqlite_close(this->clt_db);
}

void face_collection::save()
{
    if (this->clt_db != NULL)
        unqlite_commit(this->clt_db);
}

void face_collection::close()
{
    if (this->clt_db != NULL)
        unqlite_close(this->clt_db);
}

bool face_collection::handle_rc(int rc, bool print)
{
    this->err_str.clear();
    if (rc == UNQLITE_OK) {
        return true;
    } else {
        const char *buf;
        int len;
        unqlite_config(this->clt_db, UNQLITE_CONFIG_ERR_LOG, &buf, &len);
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
    rc = unqlite_open(&this->clt_db, filepath.c_str(), UNQLITE_OPEN_READWRITE);
    if (!this->handle_rc(rc, true)) return false;
    return true;
}

bool face_collection::create(const std::string &name, const std::string &filepath)
{
    int rc;
    fcint_t zero;

    rc = unqlite_open(&this->clt_db, filepath.c_str(), UNQLITE_OPEN_CREATE);
    if (!this->handle_rc(rc, true)) return false;
    rc = unqlite_kv_store(this->clt_db, "info:name", -1, name.data(), name.size() + 1);
    if (!this->handle_rc(rc, true)) return false;
    rc = unqlite_kv_store(this->clt_db, "info:idcnt", -1, &zero, sizeof(fcint_t));
    if (!this->handle_rc(rc, true)) return false;
    rc = unqlite_kv_store(this->clt_db, "info:facecnt", -1, &zero, sizeof(fcint_t));
    if (!this->handle_rc(rc, true)) return false;
    return true;
}

bool face_collection::new_id(int id, const std::string &name)
{
    int rc;
    char key[32];
    char bitmap[sizeof(fcint_t)] = {0};
    fcint_t zero = 0;
    //first, check if this id is already exist.
    if (this->id_exist(id)) return false;

    //second, create basic structure
    std::sprintf(key, "id:%d:name", id);
    rc = unqlite_kv_store(this->clt_db, key, -1, name.data(), name.size() + 1);
    if (!this->handle_rc(rc, true)) return false;
    std::sprintf(key, "id:%d:facecnt", id);
    rc = unqlite_kv_store(this->clt_db, key, -1, &zero, sizeof(fcint_t));
    if (!this->handle_rc(rc, true)) return false;
    std::sprintf(key, "id:%d:face:bitmap", id);
    rc = unqlite_kv_store(this->clt_db, key, -1, bitmap, sizeof(fcint_t));
    if (!this->handle_rc(rc, true)) return false;
    //this step can be avoided.
    for (int i = FC_ID_FACE_MIN; i <= FC_ID_FACE_MAX; i++) {
        std::sprintf(key, "id:%d:face:%d:data", id, i);
        rc = unqlite_kv_store(this->clt_db, key, -1, &zero, sizeof(fcint_t));
        if (!this->handle_rc(rc, true)) return false;
    }
    //update info:idcnt
    fcint_t id_cnt = this->read_info_idcnt();
    id_cnt++;
    rc = unqlite_kv_store(this->clt_db, "info:idcnt", -1, &id_cnt, sizeof(id_cnt));
    if (!this->handle_rc(rc, true)) return false;
    return true;
}

bool face_collection::delete_id(int id)
{
    int rc;
    char key[32];
    fcint_t id_fcnt, idcnt, fcnt;
    //first, check if this id is not exist.
    if (!this->id_exist(id)) return false;

    //
    id_fcnt = this->read_id_facecnt(id);
    idcnt = this->read_info_idcnt();
    fcnt = this->read_info_facecnt();
    //
    std::sprintf(key, "id:%d:facecnt", id);
    rc = unqlite_kv_delete(this->clt_db, key, -1);
    if (!this->handle_rc(rc, true)) return false;
    std::sprintf(key, "id:%d:face:bitmap", id);
    rc = unqlite_kv_delete(this->clt_db, key, -1);
    if (!this->handle_rc(rc, true)) return false;
    for (int i = FC_ID_FACE_MIN; i <= FC_ID_FACE_MAX; ++i) {
        std::sprintf(key, "id:%d:face:%d:data", id, i);
        rc = unqlite_kv_delete(this->clt_db, key, -1);
        if (!this->handle_rc(rc, true)) return false;
    }
    //update info:idcnt
    idcnt--;
    this->write_info_idcnt(idcnt);
    //update info:facecnt
    fcnt -= id_fcnt;
    this->write_info_facecnt(fcnt);
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

unsigned face_collection::bitmap_get(const char *bitmap, unsigned n)
{
    n--;
    return bitmap[n >> 3] >> (7 - (n & (8 - 1))) & 1;
}
void face_collection::bitmap_set_0(char *bitmap, unsigned n)
{
    n--;
    bitmap[n >> 3] &= ~(1 << (7 - (n & (8 - 1))));
}
void face_collection::bitmap_set_1(char *bitmap, unsigned n)
{
    n--;
    bitmap[n >> 3] |= 1 << (7 - (n & (8 - 1)));
}

std::string face_collection::read_info_name()
{
    std::string name;
    int rc;
    rc = unqlite_kv_fetch_callback(this->clt_db, "info:name", -1,
         [](const void *d, unsigned int n, void *name)
         {
             (void)n;
             ((std::string *)name)->clear();
             ((std::string *)name)->append((const char *)d);
             return UNQLITE_OK;
         }
         , &name);
    this->handle_rc(rc, true);
    return name;
}

void face_collection::write_info_name(const std::string &name)
{
    int rc;
    rc = unqlite_kv_store(this->clt_db, "info:name", -1, name.data(), name.size() + 1);
    this->handle_rc(rc, true);
}

fcint_t face_collection::read_info_idcnt()
{
    fcint_t cnt;
    int rc;
    unqlite_int64 len;
    rc = unqlite_kv_fetch(this->clt_db, "info:idcnt", -1, &cnt, &len);
    this->handle_rc(rc, true);
    return cnt;
}

void face_collection::write_info_idcnt(fcint_t cnt)
{
    int rc;
    fcint_t idcnt = cnt;
    rc = unqlite_kv_store(this->clt_db, "info:idcnt", -1, &idcnt, sizeof(fcint_t));
    this->handle_rc(rc, true);
}

fcint_t face_collection::read_info_facecnt()
{
    fcint_t cnt;
    int rc;
    unqlite_int64 len;
    rc = unqlite_kv_fetch(this->clt_db, "info:facecnt", -1, &cnt, &len);
    this->handle_rc(rc, true);
    return cnt;
}

void face_collection::write_info_facecnt(fcint_t cnt)
{
    int rc;
    fcint_t idcnt = cnt;
    rc = unqlite_kv_store(this->clt_db, "info:facecnt", -1, &idcnt, sizeof(fcint_t));
    this->handle_rc(rc, true);
}

bool face_collection::id_exist(fcint_t id)
{
    int rc;
    char key[32];
    unqlite_int64 len;

    std::sprintf(key, "id:%d:name", id);
    rc = unqlite_kv_fetch(this->clt_db, key, -1, NULL, &len);
    if (rc != UNQLITE_NOTFOUND) {
        this->handle_rc(rc, true);
        return true;
    } else {
        return false;
    }
}

bool face_collection::id_face_exist(fcint_t id, fcint_t facenum)
{
    char bitmap[sizeof(fcint_t)];

    this->read_id_face_bitmap(id, bitmap);
    return this->bitmap_get(bitmap, facenum) == 1;
}

std::string face_collection::read_id_name(fcint_t id)
{
    std::string name;
    int rc;
    char key[32];
    std::sprintf(key, "id:%d:name", id);//not safe!!
    rc = unqlite_kv_fetch_callback(this->clt_db, key, -1,
         [](const void *d, unsigned int n, void *p)
         {
             (void)n;
             ((std::string *)p)->clear();
             ((std::string *)p)->append((const char *)d);
             return UNQLITE_OK;
         }
         , &name);
    this->handle_rc(rc, true);
    return name;
}

void face_collection::write_id_name(fcint_t id, const std::string &name)
{
    int rc;
    char key[32];
    std::sprintf(key, "id:%d:name", id);//not safe!!
    rc = unqlite_kv_store(this->clt_db, key, -1, name.data(), name.size() + 1);
    this->handle_rc(rc, true);
}

void face_collection::read_id_face_bitmap(fcint_t id, char *bitmap)
{
    int rc;
    char key[32];
    unqlite_int64 len;
    std::sprintf(key, "id:%d:face:bitmap", id);//not safe!!

    rc = unqlite_kv_fetch(this->clt_db, key, -1, bitmap, &len);
    this->handle_rc(rc, true);
}

void face_collection::write_id_face_bitmap_0(fcint_t id, fcint_t n)
{
    int rc;
    char bitmap[sizeof(fcint_t)];
    char key[32];
    unqlite_int64 len;
    std::sprintf(key, "id:%d:face:bitmap", id);
    rc = unqlite_kv_fetch(this->clt_db, key, -1, bitmap, &len);
    this->handle_rc(rc, true);
    this->bitmap_set_0(bitmap, n);
    rc = unqlite_kv_store(this->clt_db, key, -1, bitmap, sizeof(fcint_t));
    this->handle_rc(rc, true);
}

void face_collection::write_id_face_bitmap_1(fcint_t id, fcint_t n)
{
    int rc;
    char bitmap[sizeof(fcint_t)];
    char key[32];
    unqlite_int64 len;
    std::sprintf(key, "id:%d:face:bitmap", id);
    rc = unqlite_kv_fetch(this->clt_db, key, -1, bitmap, &len);
    this->handle_rc(rc, true);
    this->bitmap_set_1(bitmap, n);
    rc = unqlite_kv_store(this->clt_db, key, -1, bitmap, sizeof(fcint_t));
    this->handle_rc(rc, true);
}

fcint_t face_collection::read_id_facecnt(fcint_t id)
{
    int rc;
    char key[32];
    unqlite_int64 len;
    fcint_t facecnt;

    std::sprintf(key, "id:%d:facecnt", id);
    rc = unqlite_kv_fetch(this->clt_db, key, -1, &facecnt, &len);
    this->handle_rc(rc, true);
    return facecnt;
}

void face_collection::write_id_facecnt(fcint_t id, fcint_t facecnt)
{
    int rc;
    char key[32];
    fcint_t cnt = facecnt;

    std::sprintf(key, "id:%d:facecnt", id);
    rc = unqlite_kv_store(this->clt_db, key, -1, &cnt, sizeof(fcint_t));
    this->handle_rc(rc, true);
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
