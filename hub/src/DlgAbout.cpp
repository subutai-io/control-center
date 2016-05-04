#include "DlgAbout.h"
#include "ui_DlgAbout.h"
#include "SystemCallWrapper.h"

static const QString str_tray_fmt   ("Tray version\t: %1");
static const QString str_p2p_fmt    ("P2P version\t: %1");
static const QString str_chrome_fmt ("Chrome version\t: %1");
static const QString str_vbox_fmt   ("VBox version\t: %1");

DlgAbout::DlgAbout(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgAbout)
{
  ui->setupUi(this);

  ui->lbl_tray_version->setText(str_tray_fmt.arg(QString(GIT_VERSION)));
  std::string str_version;
  int exit_code;

  system_call_wrapper_error_t res =
      CSystemCallWrapper::p2p_version(str_version, exit_code);

  ui->lbl_p2p_version->setText(str_p2p_fmt.arg(QString::fromStdString(str_version)));
  res = CSystemCallWrapper::chrome_version(str_version, exit_code);
  ui->lbl_chrome_version->setText(str_chrome_fmt.arg(QString::fromStdString(str_version)));
  ui->lbl_vbox_version->setText(str_vbox_fmt.arg(CSystemCallWrapper::virtual_box_version()));
}

DlgAbout::~DlgAbout()
{
  delete ui;
}
