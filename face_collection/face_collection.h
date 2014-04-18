#ifndef FACE_COLLECTION_H
#define FACE_COLLECTION_H

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

#define MAX_ID 8 //2^(MAX_ID) is the max number of id.
#define ID_MAX_FACES 8  //2^(ID_MAX_FACES) is the max amount of faces one id can have.
//cv::imencode/cv::imdecode/std::vector::data()
/* default structure:

 magic bits(0xFCFCFCFC) : 32 bits
 --info section start--
 name : 128 bit
 face width : 32 bit
 face height : 32 bit
 id count : 8 bit
 face count : 16 bit
 id list:
 id1(8 bit)|face count(8 bit)|start block


 --
 --







*/

class face_collection
{
    public:
        face_collection();
        bool load(const std::string &name, const std::string &path);
        bool create(const std::string &name, const std::string &path);
        bool save();
        bool add(int id, const cv::Mat &faceimg);
        //bool erase(int id, int num);
        //bool get(int id, int num, Mat &faceimg);
        //bool extract();
    private:

    private:
        std::string name;
        std::string path;

};

#endif // FACE_COLLECTION_H
