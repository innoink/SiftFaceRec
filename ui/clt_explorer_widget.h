#ifndef CLT_EXPLORER_WIDGET_H
#define CLT_EXPLORER_WIDGET_H

#include <QWidget>
#include <QTreeWidget>
#include <QMap>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QLineEdit>
#include "face_collection/face_collection.h"

class clt_explorer_widget : public QWidget
{
        Q_OBJECT
    public:
        explicit clt_explorer_widget(QWidget *parent = 0);
        void set_fc(face_collection *fc);

    signals:
        void new_id_done();

    public slots:
        void add_id(int id, QString &name);
        void add_face(int id, int face);

        void new_id();
        void del_face();
        void clear();
        void refresh();
        void show_img(QTreeWidgetItem * item, int column);

    private:
        QTreeWidget *tw_clt;
        QWidget     *wgt_right, *wgt_left;
        QPushButton *pb_newid, *pb_delid, *pb_delface;
        QSpinBox    *sb_id;
        QLineEdit   *le_name;

        QLabel *lb_img;

        QTreeWidgetItem *root;
        QMap<int, QTreeWidgetItem *> item_id_map;

        face_collection *fc;

        int cur_id, cur_face;

};

#endif // CLT_EXPLORER_WIDGET_H
