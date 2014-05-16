#ifndef MATCH_WIDGET_H
#define MATCH_WIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include "face_collection/face_collection.h"
#include "face_recognizer/face_recognizer.h"

class match_widget : public QWidget
{
        Q_OBJECT
    public:
        explicit match_widget(QWidget *parent = 0);
        void set_fc(face_collection *fc);

    signals:

    public slots:
    private slots:
        void choose();
        void match();
    private:
        QPushButton *pb_choose, *pb_match;
        QTextEdit   *te_info;
        QLabel      *lb_img;

        face_collection *fc;
        face_recognizer fr;

        Mat img;

};

#endif // MATCH_WIDGET_H
