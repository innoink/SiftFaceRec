#ifndef SIFT_MATCHER_WIDGET_H
#define SIFT_MATCHER_WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QGroupBox>
#include <QRadioButton>
#include <QDoubleSpinBox>
#include <opencv2/opencv.hpp>
#include <opencv2/nonfree/features2d.hpp>
extern "C" {
#include <vl/sift.h>
#include <vl/generic.h>
#include <vl/stringop.h>
#include <vl/pgm.h>
#include <vl/getopt_long.h>
}


using namespace cv;

class result_label : public QLabel
{
        Q_OBJECT
    public:
        explicit result_label(QWidget *parent = 0);
        QSize sizeHint() const;
};

class sift_demo_widget : public QWidget
{
        Q_OBJECT
    public:
        explicit sift_demo_widget(QWidget *parent = 0);

    signals:

    public slots:

        void choose_pic_1();
        void choose_pic_2();
        void do_sift();
    private:
        void sift_opencv(Mat &img1, Mat &img2, bool match, double rate);
        void sift_vlfeat(Mat &img1, Mat &img2, bool match, double rate);
    private:
        QGroupBox *gb_type;
        QRadioButton *rb_detect;
        QRadioButton *rb_detect_and_match;
        QDoubleSpinBox *dsb_good_rate;

        QGroupBox *gb_source;
        QLabel *lb_pic_1, *lb_pic_2;
        result_label *lb_result;
        QLineEdit *le_pic_1, *le_pic_2;
        QPushButton *pb_pic_1, *pb_pic_2;

        QGroupBox *gb_method;
        QRadioButton *rb_opencv;
        QRadioButton *rb_vlfeat;

        QPushButton *pb_do_sift;

        QTextEdit *te_info;

        QString recent_path;
};

#endif // SIFT_MATCHER_WIDGET_H
