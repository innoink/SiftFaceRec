#ifndef SIFT_MATCHER_WIDGET_H
#define SIFT_MATCHER_WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextEdit>
#include <opencv2/opencv.hpp>
#include <opencv2/nonfree/features2d.hpp>
using namespace cv;

class result_label : public QLabel
{
        Q_OBJECT
    public:
        explicit result_label(QWidget *parent = 0);
        QSize sizeHint() const;
};

class sift_matcher_widget : public QWidget
{
        Q_OBJECT
    public:
        explicit sift_matcher_widget(QWidget *parent = 0);

    signals:

    public slots:

        void choose_pic_1();
        void choose_pic_2();
        void sift_match();
    private:
        QLabel *lb_pic_1, *lb_pic_2;
        result_label *lb_result;
        QLineEdit *le_pic_1, *le_pic_2;
        QPushButton *pb_pic_1, *pb_pic_2;
        QPushButton *pb_match;
        QTextEdit *te_info;

        SiftFeatureDetector sfd;
        SiftDescriptorExtractor sde;
        BFMatcher bfm;
};

#endif // SIFT_MATCHER_WIDGET_H
