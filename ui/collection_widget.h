#ifndef COLLECTION_WIDGET_NEW_H
#define COLLECTION_WIDGET_NEW_H

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
#include "face_collection/face_collection.h"
#include "face_detector/face_detector.h"
#include "ui/camera_widget.h"

class face_detected_dlg : public QDialog
{
        Q_OBJECT
    public:
        explicit face_detected_dlg(bool *usy, QImage &face, QWidget *parent = 0);
    private:
        bool *user_says_yes;

        QLabel *lb_face;
        QPushButton *pb_yes;
        QPushButton *pb_no;
};

class fc_create_dlg : public QDialog
{
        Q_OBJECT
    public:
        explicit fc_create_dlg(bool *is_created, QWidget *parent = 0);
    private:
        QLineEdit *le_name;
        QLineEdit *le_path;
        QPushButton *pb_path;
        QPushButton *pb_create;
        QPushButton *pb_cancle;
};

class collection_widget : public QWidget
{
        Q_OBJECT
    public:
        explicit collection_widget(QWidget *parent = 0);
        void set_fc(face_collection *fc);

    signals:
        void take_done();

    public slots:
        void open_cam();
        void close_cam();

        void take();


        void clt_loaded();
        void clt_closed();
    private:

    private:
        camera_widget *wgt_camera;

        QWidget *wgt_right;

        QGroupBox *gb_cam;
        QSpinBox *sb_dev_no;
        QPushButton *pb_open_cam;
        QPushButton *pb_close_cam;

        QGroupBox *gb_clt;

        QLabel *lb_clt_file;
        QLabel *lb_clt_name;

        QSpinBox *sb_clt_id;
        QSpinBox *sb_clt_face;
        QPushButton *pb_take;

        face_detector fd;
        face_collection *fc;

        QString clt_recent_path;
};

#endif // COLLECTION_WIDGET_NEW_H
