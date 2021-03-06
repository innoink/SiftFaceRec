#ifndef TRAIN_WIDGET_H
#define TRAIN_WIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include "face_collection/face_collection.h"
#include "face_recognizer/face_recognizer.h"

class train_widget : public QWidget
{
        Q_OBJECT
    public:
        explicit train_widget(QWidget *parent = 0);
        void set_fc(face_collection *fc);

    signals:

    public slots:

    private slots:
        void refresh();
        void train_all();
    private:
        QTextEdit *te_trained;
        QPushButton *pb_refresh, *pb_trainall;

        face_collection *fc;
        face_recognizer fr;

};

#endif // TRAIN_WIDGET_H
