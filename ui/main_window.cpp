#include "main_window.h"

main_window::main_window(QWidget *parent) :
    QMainWindow(parent)
{
    /*
    this->wgt_camera->open_camera();
    this->wgt_camera->start_capture();
    */

    this->wgt_ft = new FancyTabWidget(this);
    this->wgt_clt = new collection_widget(this);
    this->wgt_train = new train_widget(this);
    this->setCentralWidget(this->wgt_ft);
    this->wgt_ft->insertTab(0, this->wgt_clt, QIcon(), tr("collection"));
    this->wgt_ft->insertTab(1, this->wgt_train, QIcon(), tr("train"));
    this->wgt_ft->setTabEnabled(0, true);
    this->wgt_ft->setTabEnabled(1, true);
}

main_window::~main_window()
{

}
