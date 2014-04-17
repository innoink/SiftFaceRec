#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include "ui/fancy_widget/fancytabwidget.h"
#include "ui/collection_widget.h"
#include "ui/train_widget.h"

class main_window : public QMainWindow
{
        Q_OBJECT
    public:
        explicit main_window(QWidget *parent = 0);
        ~main_window();

    signals:

    public slots:
    private:
        FancyTabWidget *wgt_ft;
        collection_widget *wgt_clt;
        train_widget  * wgt_train;
};

#endif // MAIN_WINDOW_H
