#ifndef FACE_COLLECTION_H
#define FACE_COLLECTION_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <opencv2/opencv.hpp>
#include <cstdint>
#include <cstdio>

extern "C" {
#include <unqlite.h>
}

/*
 * 人脸的 *无序* 集合.
 * 单个id人脸上限:256.
*/

/*
 info:name => collection name
 info:idcnt:4 byte => collection id count
 info:facecnt:4 byte => collection face count
 id:1:name => id1 name
 id:1:facecnt:4 byte => id1 face count
 id:1:face:bitmap:4 byte(only the first byte is used) => which face:key is used.
 id:1:face:1:data
 id:1:face:2:data
 id:1:face:3:data
 .......
 id:1:face:256:data
 id:2:face:bitmap:4 byte
 id:2:face:1:data
 id:2:face:2:data
 .......
 .......
 id:n:face:x:data
*/

#define FC_IMG_FORMAT ".png"
#define FC_IMG_PARAM_ID CV_IMWRITE_PNG_COMPRESSION
#define FC_IMG_PARAM_VALUE 0

#define FC_ID_FACE_MIN 1
#define FC_ID_FACE_MAX 256

typedef int32_t fcint_t;

class face_collection
{
    public:
        face_collection();
        ~face_collection();
        bool load(const std::string &filepath);
        bool create(const std::string &name, const std::string &filepath);
        void save();
        void close();
        bool new_id(int id, const std::string &name);
        bool delete_id(int id);
        bool add_face(int id, const cv::Mat &face, int *facenum);
        bool erase_face(int id, int facenum);
        bool clt_name(std::string &name);
        bool clt_rename(const std::string &name);
        bool clt_face_cnt(int *cnt);
        bool id_name(int id, std::string &name);
        bool id_rename(int id, const std::string &name);
        bool id_face_cnt(int id, int *cnt);
        int get_id_first_face();
        int get_id_next_face(int n);
        int get_id_last_face();
        std::string error_string();
        std::string file_path();


    private:
        //handle the return code.if err, return false, else, return true;
        bool handle_rc(int rc, bool print);

        void byte2mat(const char *buf, fcint_t len, cv::Mat &mat) const;
        void mat2byte(const cv::Mat &mat, std::vector<unsigned char> &buf) const;

        // range of n : [1,256]
        unsigned bitmap_get(const char *bitmap, unsigned n);
        void bitmap_set_0(char *bitmap, unsigned n);
        void bitmap_set_1(char *bitmap, unsigned n);

        bool id_exist(fcint_t id);
        bool id_face_exist(fcint_t id, fcint_t facenum);

        std::string read_info_name();
        void write_info_name(const std::string &name);
        fcint_t read_info_idcnt();
        void write_info_idcnt(fcint_t cnt);
        fcint_t read_info_facecnt();
        void write_info_facecnt(fcint_t cnt);
        std::string read_id_name(fcint_t id);
        void write_id_name(fcint_t id, const std::string &name);
        fcint_t read_id_facecnt(fcint_t id);
        void write_id_facecnt(fcint_t id, fcint_t facecnt);
        void read_id_face_bitmap(fcint_t id, char *bitmap);
        void write_id_face_bitmap_0(fcint_t id, fcint_t n);
        void write_id_face_bitmap_1(fcint_t id, fcint_t n);
        void read_id_face(fcint_t id, fcint_t n, cv::Mat &face);
        void write_id_face(fcint_t id, fcint_t n, const cv::Mat &face);

    private:

        std::string filepath;
        unqlite *clt_db;//collection database
        std::string err_str;

};

#endif // FACE_COLLECTION_H
