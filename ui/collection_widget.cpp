#include <QFileDialog>
#include <QDebug>
#include "collection_widget.h"
#include "utils/img_fmt_cvt.h"


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
    this->wgt_camera = new camera_widget(this);

    this->gb_cam = new QGroupBox(tr("camera"), this);
    this->sb_dev_no = new QSpinBox(this);
    this->pb_open_cam = new QPushButton(tr("open"), this);
    this->pb_close_cam = new QPushButton(tr("close"), this);

    this->gb_clt = new QGroupBox(tr("collection"), this);
    this->sb_clt_id = new QSpinBox(this->gb_clt);
    this->le_clt_path = new QLineEdit(this->gb_clt);
    this->pb_clt_choose_path = new QPushButton(tr("choose collection path"), this->gb_clt);
    this->pb_take = new QPushButton(tr("take"), this->gb_clt);

    this->pb_close_cam->setDisabled(true);
    //this->le_clt_path->setDisabled(true);


    this->vl = new QVBoxLayout;
    this->hl = new QHBoxLayout;

    this->hl->addWidget(new QLabel(tr("save path:"), this->gb_clt));
    this->hl->addWidget(this->le_clt_path);
    this->vl->addLayout(this->hl);
    this->vl->addWidget(this->pb_clt_choose_path);
    this->hl = new QHBoxLayout;
    this->hl->addWidget(new QLabel(tr("ID:"), this->gb_clt));
    this->hl->addWidget(this->sb_clt_id);
    this->vl->addLayout(this->hl);
    this->vl->addWidget(this->pb_take);
    this->gb_clt->setLayout(this->vl);

    this->vl = new QVBoxLayout;
    this->hl = new QHBoxLayout;
    this->hl->addWidget(new QLabel(tr("camera device:"), this->gb_cam));
    this->hl->addWidget(this->sb_dev_no);
    this->vl->addLayout(this->hl);
    this->hl = new QHBoxLayout;
    this->hl->addWidget(this->pb_open_cam);
    this->hl->addWidget(this->pb_close_cam);
    this->vl->addLayout(this->hl);
    this->gb_cam->setLayout(this->vl);

    this->hl = new QHBoxLayout;
    this->vl = new QVBoxLayout;
    this->hl->addWidget(this->wgt_camera);
    this->vl->addWidget(this->gb_cam);
    this->vl->addWidget(this->gb_clt);
    this->hl->addLayout(this->vl);
    this->setLayout(this->hl);

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
    imwrite((this->le_clt_path->text() + "/" + QString::number(id) + "_" + QString::number(this->id_map[id]) + ".png").toLatin1().constData(), m_img);
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
