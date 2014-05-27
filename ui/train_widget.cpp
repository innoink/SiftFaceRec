#include "train_widget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>

train_widget::train_widget(QWidget *parent) :
    QWidget(parent)
{
    this->te_trained = new QTextEdit(this);
    this->pb_refresh = new QPushButton(tr("Refresh"));
    this->pb_trainall = new QPushButton(tr("Train All"));

    //te_trained->setMinimumSize(300, 300);

    QHBoxLayout *hl;
    QVBoxLayout *vl;
    hl = new QHBoxLayout;
    vl = new QVBoxLayout;
    vl->addWidget(te_trained);
    hl->addWidget(pb_refresh);
    hl->addWidget(pb_trainall);
    vl->addLayout(hl);
    setLayout(vl);

    connect(pb_refresh, &QPushButton::clicked, this, &train_widget::refresh);
    connect(pb_trainall, &QPushButton::clicked, this, &train_widget::train_all);
}

void train_widget::set_fc(face_collection *fc)
{
    enter_func();

    this->fc = fc;
    leave_func();

}

void train_widget::refresh()
{
    enter_func();

    te_trained->clear();
    std::vector<int> ids, faces;
    ids = fc->all_id();
    QString line;
    for (int i = 0; i < ids.size(); i++) {
        faces = fc->all_face(ids[i]);
        for (int j = 0; j < faces.size(); j++) {
            line.clear();
            line.append(QString("ID:%1 Face:%2 ").arg(ids[i]).arg(faces[j]));
            if (fc->is_trained(ids[i], faces[j])) {
                line.append(QString("Trained: True"));
            } else {
                line.append(QString("Trained: False"));
            }
            te_trained->append(line);
        }
    }
    leave_func();

}

void train_widget::train_all()
{
    enter_func();

    std::vector<int> ids, faces;
    ids = fc->all_id();
    for (int i = 0; i < ids.size(); i++) {
        faces = fc->all_face(ids[i]);
        for (int j = 0; j < faces.size(); j++) {
            if (fc->is_trained(ids[i], faces[j])) continue;
            Mat face;
            fc->get_face(ids[i], faces[j], face);
            std::vector<sift_keypoint_descr_t> kpds;
            fr.train(face, kpds);

            fc->set_train_data(ids[i], faces[j], kpds);
        }
    }
    leave_func();
}
