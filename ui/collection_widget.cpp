#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include "collection_widget.h"
#include "utils/img_fmt_cvt.h"
#include "face_collection/face_collection.h"


face_detected_dlg::face_detected_dlg(bool *usy, QImage &face, QWidget *parent) :
    QDialog(parent), user_says_yes(usy)
{

    this->lb_face = new QLabel(this);
    this->lb_face->setPixmap(QPixmap::fromImage(face));
    this->pb_yes = new QPushButton(tr("yes"), this);
    this->pb_no = new QPushButton(tr("no"), this);
    QVBoxLayout *vl;
    QHBoxLayout *hl;
    vl = new QVBoxLayout;
    hl = new QHBoxLayout;
    vl->addWidget(this->lb_face);
    hl->addWidget(this->pb_yes);
    hl->addWidget(this->pb_no);
    vl->addLayout(hl);
    this->setLayout(vl);

    connect(this->pb_yes, &QPushButton::clicked, [this](){*this->user_says_yes = true;this->close();});
    connect(this->pb_no, &QPushButton::clicked, [this](){*this->user_says_yes = false;this->close();});

}

collection_widget::collection_widget(QWidget *parent) :
    QWidget(parent)
{
    QHBoxLayout *hl;
    QVBoxLayout *vl;
    QGridLayout *gl;

    this->wgt_right = new QWidget(this);
    this->gb_cam = new QGroupBox(tr("camera"), this->wgt_right);
    this->sb_dev_no = new QSpinBox(this->gb_cam);
    this->pb_open_cam = new QPushButton(tr("open"), this->gb_cam);
    this->pb_close_cam = new QPushButton(tr("close"), this->gb_cam);

    vl = new QVBoxLayout;
    hl = new QHBoxLayout;
    hl->addWidget(new QLabel(tr("camera device:"), this->gb_cam));
    hl->addWidget(this->sb_dev_no);
    vl->addLayout(hl);
    hl = new QHBoxLayout;
    hl->addWidget(this->pb_open_cam);
    hl->addWidget(this->pb_close_cam);
    vl->addLayout(hl);
    this->gb_cam->setLayout(vl);

    this->gb_clt = new QGroupBox(tr("collection"), this->wgt_right);
    this->sb_clt_id = new QSpinBox(this->gb_clt);
    this->le_clt_path = new QLineEdit(this->gb_clt);
    this->pb_clt_choose_path = new QPushButton(tr("..."), this->gb_clt);
    this->pb_take = new QPushButton(tr("take"), this->gb_clt);

    vl = new QVBoxLayout;
    gl = new QGridLayout;
    gl->addWidget(new QLabel(tr("save path:"), this->gb_clt), 0, 0);
    gl->addWidget(this->le_clt_path, 0, 1);
    gl->addWidget(this->pb_clt_choose_path, 0, 2);
    gl->addWidget(new QLabel(tr("ID:"), this->gb_clt), 1, 0);
    gl->addWidget(this->sb_clt_id, 1, 1);
    vl->addLayout(gl);
    vl->addWidget(this->pb_take);
    this->gb_clt->setLayout(vl);

    vl = new QVBoxLayout;
    vl->addWidget(this->gb_cam);
    vl->addWidget(this->gb_clt);
    this->wgt_right->setLayout(vl);

    this->wgt_camera = new camera_widget(this);
    hl = new QHBoxLayout;
    vl = new QVBoxLayout;
    hl->addWidget(this->wgt_camera);
    hl->addWidget(this->wgt_right);
    this->setLayout(hl);

    this->wgt_right->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    this->pb_close_cam->setDisabled(true);

    connect(this->pb_open_cam, &QPushButton::clicked, this, &collection_widget::open_cam);
    connect(this->pb_close_cam, &QPushButton::clicked, this, &collection_widget::close_cam);
    connect(this->pb_clt_choose_path, &QPushButton::clicked, this, &collection_widget::choose_path);
    connect(this->pb_take, &QPushButton::clicked, this, &collection_widget::take);
}

void collection_widget::open_cam()
{
    //static face_detector fd;
    static frame_processor face_marker;
    static struct frame_processor_unit unit;
    unit.name = QString("face marker");
    unit.func = face_detector::face_marker_lbp;
    face_marker.append(unit);
    this->wgt_camera->set_processor(face_marker);
    if (this->wgt_camera->open_camera(this->sb_dev_no->value())) {
        this->pb_open_cam->setDisabled(true);
        this->pb_close_cam->setEnabled(true);
    }
}

void collection_widget::close_cam()
{
    if (this->wgt_camera->close_camera()) {
        this->pb_close_cam->setDisabled(true);
        this->pb_open_cam->setEnabled(true);
    }
}

void collection_widget::choose_path()
{
    QString path;
    path = QFileDialog::getExistingDirectory(this, tr("choose a path"), "./");
    this->le_clt_path->setText(path);
}

void collection_widget::save_face(const IplImage *faceimg, int id)
{
    Mat m_img(faceimg, true);
    if (!this->id_map.contains(id))
        this->id_map.insert(id, 0);
    imwrite((this->le_clt_path->text() + "/" + QString::number(id) + "_" + QString::number(this->id_map[id]) + ".png").toLocal8Bit().constData(), m_img);
    this->id_map[id]++;
}

void collection_widget::take()
{
    static Mat m_tmp;
    static IplImage iplframe;
    static Mat face_marked;
    static QImage face_marked_qimg;
    static face_detected_dlg *fdw;
    static bool usy = true;
    if (!this->wgt_camera->is_started()) {
        QMessageBox msgBox;
        msgBox.setText("Start Camera First!");
        msgBox.exec();
        return;
    }
    if (this->le_clt_path->text().isEmpty()) {
        QMessageBox msgBox;
        msgBox.setText("Set Save Path!");
        msgBox.exec();
        return;
    }

    m_tmp = this->wgt_camera->get_frame_mat().clone();
    cvtColor(m_tmp, m_tmp, COLOR_BGR2GRAY);
    iplframe = m_tmp;
    fd.set_input_image(&iplframe);
    fd.get_face_parameters(face_marked);
    mat2qimage(face_marked, face_marked_qimg);
    fdw = new face_detected_dlg(&usy, face_marked_qimg, this);
    fdw->exec();
    if (usy) {
        this->save_face(fd.get_face_img(), this->sb_clt_id->value());
    } else {
        delete fdw;
        return;
    }
    delete fdw;
}
