#include "clt_explorer_widget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QDebug>
#include "utils/img_fmt_cvt.h"

clt_explorer_widget::clt_explorer_widget(QWidget *parent) :
    QWidget(parent)
{

    this->wgt_right = new QWidget(this);
    this->wgt_left = new QWidget(this);
    this->pb_newid = new QPushButton(tr("Add"), this->wgt_left);
    this->pb_delface = new QPushButton(tr("Delete Face"), this->wgt_right);
    this->sb_id   = new QSpinBox(wgt_left);
    this->le_name = new QLineEdit(wgt_left);

    this->tw_clt = new QTreeWidget(wgt_left);
    this->tw_clt->setColumnCount(1);
    tw_clt->setHeaderLabel(tr("collection"));
    QStringList rootlabel;
    rootlabel.append(tr("root"));
    root = new QTreeWidgetItem(tw_clt, rootlabel);
    wgt_left->setMaximumWidth(300);

    this->lb_img = new QLabel(wgt_right);

    QHBoxLayout *hl;
    QVBoxLayout *vl;
    QGridLayout *gl;

    vl = new QVBoxLayout;
    gl = new QGridLayout;
    vl->addWidget(tw_clt);
    gl->addWidget(new QLabel(tr("ID:"), wgt_left), 0, 0);
    gl->addWidget(sb_id, 0, 1);
    gl->addWidget(new QLabel(tr("Name:"), wgt_left), 1, 0);
    gl->addWidget(le_name, 1, 1);
    vl->addLayout(gl);
    vl->addWidget(pb_newid);
    wgt_left->setLayout(vl);

    vl = new QVBoxLayout;
    vl->addWidget(lb_img);
    vl->addWidget(pb_delface);
    wgt_right->setLayout(vl);


    hl = new QHBoxLayout;
    hl->addWidget(wgt_left);
    hl->addWidget(wgt_right);
    this->setLayout(hl);

    pb_delface->setDisabled(true);

    connect(pb_delface, &QPushButton::clicked, this, &clt_explorer_widget::del_face);
    connect(pb_newid, &QPushButton::clicked, this, &clt_explorer_widget::new_id);
    connect(this, &clt_explorer_widget::new_id_done, this, &clt_explorer_widget::refresh);
    connect(tw_clt, &QTreeWidget::itemClicked, this, &clt_explorer_widget::show_img);
}

void clt_explorer_widget::set_fc(face_collection *fc)
{
    this->fc = fc;
}

void clt_explorer_widget::add_id(int id, QString &name)
{
    enter_func();
    QTreeWidgetItem *id_item;
    QStringList id_label;

    id_label.append(QString("%1(%2)").arg(id).arg(name));
    id_item = new QTreeWidgetItem(root, id_label);
    root->addChild(id_item);
    item_id_map.insert(id, id_item);
    leave_func();
}
void clt_explorer_widget::add_face(int id, int face)
{
    enter_func();

    QTreeWidgetItem *face_item;
    QStringList face_label;

    if (!item_id_map.contains(id)) return;

    face_label.append(QString("%1").arg(face));
    face_item = new QTreeWidgetItem(item_id_map[id], face_label);
    item_id_map[id]->addChild(face_item);

    leave_func();
}

void clt_explorer_widget::clear()
{
    enter_func();

    this->item_id_map.clear();
    this->lb_img->clear();
    delete this->root;
    QStringList rootlabel;
    rootlabel.append(tr("root"));
    root = new QTreeWidgetItem(tw_clt, rootlabel);

    leave_func();
}

void clt_explorer_widget::refresh()
{
    enter_func();

    this->clear();
    std::vector<int> ids = fc->all_id();
    QString name;
    for (int i = 0; i < ids.size(); ++i) {
        name = fc->id_name(ids[i]).c_str();
        this->add_id(ids[i], name);
        std::vector<int> faces;
        faces = fc->all_face(ids[i]);
        for (int j = 0; j < faces.size(); j++) {
            this->add_face(ids[i], faces[j]);
        }
    }

    leave_func();
}

void clt_explorer_widget::new_id()
{
    enter_func();

    int id = sb_id->value();
    QString name = le_name->text();
    if (fc->id_exist(id)) {
        //error
        qDebug() << "id exist";
        return;
    }
    fc->new_id(id, name.toStdString());
    emit this->new_id_done();
    leave_func();
}

void clt_explorer_widget::del_face()
{
    enter_func();

    fc->erase_face(cur_id, cur_face);
    refresh();
    leave_func();
}

void clt_explorer_widget::show_img(QTreeWidgetItem *item, int column)
{
    enter_func();

    pb_delface->setDisabled(true);
    lb_img->clear();
    if (item->text(column).contains('r')) return;
    if (item->text(column).contains('(')) return;
    cur_face = item->text(column).toInt();
    cur_id = item->parent()->text(column).left(item->parent()->text(column).indexOf('(')).toInt();
    QImage img;
    cv::Mat mimg;
    if (!fc->get_face(cur_id, cur_face, mimg)) return;
    mat2qimage(mimg, img);
    lb_img->setPixmap(QPixmap::fromImage(img));
    pb_delface->setEnabled(true);
    leave_func();
}
