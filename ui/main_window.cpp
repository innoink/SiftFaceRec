#include "main_window.h"

main_window::main_window(QWidget *parent) :
    QMainWindow(parent)
{
    this->wgt_ft = new FancyTabWidget(this);
    this->wgt_clt = new collection_widget(this);
    this->wgt_train = new train_widget(this);
    this->wgt_sm = new sift_demo_widget(this);
    this->setCentralWidget(this->wgt_ft);
    this->wgt_ft->insertTab(0, this->wgt_clt, QIcon(), tr("collection"));
    this->wgt_ft->insertTab(1, this->wgt_train, QIcon(), tr("train"));
    this->wgt_ft->insertTab(2, this->wgt_sm, QIcon(), tr("SIFT demo"));
    this->wgt_ft->setTabEnabled(0, true);
    this->wgt_ft->setTabEnabled(1, true);
    this->wgt_ft->setTabEnabled(2, true);
}

main_window::~main_window()
{

}
