#include "img_fmt_cvt.h"
#include <QDebug>

bool mat2qimage(const Mat &mat, QImage &qimage)
{
    // 8-bits unsigned, NO. OF CHANNELS=1
    if (mat.type() == CV_8UC1)
    {
        // Set the color table (used to translate colour indexes to qRgb values)
        QVector<QRgb> colorTable;
        for (int i = 0 ; i < 256 ; i++)
            colorTable.push_back(qRgb(i, i, i));
        // Copy input Mat
        const uchar *qImageBuffer = (const uchar *)mat.data;
        // Create QImage with same dimensions as input Mat
        qimage = QImage(qImageBuffer, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8).copy();
        qimage.setColorTable(colorTable);
    }
    // 8-bits unsigned, NO. OF CHANNELS=3
    else if (mat.type() == CV_8UC3)
    {
        // Copy input Mat
        const uchar *qImageBuffer = (const uchar *)mat.data;
        // Create QImage with same dimensions as input Mat
        qimage = QImage(qImageBuffer, mat.cols, mat.rows, mat.step, QImage::Format_RGB888).rgbSwapped();
    }
    else
    {
        qDebug() << "ERROR: Mat could not be converted to QImage.";
        return false;
    }
    return true;
}


bool qimage2iplimage(const QImage &qimage, IplImage **iplimage)
{
    static IplImage *_iplimg;
    int channel = 0;
    if (qimage.format() == QImage::Format_RGB888) channel = 3;
    if (qimage.format() == QImage::Format_ARGB32) channel = 4;
    if (channel == 0) {
        qDebug() << "ERROR: QImage could not be converted to IplImage.";
        return false;
    }

    *iplimage = cvCreateImage(cvSize(qimage.width(), qimage.height()), 8, channel);
    _iplimg = cvCreateImageHeader(cvSize(qimage.width(), qimage.height()), 8, channel);
    _iplimg->imageData = (char *)qimage.bits();
    cvCopy(_iplimg, *iplimage, 0);
    cvReleaseImageHeader(&_iplimg);

    if (channel == 3)
        cvConvertImage(*iplimage, *iplimage, CV_CVTIMG_SWAP_RB);

    return true;
}



bool iplimage2qimage(const IplImage *iplimage, QImage &qimage)
{
    int height = iplimage->height;
    int width = iplimage->width;

    if (iplimage->depth == IPL_DEPTH_8U && iplimage->nChannels == 3) {
        const uchar *qImageBuffer = (const uchar *)iplimage->imageData;
        QImage img(qImageBuffer, width, height, QImage::Format_RGB888);
        qimage = img.rgbSwapped();
        return true;
    } else if (iplimage->depth == IPL_DEPTH_8U && iplimage->nChannels == 1) {
        const uchar *qImageBuffer = (const uchar *)iplimage->imageData;
        qimage = QImage(qImageBuffer, width, height, QImage::Format_Indexed8);

        QVector<QRgb> colorTable;
        for (int i = 0 ; i < 256 ; i++) {
            colorTable.push_back(qRgb(i, i, i));
        }
        qimage.setColorTable(colorTable);
        return true;
    } else {
        qWarning() << "Image cannot be converted.";
        return false;
    }
}
