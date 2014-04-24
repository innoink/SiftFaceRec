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
#include <vedis.h>
}

/*
 * 人脸的 *无序* 集合.
 * 单个id人脸上限:256.
*/

/*
 str  collection:name
 set  collection:idset
 str  id:1:name
 set  id:1:faceset
 hash id:1:faces 0 xxx 1 xxx...

 id:2:name
 id:2:faceset
 id:3:name
*/

#define FC_IMG_FORMAT ".png"
#define FC_IMG_PARAM_ID CV_IMWRITE_PNG_COMPRESSION
#define FC_IMG_PARAM_VALUE 0

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
        std::string error_string();
        std::string file_path();


    private:
        //handle the return code.if err, return false, else, return true;
        bool handle_rc(int rc, bool print);

        void byte2mat(const char *buf, fcint_t len, cv::Mat &mat) const;
        void mat2byte(const cv::Mat &mat, std::vector<unsigned char> &buf) const;

        bool id_exist(fcint_t id);
        bool id_face_exist(fcint_t id, fcint_t facenum);

        std::string read_id_name(fcint_t id);
        void write_id_name(fcint_t id, const std::string &name);


        void read_id_face(fcint_t id, fcint_t n, cv::Mat &face);
        void write_id_face(fcint_t id, fcint_t n, const cv::Mat &face);

    private:

        std::string filepath;
        vedis *clt_db;//collection database
        std::string err_str;

};

#endif // FACE_COLLECTION_H
