#ifndef IMG_FMT_CVT_H
#define IMG_FMT_CVT_H
#include <QImage>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cv.hpp>
using namespace cv;
bool mat2qimage(const Mat& mat, QImage &qimage);
bool iplimage2qimage(const IplImage *iplimage, QImage &qimage);
bool qimage2iplimage(const QImage &qimage, IplImage **iplimage);

#endif // IMG_FMT_CVT_H
