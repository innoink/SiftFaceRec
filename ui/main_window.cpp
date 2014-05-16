#include "main_window.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QThread>
#include <QDebug>

main_window::main_window(QWidget *parent) :
    QMainWindow(parent)
{
    this->wgt_ft = new FancyTabWidget(this);
    this->wgt_clt = new collection_widget(this);
    this->wgt_train = new train_widget(this);
    this->wgt_sm = new sift_demo_widget(this);
    this->wgt_cltexp = new clt_explorer_widget(this);
    this->wgt_match = new match_widget(this);
    this->setCentralWidget(this->wgt_ft);
    this->wgt_ft->insertTab(0, this->wgt_clt, QIcon(), tr("collection"));
    this->wgt_ft->insertTab(1, this->wgt_cltexp, QIcon(), tr("explorer"));
    this->wgt_ft->insertTab(2, this->wgt_train, QIcon(), tr("train"));
    this->wgt_ft->insertTab(3, this->wgt_sm, QIcon(), tr("SIFT demo"));
    this->wgt_ft->insertTab(4, this->wgt_match, QIcon(), tr("match"));
    this->wgt_ft->setTabEnabled(0, true);
    this->wgt_ft->setTabEnabled(1, true);
    this->wgt_ft->setTabEnabled(2, true);
    this->wgt_ft->setTabEnabled(3, true);
    this->wgt_ft->setTabEnabled(4, true);

    this->wgt_clt->set_fc(&fc);
    this->wgt_cltexp->set_fc(&fc);
    this->wgt_train->set_fc(&fc);
    this->wgt_match->set_fc(&fc);
    this->clt_recent_path = QString("./");

    create_actions();
    create_menus();
    connect(this, &main_window::clt_loaded, wgt_clt, &collection_widget::clt_loaded);
    connect(this, &main_window::clt_closed, wgt_clt, &collection_widget::clt_closed);
    connect(this, &main_window::clt_loaded,
            [this]()
            {
                act_load->setDisabled(true);
                act_close->setEnabled(true);
            });
    connect(this, &main_window::clt_closed,
            [this]()
            {
                act_load->setEnabled(true);
                act_close->setDisabled(true);
            });
    connect(this, &main_window::clt_loaded, wgt_cltexp, &clt_explorer_widget::refresh);
    connect(this, &main_window::clt_closed, wgt_cltexp, &clt_explorer_widget::clear);
    connect(wgt_clt, &collection_widget::take_done, wgt_cltexp, &clt_explorer_widget::refresh);
    qDebug() << QThread::currentThreadId();
}

main_window::~main_window()
{
fc.close();
}

void main_window::create_actions()
{
    act_new = new QAction(tr("&New"), this);
    connect(act_new, &QAction::triggered, this, &main_window::new_collection);

    act_load = new QAction(tr("Load..."), this);
    connect(act_load, &QAction::triggered, this, &main_window::load_collection);

    act_close = new QAction(tr("&Close"), this);
    connect(act_close, &QAction::triggered, this, &main_window::close_collection);
    act_close->setDisabled(true);
}

void main_window::create_menus()
{
    mn_file = menuBar()->addMenu(tr("&File"));
    mn_file->addAction(act_new);
    mn_file->addAction(act_load);
    mn_file->addAction(act_close);

}

void main_window::new_collection()
{
    bool is_created = false;
    fc_create_dlg *fccdlg;
    fccdlg = new fc_create_dlg(&is_created, this);
    fccdlg->exec();
}

void main_window::load_collection()
{
     QString clt_path;
     clt_path = QFileDialog::getOpenFileName(this, tr("choose collection"), clt_recent_path, tr("Face Collection Files (*.fc)"));
     if (clt_path.isEmpty()) return;
     //clt_recent_path = clt_path.
     fc.close();
     if (!fc.load(clt_path.toStdString())) {
         QMessageBox::critical(NULL, tr("Load Collection Error"), fc.error_string().c_str(), QMessageBox::Yes, QMessageBox::Yes);
         return;
     }
     emit this->clt_loaded();
}

void main_window::close_collection()
{
    fc.close();
    emit this->clt_closed();
}

