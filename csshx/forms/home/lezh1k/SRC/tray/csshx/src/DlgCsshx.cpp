#include "/home/lezh1k/SRC/tray/csshx/include/DlgCsshx.h"
#include "ui_DlgCsshx.h"

DlgCsshx::DlgCsshx(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgCsshx)
{
  ui->setupUi(this);
}

DlgCsshx::~DlgCsshx()
{
  delete ui;
}
