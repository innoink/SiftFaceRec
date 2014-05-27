#include "match_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include "utils/img_fmt_cvt.h"
#include <QThread>
#include <QDebug>

match_widget::match_widget(QWidget *parent) :
    QWidget(parent)
{
    pb_choose = new QPushButton(tr("choose"), this);
    pb_match = new QPushButton(tr("match"), this);
    lb_img = new QLabel(this);
    te_info = new QTextEdit(this);

    QHBoxLayout *hl;
    QVBoxLayout *vl;

    hl = new QHBoxLayout;
    vl = new QVBoxLayout;

    vl->addWidget(pb_choose);
    vl->addWidget(pb_match);
    hl->addLayout(vl);
    hl->addWidget(lb_img);
    hl->addWidget(te_info);

    setLayout(hl);
    connect(pb_choose, SIGNAL(clicked()), this, SLOT(choose()));
    connect(pb_match, SIGNAL(clicked()), this, SLOT(match()));

}
void match_widget::set_fc(face_collection *fc)
{
    this->fc = fc;
}

void match_widget::choose()
{
    QString pic;
    pic = QFileDialog::getOpenFileName(this, tr("choose pic"), tr("./"), tr("Image Files (*.png *.jpg *.bmp *.pgm)"));
    if (pic.isEmpty()) return;
    this->img = imread(pic.toLocal8Bit().constData(), CV_LOAD_IMAGE_GRAYSCALE);
    QImage qimg;
    mat2qimage(img, qimg);
    this->lb_img->setPixmap(QPixmap::fromImage(qimg));
}
void match_widget::match()
{
    enter_func();
    std::vector<sift_keypoint_descr_t> kpds_test;
    std::vector<int> ids;
    te_info->clear();
    fr.train(img, kpds_test);
    print_kpds(kpds_test);
    puts("=================================================================");
    te_info->append(QString("test img:\nkpds cnt=>%1").arg(kpds_test.size()));
    ids = fc->all_id();
    for (int i = 0; i < ids.size(); i++) {
        std::vector<int> faces;
        faces = fc->all_face(ids[i]);
        for (int j = 0; j < faces.size(); j++) {
            int mcnt = 0;
            std::vector<sift_keypoint_descr_t> kpds_clt;
            fc->get_train_data(ids[i], faces[j], kpds_clt);
            print_kpds(kpds_clt);
            fr.match(false, kpds_test, kpds_clt, 0.75, &mcnt);
            QString info = QString("ID:%1 Face:%2 kpds cnt:%3 match cnt:%4")
                           .arg(ids[i]).arg(faces[j]).arg(kpds_clt.size()).arg(mcnt);
            mcnt = 0;

            fr.match(true, kpds_test, kpds_clt, 0.75, &mcnt);
            info += QString("after :%1").arg(mcnt);
            te_info->append(info);
        }
    }
    leave_func();
}
