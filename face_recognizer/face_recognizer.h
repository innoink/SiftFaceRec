#ifndef FACE_RECOGNIZER_H
#define FACE_RECOGNIZER_H

#include <opencv2/nonfree/features2d.hpp>
using namespace cv;
class face_recognizer
{
    public:
        face_recognizer();
    private:
        SiftFeatureDetector sfd;
};

#endif // FACE_RECOGNIZER_H
