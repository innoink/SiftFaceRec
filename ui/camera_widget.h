#ifndef CAMERA_WIDGET_H
#define CAMERA_WIDGET_H

#include <QObject>
#include <QMutex>
#include <QString>
#include <QList>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;

typedef void (*frame_proc_func) (Mat *img);

struct frame_processor_unit {
        QString name;
        frame_proc_func func;
};

typedef QList<frame_processor_unit> frame_processor;
class capture_worker : public QObject
{
        Q_OBJECT
    public:
        explicit capture_worker(VideoCapture *capture, QMutex *stopped_mutex, bool *stopped);
    public slots:
        //这个函数牺牲了事件循环。
        void do_capture();
        void set_processor(const frame_processor &fp);
        void remove_processor();
    signals:
        void capture_started();
        void capture_stopped();
        void new_frame(QImage frame, Mat *origin_frame);

    private:
        VideoCapture *capture;
        QMutex *stopped_mutex;
        bool *stopped;
        frame_processor fp;
};


#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QThread>

class camera_widget : public QWidget
{
        Q_OBJECT
    public:
        explicit camera_widget(QWidget *parent = 0);
        ~camera_widget();
        bool is_started();

        bool open_camera(int dev_no);
        bool close_camera();


    signals:
        void image_captured(int id, const QImage &preview);
        void set_processor(const frame_processor &fp);

        void start_c_worker();

    public slots:

        void update_frame(QImage frame, Mat *origin_frame);
        Mat get_frame_mat();

        void start_capture();
        void stop_capture();

    private slots:
        void release_capture();
    private:
        QSize sizeHint() const;
    private:
        bool started;

        QHBoxLayout *hl;
        Mat *corrent_frame;
        QLabel *lb_frame;

        VideoCapture capture;

        capture_worker *c_worker;
        QThread *c_worker_thread;

        QMutex c_worker_stopped_mutex;
        bool c_worker_stopped;

};

#endif // CAMERA_WIDGET_H
