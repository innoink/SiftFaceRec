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

fc_create_dlg::fc_create_dlg(bool *is_created, face_collection &fc, QWidget *parent) :
    QDialog(parent)
{
    this->le_name = new QLineEdit(this);
    this->le_path = new QLineEdit(this);
    this->pb_path = new QPushButton(tr("choose path:"), this);
    this->pb_create = new QPushButton(tr("Create"), this);
    this->pb_cancle = new QPushButton(tr("Cancle"), this);

    QVBoxLayout *vl = new QVBoxLayout;
    QHBoxLayout *hl = new QHBoxLayout;
    hl->addWidget(new QLabel(tr("Collection Name:"), this));
    hl->addWidget(this->le_name);
    vl->addLayout(hl);
    hl = new QHBoxLayout;
    hl->addWidget(new QLabel(tr("Collection Path:"), this));
    hl->addWidget(this->le_path);
    hl->addWidget(this->pb_path);
    vl->addLayout(hl);
    hl = new QHBoxLayout;
    hl->addWidget(this->pb_create);
    hl->addWidget(this->pb_cancle);
    vl->addLayout(hl);

    this->setLayout(vl);

    connect(this->le_name, &QLineEdit::textChanged,
            [this]()
            {
                if (this->le_name->text().isEmpty() || this->le_path->text().isEmpty()) {
                    this->pb_create->setDisabled(true);
                } else {
                    this->pb_create->setEnabled(true);
                }
            });

    connect(this->le_path, &QLineEdit::textChanged,
            [this]()
            {
                if (this->le_name->text().isEmpty() || this->le_path->text().isEmpty()) {
                    this->pb_create->setDisabled(true);
                } else {
                    this->pb_create->setEnabled(true);
                }
            });

    connect(this->pb_path, &QPushButton::clicked,
            [this]()
            {
                QString path;
                path = QFileDialog::getSaveFileName(this, tr("choose path"), "./unnamed.fc", tr("Face Collection Files (*.fc)"));
                if (path.isEmpty()) return;
                this->le_path->setText(path);
            });

    connect(this->pb_create, &QPushButton::clicked,
            [&]()
            {
                fc.close();
                if (!fc.create(this->le_name->text().toStdString(), this->le_path->text().toStdString())) {
                    QMessageBox::critical(NULL, tr("Creation Failed!"), fc.error_string().c_str(), QMessageBox::Yes, QMessageBox::Yes);
                    *is_created = false;
                    this->close();
                } else {
                    *is_created = true;
                    this->close();

                }
            });
    connect(this->pb_cancle, &QPushButton::clicked,
            [&]()
            {
                *is_created = false;
                this->close();
            });
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
    this->pb_load_clt = new QPushButton(tr("Load"), this->gb_clt);
    this->pb_create_clt = new QPushButton(tr("Create"), this->gb_clt);
    this->pb_close_clt = new QPushButton(tr("close"), this->gb_clt);
    this->lb_clt_file = new QLabel(this->gb_clt);
    this->lb_clt_name = new QLabel(this->gb_clt);
    this->sb_clt_id = new QSpinBox(this->gb_clt);
    this->pb_take = new QPushButton(tr("take"), this->gb_clt);

    vl = new QVBoxLayout;
    gl = new QGridLayout;
    hl = new QHBoxLayout;
    hl->addWidget(this->pb_load_clt);
    hl->addWidget(this->pb_create_clt);
    hl->addWidget(this->pb_close_clt);
    gl->addWidget(new QLabel(tr("Collection file:"), this->gb_clt), 0, 0, Qt::AlignLeft);
    gl->addWidget(this->lb_clt_file, 0, 1, Qt::AlignRight);
    gl->addWidget(new QLabel(tr("Collection name:"), this->gb_clt), 1, 0, Qt::AlignLeft);
    gl->addWidget(this->lb_clt_name, 1, 1, Qt::AlignRight);
    gl->addWidget(new QLabel(tr("ID:"), this->gb_clt), 2, 0, Qt::AlignLeft);
    gl->addWidget(this->sb_clt_id, 2, 1, Qt::AlignRight);
    vl->addLayout(hl);
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

    this->pb_close_cam->setDisabled(true);
    this->pb_close_clt->setDisabled(true);
    this->pb_take->setDisabled(true);
    this->lb_clt_file->setTextInteractionFlags(Qt::TextSelectableByMouse);
    this->lb_clt_name->setTextInteractionFlags(Qt::TextSelectableByMouse);
    this->wgt_right->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    this->clt_recent_path = QString("./");

    connect(this->pb_open_cam, &QPushButton::clicked, this, &collection_widget::open_cam);
    connect(this->pb_close_cam, &QPushButton::clicked, this, &collection_widget::close_cam);
    connect(this->pb_load_clt, &QPushButton::clicked, this, &collection_widget::load_clt);
    connect(this->pb_create_clt, &QPushButton::clicked, this, &collection_widget::create_clt);
    connect(this->pb_close_clt, &QPushButton::clicked, this, &collection_widget::close_clt);
    connect(this->pb_take, &QPushButton::clicked, this, &collection_widget::take);

    connect(this, &collection_widget::clt_loaded,
            [this]()
            {
                this->pb_load_clt->setDisabled(true);
                this->pb_create_clt->setDisabled(true);
                this->pb_close_clt->setEnabled(true);
                QString fname(this->fc.file_path().c_str());
                this->lb_clt_file->setText(fname.right(fname.length() - fname.lastIndexOf('/') - 1));
                std::string name;
                this->fc.clt_name(name);
                this->lb_clt_name->setText(name.c_str());
                this->pb_take->setEnabled(true);
            });
    connect(this, &collection_widget::clt_closed,
            [this]()
            {
                this->pb_load_clt->setEnabled(true);
                this->pb_create_clt->setEnabled(true);
                this->pb_close_clt->setDisabled(true);
                this->lb_clt_file->setText("");
                this->lb_clt_name->setText("");
                this->pb_take->setDisabled(true);
            });
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

void collection_widget::load_clt()
{
    QString clt_path;
    clt_path = QFileDialog::getOpenFileName(this, tr("choose collection"), this->clt_recent_path, tr("Face Collection Files (*.fc)"));
    if (clt_path.isEmpty()) return;
    fc.close();
    if (!fc.load(clt_path.toStdString())) {
        QMessageBox::critical(NULL, tr("Load Collection Error"), fc.error_string().c_str(), QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
    emit this->clt_loaded();
}

void collection_widget::create_clt()
{
    bool is_created = false;
    fc_create_dlg *fccdlg;
    fccdlg = new fc_create_dlg(&is_created, this->fc, this);
    fccdlg->exec();
    if (is_created) {
        emit this->clt_loaded();
    }
}

void collection_widget::close_clt()
{
    this->fc.close();
    emit this->clt_closed();
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

    m_tmp = this->wgt_camera->get_frame_mat().clone();
    cvtColor(m_tmp, m_tmp, COLOR_BGR2GRAY);
    iplframe = m_tmp;
    fd.set_input_image(&iplframe);
    fd.get_face_parameters(face_marked);
    mat2qimage(face_marked, face_marked_qimg);
    fdw = new face_detected_dlg(&usy, face_marked_qimg, this);
    fdw->exec();
    if (usy) {
    } else {
        delete fdw;
        return;
    }
    delete fdw;
}
