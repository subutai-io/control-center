#include "DlgCreatePeer.h"
#include "ui_DlgCreatePeer.h"

DlgCreatePeer::DlgCreatePeer(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgCreatePeer)
{

  ui->setupUi(this);
}

DlgCreatePeer::~DlgCreatePeer()
{
  delete ui;
}
