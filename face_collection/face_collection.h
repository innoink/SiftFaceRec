#ifndef FACE_COLLECTION_H
#define FACE_COLLECTION_H

#include <string>
#include <vector>
#include <map>
#include <opencv2/opencv.hpp>

#include <cstdint>
#include <cstdio>
#include <cstdlib>

#define FC_IMG_FORMAT ".png"
#define FC_IMG_PARAM_ID CV_IMWRITE_PNG_COMPRESSION
#define FC_IMG_PARAM_VALUE 0

#define MAX_ID_BITS (8) //2^(MAX_ID) is the max number of id.
#define ID_MAX_FACES_BITS (8)  //2^(ID_MAX_FACES) is the max amount of faces one id can have.

#define FC_ID_MAX (255)
#define FC_ID_MIN (1)

#define FC_ID_FACE_COUNT_MAX (255)
#define FC_ID_FACE_COUNT_MIN (0)

#define FC_FACE_LEN_MAX (64 * 1024) //64 KB

#define FC_DATA_BLK_SIZE (4 * 1024) //4 KB
#define FC_INDEX_BLK_SIZE (6)
//byte
#define FC_MAGIC_BITS_LEN      (4)
#define FC_SECTION_INFO_START  FC_MAGIC_BITS_LEN
#define FC_SECTION_INFO_LEN    (9035)
#define FC_SECTION_UNUSED_START (FC_SECTION_INFO_START + FC_SECTION_INFO_LEN)
#define FC_SECTION_UNUSED_LEN  (7345)
#define FC_HEAD_LEN            (16 * 1024)
#define FC_SECTION_INDEX_START FC_HEAD_LEN
#define FC_INDEX_BITMAP_START  FC_SECTION_INDEX_START
#define FC_INDEX_BITMAP_LEN    (8 * 1024)
#define FC_INDEX_BLK_START     (FC_INDEX_BITMAP_START + FC_INDEX_BITMAP_LEN)
#define FC_INDEX_BLK_COUNT     (65536)
#define FC_INDEX_BLK_ADDR_LEN  (2)
#define FC_INDEX_FACE_LEN_LEN  (2)
#define FC_SECTION_INDEX_LEN   (FC_INDEX_BITMAP_LEN + FC_INDEX_BLK_COUNT * FC_INDEX_BLK_SIZE)
#define FC_SECTION_DATA_START  (FC_HEAD_LEN + FC_SECTION_INDEX_LEN)
#define FC_DATA_BITMAP_START   FC_SECTION_DATA_START
#define FC_DATA_BITMAP_LEN     (8 * 1024)
#define FC_DATA_BLK_START      (FC_DATA_BITMAP_START + FC_DATA_BITMAP_LEN)
#define FC_DATA_BLK_ADDR_LEN   (2)
#define FC_DATA_BLK_COUNT      (65536)

#define FC_NAME_LEN            (32)
#define FC_FACE_WIDTH_LEN      (4)
#define FC_FACE_HEIGHT_LEN     (4)
#define FC_ID_COUNT_LEN        (1)
#define FC_FACE_COUNT_LEN      (4)
#define FC_ID_BITMAP_LEN       (32)
#define FC_ID_INFO_START       (\
    FC_SECTION_INFO_START + FC_NAME_LEN + FC_FACE_WIDTH_LEN + FC_FACE_HEIGHT_LEN + FC_ID_COUNT_LEN +\
    FC_FACE_COUNT_LEN + FC_ID_BITMAP_LEN \
                               )
#define FC_ID_NAME_LEN         (32)
#define FC_ID_FACE_COUNT_LEN   (1)
#define FC_ID_START_BLK_LEN    (2)
#define FC_ID_INFO_LEN         (35)


/* default structure:

 magic bits(0xFCFCFCFC) : 32 bits
 --info section start--
 name : 256 bit //248 avaliable
 face width : 32 bit
 face height : 32 bit
 id count : 8 bit
 face count : 16 bit
 id_bitmap[256 bit];
 id info:
 id1 name[256 bit]|face count(8 bit)|start blk(16 bit) //35 byte
 id2 name[256 bit]|face count(8 bit)|start blk(16 bit) //35 byte
 ....
 id256 name[256 bit]|face count(8 bit)|start blk(16 bit) //35 byte
 --info section end(9035 byte)--
 --unused section start--
 00 00 00...
 00 00 00...
 --unused section end(7345 byte)--
 ----- head total 16 KB -----
 --index section start--
 index bitmap[8KB(8192 bit)]
 blk0[48 bit]=>[face len(16 bit) | face start blk(16 bit) | next blk(16 bit)]
 blk1[]
 ...
 blk65535[]
 --index section end(392 KB)--
 --data section start--
 data bitmap[8KB(8192 bit)]
 blk0[4kB]=>[4KB-16bit|next blk(16 bit)]
 blk1[]
 ...
 blk65535[]
 --data section end--

*/

#define FC_MAGIC_BITS 0xFCFCFCFC
#define FC_ID_INFO_OFFSET(id) \
    (FC_ID_INFO_START + ((id) - 1) * FC_ID_INFO_LEN)
#define FC_INDEX_BLK_OFFSET(blk) \
    (FC_INDEX_BLK_START + (blk) * FC_INDEX_BLK_SIZE)
#define FC_DATA_BLK_OFFSET(blk) \
    (FC_SECTION_DATA_START + (blk) * FC_DATA_BLK_SIZE)
#define FC_FACE_BLK_COUNT(face_len) \
    ((face_len) / (FC_DATA_BLK_SIZE - FC_DATA_BLK_ADDR_LEN) + \
    ((face_len) % (FC_DATA_BLK_SIZE) == 0 ? 0 : 1))

struct fc_id_info_t {
        char          name[32];
        uint8_t       face_cnt;
        uint16_t      start_blk;
};

struct fc_info_t {
        char          name[32];
        uint32_t      face_width;
        uint32_t      face_height;
        uint8_t       id_cnt;
        uint16_t      face_cnt;
        std::map<uint8_t, struct fc_id_info_t> id_info;
};

class face_collection
{
    public:
        face_collection();
        ~face_collection();
        bool is_loaded() const;
        bool load(const std::string &filepath);
        bool create(const std::string &filepath, const std::string &name, int face_width, int face_height);
        bool save(const std::string &filepath);
        //bool close();
        bool new_id(int id, const std::string &name);
        bool delete_id(int id);
        bool add(int id, const cv::Mat &faceimg);
        bool erase(int id, int facenum);
        bool get(int id, int facenum, cv::Mat &faceimg);
        bool extract(const std::string &dirpath) const;
        bool get_info(struct fc_info_t &info) const;
    private:
        void clear_info();
        uint32_t read_id_bitmap(uint32_t n);
        void write_id_bitmap_0(uint32_t n);
        void write_id_bitmap_1(uint32_t n);
        uint32_t read_index_bitmap(uint32_t n);
        void write_index_bitmap_0(uint32_t n);
        void write_index_bitmap_1(uint32_t n);
        uint32_t read_data_bitmap(uint32_t n);
        void write_data_bitmap_0(uint32_t n);
        void write_data_bitmap_1(uint32_t n);
        bool load_info();
        bool load_id_info();
        bool load_index_bitmap();
        bool load_data_bitmap();

        bool create_empty_collection();

        uint32_t find_empty_index_blk();
        bool find_empty_data_blk(int blk_cnt, std::vector<uint16_t> blks);
        void free_index_blk();
        void free_data_blk();

        void byte2mat(const unsigned char *buf, uint16_t len, cv::Mat &mat) const;
        void mat2byte(const cv::Mat &mat, unsigned char *buf, uint16_t *len) const;

        bool find_face_index_blk(int id, int face_num, uint16_t *index_blk);
        bool find_face_data_blks(int id, int face_num, std::vector<uint16_t> &data_blks);




    private:
        static unsigned char zero_buffer[128];
        bool                 loaded;
        bool                 modified;
        struct fc_info_t     info;
        std::string          name;
        std::string          path;
        std::FILE*           fc_fp;

        //C系列IO函数自带buffer，这里就不添加额外的buffer了。
        //char*            d_buf;//data blk buffer

        unsigned char*   cur_blk;
        unsigned char    id_bitmap[32];
        unsigned char*   index_bitmap;
        unsigned char*   data_bitmap;

};

#endif // FACE_COLLECTION_H
