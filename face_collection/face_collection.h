#ifndef FACE_COLLECTION_H
#define FACE_COLLECTION_H

#include <iostream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <cstdint>
#include <cstdio>

#include "face_recognizer/face_recognizer.h"

extern "C" {
#include <vedis.h>
}
#include <QDebug>
#include <QThread>

#define enter_func() \
    (qDebug() << "ENTER:"<<__func__ << QThread::currentThreadId())// << str)

#define leave_func() \
    (qDebug() << "LEAVE:"<<__func__ << QThread::currentThreadId())// << str)


/*
 str  collection:name
 set  collection:idset
 str  id:1:name
 set  id:1:faceset
 kv   id:1:face:1:data
 str  id:1:face:1:siftkp:trained
 str  id:1:face:1:siftkp:count
 kv   id:1:face:1:siftkp:1:x
 kv   id:1:face:1:siftkp:1:y
 kv   id:1:face:1:siftkp:1:sigma
 kv   id:1:face:1:siftkp:1:angle
 kv   id:1:face:1:siftkp:1:descr
 kv   id:1:face:1:siftkp:2:x
 kv   id:1:face:1:siftkp:2:y
 kv   id:1:face:1:siftkp:2:sigma
 kv   id:1:face:1:siftkp:2:angle
 kv   id:1:face:1:siftkp:2:descr
 kv   id:1:face:2:data
 ...


*/

#define FC_DB_FORMAT ".fc"
#define FC_IMG_FORMAT ".png"
#define FC_IMG_PARAM_ID CV_IMWRITE_PNG_COMPRESSION
#define FC_IMG_PARAM_VALUE 0

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
        std::vector<int> all_id();
        bool add_face(int id, int facenum, const cv::Mat &face);
        bool erase_face(int id, int facenum);
        bool get_face(int id, int facenum, cv::Mat &face);
        int max_face(int id);
        std::vector<int> all_face(int id);
        std::string clt_name();
        bool clt_rename(const std::string &name);
        bool clt_face_cnt(int *cnt);
        std::string id_name(int id);
        bool id_rename(int id, const std::string &name);
        bool id_face_cnt(int id, int *cnt);
        std::string error_string();
        std::string file_path();
        bool id_exist(int id);
        bool id_face_exist(int id, int facenum);

        bool set_train_data(int id, int facenum, std::vector<struct sift_keypoint_descr_t> &kpds);
        bool get_train_data(int id, int facenum, std::vector<struct sift_keypoint_descr_t> &kpds);
        bool is_trained(int id, int facenum);

    private:
        //handle the return code.if err, return false, else, return true;
        bool handle_rc(int rc, bool print);

        void byte2mat(const char *buf, int len, cv::Mat &mat) const;
        void mat2byte(const cv::Mat &mat, std::vector<unsigned char> &buf) const;

        void my_exec_fmt(const char *fmt, ...);
        void my_kv_store(const char *key, const void *data, vedis_int64 size);
        void my_kv_delete(const char *key);
        void my_kv_fetch(const char *key, void *buf, vedis_int64 *len);

    private:

        std::string filepath;
        vedis *clt_db;//collection database
        std::string err_str;

};

#endif // FACE_COLLECTION_H
