#ifndef COLLECTION_WIDGET_H
#define COLLECTION_WIDGET_H

#include <QWidget>
#include <QDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QHash>
#include "face_detector/face_detector.h"
#include "ui/camera_widget.h"

class face_detected_dlg : public QDialog
{
        Q_OBJECT
    public:
        explicit face_detected_dlg(bool *usy, QImage &face, QWidget *parent = 0);
    signals:
    private:
        bool *user_says_yes;

        QLabel *lb_face;
        QPushButton *pb_yes;
        QPushButton *pb_no;
};

class collection_widget : public QWidget
{
        Q_OBJECT
    public:
        explicit collection_widget(QWidget *parent = 0);

    signals:

    public slots:
        void open_cam();
        void close_cam();
        void choose_path();
        void take();
    private:
        void save_face(const Mat &faceimg, int id);
        void save_face(const IplImage *faceimg, int id);
    private:
        camera_widget *wgt_camera;

        QWidget *wgt_right;

        QGroupBox *gb_cam;
        QSpinBox *sb_dev_no;
        QPushButton *pb_open_cam;
        QPushButton *pb_close_cam;

        QGroupBox *gb_clt;
        QLineEdit *le_clt_path;
        QPushButton *pb_clt_choose_path;
        QSpinBox *sb_clt_id;
        QPushButton *pb_take;

        face_detector fd;

        QHash<int, int> id_map;


};

#endif // COLLECTION_WIDGET_H
