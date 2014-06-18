#include "camera_widget.h"
#include "utils/img_fmt_cvt.h"

#include <QDebug>

capture_worker::capture_worker(VideoCapture *capture, QMutex *stop_mutex, bool *stopped):
    capture(capture), stopped_mutex(stop_mutex), stopped(stopped)
{
    this->setParent(0);
    *(this->stopped) = true;
}

void capture_worker::do_capture()
{
    static Mat *frame;
    static QImage frame_qimg;
    static Mat    frame_mat;
    static struct frame_processor_unit fpu;

    this->stopped_mutex->lock();
    *(this->stopped) = false;
    this->stopped_mutex->unlock();
    emit capture_started();
    while (true) {
        this->stopped_mutex->lock();
        if (*(this->stopped)) {
            this->stopped_mutex->unlock();
            break;
        }
        this->stopped_mutex->unlock();
        if (!this->capture->grab())
            continue;
        frame = new Mat;
        if (!this->capture->retrieve(*frame)) continue;
        frame_mat = frame->clone();
        foreach (fpu, this->fp) {
            fpu.func(&frame_mat);
        }
        mat2qimage(frame_mat, frame_qimg);
        emit new_frame(frame_qimg, frame);
        frame_mat.release();
    }
    emit capture_stopped();
}

void capture_worker::set_processor(const frame_processor &fp)
{
    this->fp = fp;
}

void capture_worker::remove_processor()
{
    this->fp.clear();
}

camera_widget::camera_widget(QWidget *parent) :
    QWidget(parent)
{

    qRegisterMetaType<frame_processor>("frame_processor");


    this->c_worker = new capture_worker(&this->capture, &this->c_worker_stopped_mutex, &this->c_worker_stopped);
    this->c_worker_thread = new QThread(this);
    this->c_worker->moveToThread(this->c_worker_thread);

    this->lb_frame = new QLabel(this);
    this->hl = new QHBoxLayout(this);

    this->hl->setMargin(0);
    this->hl->setSpacing(0);
    this->hl->addWidget(this->lb_frame);

    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    this->corrent_frame = NULL;
    this->started = false;

    connect(this, &camera_widget::start_c_worker, this->c_worker, &capture_worker::do_capture);
    connect(this->c_worker_thread, &QThread::finished, this->c_worker, &capture_worker::deleteLater);
    connect(this, &camera_widget::set_processor, this->c_worker, &capture_worker::set_processor);
    connect(this->c_worker, &capture_worker::new_frame, this, &camera_widget::update_frame);

    this->c_worker_thread->start();
}
QSize camera_widget::sizeHint() const
{
    return QSize(480, 640);
}

camera_widget::~camera_widget()
{
    this->close_camera();

    this->c_worker_thread->quit();
    this->c_worker_thread->wait();

    delete this->corrent_frame;
}

bool camera_widget::open_camera(int dev_no)
{
    if (this->capture.isOpened()) return false;
    this->capture.open(dev_no);
    if (!this->capture.isOpened())
        return false;
    this->start_capture();
    return true;
}

bool camera_widget::close_camera()
{
    connect(this->c_worker, &capture_worker::capture_stopped, this, &camera_widget::release_capture);
    this->stop_capture();
    return true;
}

void camera_widget::release_capture()
{
    if (this->capture.isOpened())
    {
        this->capture.release();
    }
    this->lb_frame->setPixmap(QPixmap());
    disconnect(this->c_worker, &capture_worker::capture_stopped, this, &camera_widget::release_capture);
}

void camera_widget::update_frame(QImage frame, Mat *origin_frame)
{
    this->lb_frame->setPixmap(QPixmap::fromImage(frame));
    if (this->corrent_frame != NULL) delete this->corrent_frame;
    this->corrent_frame = origin_frame;
}

void camera_widget::start_capture()
{

    emit this->start_c_worker();
    //not correct!!
    this->started = true;
}

bool camera_widget::is_started()
{
    return this->started;
}


void camera_widget::stop_capture()
{
    this->c_worker_stopped_mutex.lock();
    this->c_worker_stopped = true;
    this->c_worker_stopped_mutex.unlock();
    this->started = false;
}

Mat camera_widget::get_frame_mat()
{
    return *this->corrent_frame;
}
