#include "face_collection.h"

unsigned char face_collection::zero_buffer[128];

face_collection::face_collection()
{
    this->loaded = false;
    this->modified = false;
    this->fc_fp = NULL;
    this->index_bitmap = new unsigned char[FC_INDEX_BITMAP_LEN];
    this->data_bitmap = new unsigned char[FC_DATA_BITMAP_LEN];
}

face_collection::~face_collection()
{
    delete[] this->index_bitmap;
    delete[] this->data_bitmap;
}

bool face_collection::load(const std::string &filepath)
{
    this->fc_fp = std::fopen(filepath.c_str(), "rb+");
    if (this->fc_fp == NULL) {
        return false;
    }
    this->path = filepath;
    this->load_info();
    this->name = std::string(this->info.name);
    return true;
}

uint32_t face_collection::read_id_bitmap(uint32_t n)
{
    // 0 <= n <= 255
    return this->id_bitmap[n >> 3] >> (7 - (n & (8 - 1))) & 1;
}
void face_collection::write_id_bitmap_0(uint32_t n)
{
    this->id_bitmap[n >> 3] &= ~(1 << (7 - (n & (8 - 1))));
}
void face_collection::write_id_bitmap_1(uint32_t n)
{
    this->id_bitmap[n >> 3] |= 1 << (7 - (n & (8 - 1)));
}
uint32_t face_collection::read_index_bitmap(uint32_t n)
{
    // 0 <= n <= 65535
    return this->index_bitmap[n >> 3] >> (7 - (n & (8 - 1))) & 1;
}
void face_collection::write_index_bitmap_0(uint32_t n)
{
    this->index_bitmap[n >> 3] &= ~(1 << (7 - (n & (8 - 1))));
}
void face_collection::write_index_bitmap_1(uint32_t n)
{
    this->index_bitmap[n >> 3] |= 1 << (7 - (n & (8 - 1)));
}
uint32_t face_collection::read_data_bitmap(uint32_t n)
{
    // 0 <= n <= 65535
    return this->data_bitmap[n >> 3] >> (7 - (n & (8 - 1))) & 1;
}
void face_collection::write_data_bitmap_0(uint32_t n)
{
    this->data_bitmap[n >> 3] &= ~(1 << (7 - (n & (8 - 1))));
}
void face_collection::write_data_bitmap_1(uint32_t n)
{
    this->data_bitmap[n >> 3] |= 1 << (7 - (n & (8 - 1)));
}

bool face_collection::load_info()
{
    uint32_t magic;
    std::fread(&magic, sizeof(magic), 1, this->fc_fp);
    if (magic != FC_MAGIC_BITS)
        return false;
    std::fread(this->info.name, FC_NAME_LEN, 1, this->fc_fp);
    std::fread(&this->info.face_width, sizeof(this->info.face_width), 1, this->fc_fp);
    std::fread(&this->info.face_height, sizeof(this->info.face_height), 1, this->fc_fp);
    std::fread(&this->info.id_cnt, sizeof(this->info.id_cnt), 1, this->fc_fp);
    std::fread(&this->info.face_cnt, sizeof(this->info.face_cnt), 1, this->fc_fp);
    //load id bitmap
    std::fread(this->id_bitmap, FC_ID_BITMAP_LEN, 1, this->fc_fp);
    this->load_id_info();
    //load index bitmap
    std::fseek(this->fc_fp, FC_INDEX_BITMAP_START, SEEK_SET);
    std::fread(this->index_bitmap, FC_INDEX_BITMAP_LEN, 1, this->fc_fp);
    //load data bitmap
    std::fseek(this->fc_fp, FC_DATA_BITMAP_START, SEEK_SET);
    std::fread(this->data_bitmap, FC_DATA_BITMAP_LEN, 1, this->fc_fp);
    this->loaded = true;
    return true;
}

bool face_collection::load_id_info()
{
    uint8_t i;
    uint8_t id;
    struct fc_id_info_t id_info;
    for (i = FC_ID_MIN - 1; i < FC_ID_MAX - 1; ++i) {
        if (this->read_id_bitmap(i)) {
            id = i + 1;
            std::fseek(this->fc_fp, FC_ID_INFO_START + i * FC_ID_INFO_LEN, SEEK_SET);
            std::fread(id_info.name, FC_ID_NAME_LEN, 1, this->fc_fp);
            std::fread(&id_info.face_cnt, sizeof(id_info.face_cnt), 1, this->fc_fp);
            std::fread(&id_info.start_blk, sizeof(id_info.start_blk), 1, this->fc_fp);
            this->info.id_info[id] = id_info;
        }
    }
}
bool face_collection::is_loaded() const
{
    return this->loaded;
}
void face_collection::clear_info()
{
    memset(this->info.name, 0, FC_ID_NAME_LEN);
    this->info.face_width = 0;
    this->info.face_height = 0;
    this->info.id_cnt = 0;
    this->info.face_cnt = 0;
    this->info.id_info.clear();
}

bool face_collection::create_empty_collection()
{
    unsigned i;
    const uint32_t magic = FC_MAGIC_BITS;
    std::rewind(this->fc_fp);
    std::fwrite(&magic, FC_MAGIC_BITS_LEN, 1, this->fc_fp);
    std::fwrite(this->info.name, FC_NAME_LEN, 1, this->fc_fp);
    std::fwrite(&this->info.face_width, FC_FACE_WIDTH_LEN, 1, this->fc_fp);
    std::fwrite(&this->info.face_height, FC_FACE_HEIGHT_LEN, 1, this->fc_fp);
    std::fwrite(this->zero_buffer, FC_ID_COUNT_LEN, 1, this->fc_fp);
    std::fwrite(this->zero_buffer, FC_FACE_COUNT_LEN, 1, this->fc_fp);
    std::fwrite(this->zero_buffer, FC_ID_BITMAP_LEN, 1, this->fc_fp);
    for (i = 0; i < 255; i++) {
        std::fwrite(this->zero_buffer, FC_ID_INFO_LEN, 1, this->fc_fp);
    }
    for (i = 0; i < FC_SECTION_UNUSED_LEN; i++) {
        std::fwrite(this->zero_buffer, 1, 1, this->fc_fp);
    }
    for (i = 0; i < FC_SECTION_INDEX_LEN / 128; i++) {
        std::fwrite(this->zero_buffer, 128, 1, this->fc_fp);
    }

}

bool face_collection::create(const std::string &filepath, const std::string &name, int face_width, int face_height)
{
    std::FILE *fp;
    this->save(std::string());
    fp = std::fopen(filepath.c_str(), "wb+");
    if (fp = NULL) {
        return false;
    }
    this->fc_fp = fp;
    this->clear_info();
    memcpy(this->info.name, name.c_str(), name.length());
    this->info.face_width = face_width;
    this->info.face_height = face_height;
    this->create_empty_collection();
    return true;
}

bool face_collection::save(const std::string &filepath)
{
    if (this->fc_fp != NULL) {
        std::fclose(this->fc_fp);
        this->fc_fp = NULL;
    }
    return true;
}

bool face_collection::new_id(int id, const std::string &name)
{
    struct fc_id_info_t new_id_info;
    uint8_t fc_id;
    if (id < FC_ID_MIN || id > FC_ID_MAX) return false;
    if (this->loaded && this->info.id_info.count(fc_id) == 0) {
        strcpy(new_id_info.name, name.c_str());
        new_id_info.face_cnt = 0;
        new_id_info.start_blk = 0;
        this->info.id_info[fc_id] = new_id_info;
        this->write_id_bitmap_1(fc_id - 1);
        std::fseek(this->fc_fp, FC_ID_INFO_OFFSET(fc_id), SEEK_SET);
        std::fwrite(new_id_info.name, FC_ID_NAME_LEN, 1, this->fc_fp);
        std::fwrite(&new_id_info.face_cnt, sizeof(new_id_info.face_cnt), 1, this->fc_fp);
        std::fwrite(&new_id_info.start_blk, sizeof(new_id_info.start_blk), 1, this->fc_fp);
        return true;
    } else {
        return false;
    }
}

bool face_collection::get(int id, int facenum, cv::Mat &faceimg)
{
    unsigned char index_blk_buf[FC_INDEX_BLK_SIZE];
    unsigned char *face_buf;
    if (id < FC_ID_MIN || id > FC_ID_MAX) return false;
    if (facenum < FC_ID_FACE_COUNT_MIN || id > FC_ID_FACE_COUNT_MAX) return false;
    if (facenum == 0) return true;

    if (this->loaded && this->info.id_info.count(id) == 1) {
        if (this->info.id_info.at(id).face_cnt < facenum)
            return false;
        uint16_t blk, next_blk;
        next_blk = blk = this->info.id_info.at(id).start_blk;
        for (uint8_t i = 1; i < facenum; i++) {
            blk = next_blk;
            std::fseek(this->fc_fp, FC_INDEX_BLK_OFFSET(blk), SEEK_SET);
            std::fread(index_blk_buf, FC_INDEX_BLK_SIZE, 1, this->fc_fp);
            next_blk = *((uint16_t *)(index_blk_buf + FC_INDEX_BLK_SIZE - sizeof(next_blk)));
        }

        uint16_t face_len, face_start_blk, face_next_blk = 0, face_blk_cnt, face_offset = 0;
        face_len = *((uint16_t *)index_blk_buf);
        face_start_blk = *((uint16_t *)(index_blk_buf + sizeof(face_len)));
        face_blk_cnt = FC_FACE_BLK_COUNT(face_len);
        face_buf = new unsigned char[FC_FACE_LEN_MAX];
        for (uint16_t i = 0; i < face_blk_cnt; i++) {
            std::fread(face_buf + face_offset, FC_DATA_BLK_SIZE, 1, this->fc_fp);
            if (i == face_blk_cnt - 1) {
                face_offset = face_len;
            } else {
                face_offset += (FC_DATA_BLK_SIZE - FC_DATA_BLK_ADDR_LEN);
                face_next_blk = *((uint16_t *)(face_buf + face_offset));
            }
            std::fseek(this->fc_fp, FC_DATA_BLK_OFFSET(face_next_blk), SEEK_SET);
        }
        this->byte2mat(face_buf, face_len, faceimg);
    } else {
        return false;
    }
}

void face_collection::byte2mat(const unsigned char *buf, uint16_t len, cv::Mat &mat) const
{
    std::vector<unsigned char> imgbuf(buf, buf + len / sizeof(unsigned char));
    mat = cv::imdecode(imgbuf, CV_LOAD_IMAGE_ANYDEPTH);
}

void face_collection::mat2byte(const cv::Mat &mat, unsigned char *buf, uint16_t *len) const
{
    std::vector<unsigned char> imgbuf;
    std::vector<int> param;
    param.push_back(FC_IMG_PARAM_ID);
    param.push_back(FC_IMG_PARAM_VALUE);
    cv::imencode(FC_IMG_FORMAT, mat, imgbuf, param);
    //error
    if (imgbuf.size() <= 0 || imgbuf.size() > FC_FACE_LEN_MAX) return;

    memcpy(buf, imgbuf.data(), imgbuf.size());
    *len = imgbuf.size();
}
uint32_t face_collection::find_empty_index_blk()
{
    uint32_t i;
    for (i = 0; i < FC_INDEX_BLK_COUNT; ++i) {
        if (this->read_index_bitmap(i) == 0)
            return i;
    }
    return FC_INDEX_BLK_COUNT;
}
bool face_collection::find_empty_data_blk(int blk_cnt, std::vector<uint16_t> blks)
{
    uint32_t i;
    for (i = 0; i < FC_DATA_BLK_COUNT && blk_cnt > 0; ++i) {
        if (this->read_data_bitmap(i) == 0 && i < FC_DATA_BLK_COUNT) {
            uint16_t j = i;
            blks.push_back(j);
            --blk_cnt;
        }
    }
    if (blk_cnt > 0) return false;
    else return true;
}
bool face_collection::add(int id, const cv::Mat &faceimg)
{
    unsigned char *face_buf;
    uint16_t face_offset;
    uint16_t face_len;
    if (!this->loaded) return false;
    if (id < FC_ID_MIN || id > FC_ID_MAX) return false;
    if (this->info.id_info.count(id) == 0) return false;

    //find empty blks
    uint8_t face_num = this->info.id_info[id].face_cnt;
    uint32_t empty_blk;
    uint16_t index_blk;
    uint16_t face_blk_cnt;
    std::vector<uint16_t> empty_data_blks;
    face_blk_cnt = FC_FACE_BLK_COUNT(face_len);
    if ((empty_blk = this->find_empty_index_blk()) >= FC_INDEX_BLK_COUNT && this->info.id_info[id].start_blk == 0)
        return false;
    index_blk = empty_blk;
    if (!this->find_empty_data_blk(face_blk_cnt, empty_data_blks)) return false;

    this->write_index_bitmap_1(index_blk);
    for (uint8_t i = 0; i < empty_data_blks.size(); ++i) {
        this->write_data_bitmap_1(empty_data_blks[i]);
    }

    std::fseek(this->fc_fp, FC_ID_INFO_OFFSET(id) + FC_ID_NAME_LEN, SEEK_SET);
    std::fwrite(&face_num, FC_ID_FACE_COUNT_LEN, 1, this->fc_fp);
    if (this->info.id_info[id].start_blk == 0) { //first face:
        std::fwrite(&index_blk, FC_ID_START_BLK_LEN, 1, this->fc_fp);
    } else { //append index_blk to the list:
        uint16_t index_next_blk;
        std::fseek(this->fc_fp,
                   FC_INDEX_BLK_OFFSET(this->info.id_info[id].start_blk) +
                   FC_INDEX_FACE_LEN_LEN + FC_INDEX_BLK_ADDR_LEN, SEEK_SET);
        std::fread(&index_next_blk, FC_INDEX_BLK_ADDR_LEN, 1, this->fc_fp);
        for (uint8_t i = 1; i < face_num - 1; ++i) {
            std::fseek(this->fc_fp,
                       FC_INDEX_BLK_OFFSET(index_next_blk) +
                       FC_INDEX_FACE_LEN_LEN + FC_INDEX_BLK_ADDR_LEN, SEEK_SET);
            std::fread(&index_next_blk, FC_INDEX_BLK_ADDR_LEN, 1, this->fc_fp);
        }
        std::fseek(this->fc_fp,
                   FC_INDEX_BLK_OFFSET(index_next_blk) +
                   FC_INDEX_FACE_LEN_LEN + FC_INDEX_BLK_ADDR_LEN, SEEK_SET);
        std::fwrite(&index_blk, FC_INDEX_BLK_ADDR_LEN, 1, this->fc_fp);
    }

    //write new index:
    std::fseek(this->fc_fp, FC_INDEX_BLK_OFFSET(index_blk), SEEK_SET);
    std::fwrite(&face_len, FC_INDEX_FACE_LEN_LEN, 1, this->fc_fp);
    std::fwrite(empty_data_blks.data(), FC_DATA_BLK_ADDR_LEN, 1, this->fc_fp);

    face_buf = new unsigned char[FC_FACE_LEN_MAX];
    this->mat2byte(faceimg, face_buf, &face_len);

    //write face data:
    face_offset = 0;
    empty_data_blks.push_back(0);
    for (uint8_t i = 0; i < empty_data_blks.size() - 1; ++i) {
        std::fseek(this->fc_fp, FC_DATA_BLK_OFFSET(empty_data_blks[i]), SEEK_SET);
        std::fwrite(face_buf + face_offset, FC_DATA_BLK_SIZE - FC_DATA_BLK_ADDR_LEN, 1, this->fc_fp);
        std::fwrite(&empty_data_blks[i + 1], FC_DATA_BLK_ADDR_LEN, 1, this->fc_fp);
        face_offset += (FC_DATA_BLK_SIZE - FC_DATA_BLK_ADDR_LEN);
    }

    //update this->info
    //perhaps this is the last thing to do.
    this->info.face_cnt++;
    this->info.id_info[id].face_cnt++;
    if (this->info.id_info[id].start_blk == 0)
        this->info.id_info[id].start_blk = index_blk;

    delete[] face_buf;
    return true;
}

bool face_collection::delete_id(int id)
{
    if (!this->loaded) return false;
    if (id < FC_ID_MIN || id > FC_ID_MAX) return false;
    if (this->info.id_info.count(id) != 0) return false;

    //erase data and index
    for (uint8_t i = 0; i < this->info.id_info[id].face_cnt; ++i) {
        this->erase(id, i + i);
    }
    //delete id info


    return true;
}


//todo..
bool face_collection::erase(int id, int facenum)
{
    if (!this->loaded) return false;
    if (id < FC_ID_MIN || id > FC_ID_MAX) return false;
    if (this->info.id_info.count(id) != 0) return false;
    if (facenum > this->info.id_info[id].face_cnt) return false;
    if (facenum <= FC_ID_FACE_COUNT_MIN) return false;

    uint16_t index_blk, index_blk_prev, index_blk_next;
    std::vector<uint16_t> data_blks;
    this->find_face_index_blk(id, facenum, &index_blk);
    this->find_face_data_blks(id, facenum, data_blks);

    for (uint16_t i = 0; i < data_blks.size(); ++i) {
        this->write_data_bitmap_0(data_blks[i]);
    }

    if (facenum == 1) {
        if (this->info.id_info[id].face_cnt == 1) {
            this->info.id_info[id].face_cnt = 0;
            this->info.id_info[id].start_blk = 0;
            this->write_index_bitmap_0(index_blk);

        }
    }


    this->info.face_cnt--;
    std::fseek(this->fc_fp, FC_SECTION_INFO_START +
               FC_NAME_LEN + FC_FACE_WIDTH_LEN +
               FC_FACE_HEIGHT_LEN + FC_ID_COUNT_LEN, SEEK_SET);
    std::fwrite(&this->info.face_cnt, FC_FACE_COUNT_LEN, 1, this->fc_fp);
}

bool face_collection::find_face_index_blk(int id, int face_num, uint16_t *index_blk)
{
    if (!this->loaded) return false;
    if (id < FC_ID_MIN || id > FC_ID_MAX) return false;
    if (this->info.id_info.count(id) != 0) return false;
    if (face_num > this->info.id_info[id].face_cnt) return false;
    if (face_num <= FC_ID_FACE_COUNT_MIN) return false;

    uint16_t next_blk = this->info.id_info[id].start_blk;

    for (uint8_t i = 0; i < face_num - 1; ++i) {
        std::fseek(this->fc_fp, FC_INDEX_BLK_OFFSET(next_blk), SEEK_SET);
        std::fseek(this->fc_fp, FC_INDEX_FACE_LEN_LEN + FC_DATA_BLK_ADDR_LEN, SEEK_CUR);
        std::fread(&next_blk, FC_INDEX_BLK_ADDR_LEN, 1, this->fc_fp);
    }
    *index_blk = next_blk;
    return true;
}

bool face_collection::find_face_data_blks(int id, int face_num, std::vector<uint16_t> &data_blks)
{
    if (!this->loaded) return false;
    if (id < FC_ID_MIN || id > FC_ID_MAX) return false;
    if (this->info.id_info.count(id) != 0) return false;
    if (face_num > this->info.id_info[id].face_cnt) return false;
    if (face_num <= FC_ID_FACE_COUNT_MIN) return false;

    uint16_t index_blk, face_len, face_start_blk, face_blk_cnt, next_blk;
    this->find_face_index_blk(id, face_num, &index_blk);
    std::fseek(this->fc_fp, FC_INDEX_BLK_OFFSET(index_blk), SEEK_SET);
    std::fread(&face_len, FC_INDEX_FACE_LEN_LEN, 1, this->fc_fp);
    std::fread(&face_start_blk, FC_DATA_BLK_ADDR_LEN, 1, this->fc_fp);
    face_blk_cnt = FC_FACE_BLK_COUNT(face_len);
    next_blk = face_start_blk;
    for (uint16_t i = 0; i < face_blk_cnt; ++i) {
        data_blks.push_back(next_blk);
        std::fseek(this->fc_fp, FC_DATA_BLK_OFFSET(next_blk), SEEK_SET);
        std::fseek(this->fc_fp, FC_DATA_BLK_SIZE - FC_DATA_BLK_ADDR_LEN, SEEK_CUR);
        std::fread(&next_blk, FC_DATA_BLK_ADDR_LEN, 1, this->fc_fp);
    }
    return true;
}
