#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include "ui/fancy_widget/fancytabwidget.h"
#include "ui/collection_widget.h"
#include "ui/train_widget.h"
#include "ui/sift_demo_widget.h"
#include "ui/clt_explorer_widget.h"
#include "ui/match_widget.h"
#include "face_collection/face_collection.h"

class main_window : public QMainWindow
{
        Q_OBJECT
    public:
        explicit main_window(QWidget *parent = 0);
        ~main_window();

    signals:
        void clt_loaded();
        void clt_closed();

    private slots:
        void new_collection();
        void load_collection();
        void close_collection();
    private:

        FancyTabWidget *wgt_ft;
        collection_widget *wgt_clt;
        train_widget  * wgt_train;
        sift_demo_widget *wgt_sm;
        clt_explorer_widget *wgt_cltexp;
        match_widget *wgt_match;

        void create_menus();
        QMenu *mn_file;

        void create_actions();
        QAction *act_new;
        QAction *act_load;
        QAction *act_close;

        QString clt_recent_path;


        face_collection fc;
};

#endif // MAIN_WINDOW_H
