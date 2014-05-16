#ifndef FACE_RECOGNIZER_H
#define FACE_RECOGNIZER_H

#include <opencv2/opencv.hpp>
#include <vector>
using namespace cv;
extern "C" {
#include <vl/sift.h>
#include <vl/generic.h>
#include <vl/stringop.h>
#include <vl/pgm.h>
#include <vl/getopt_long.h>
}

struct sift_keypoint_descr_t {
        float x, y, sigma, angle;
        float descr[128];
};
void print_kpds(std::vector<sift_keypoint_descr_t> &kpds);

class face_recognizer
{
    public:
        face_recognizer();
        void train(Mat &img, std::vector<struct sift_keypoint_descr_t> &kpds);
        void match(std::vector<struct sift_keypoint_descr_t> &kpds1,
                   std::vector<struct sift_keypoint_descr_t> &kpds2,
                   double thresh,
                   int *m_cnt);
};

#endif // FACE_RECOGNIZER_H
