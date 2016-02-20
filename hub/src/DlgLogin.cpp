#include "DlgLogin.h"
#include "ui_DlgLogin.h"
#include "SettingsManager.h"


DlgLogin::DlgLogin(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgLogin),
  m_login_count(0)
{
  ui->setupUi(this);
  ui->lbl_status->setText("");
  ui->lbl_status->setVisible(false);

  if (CSettingsManager::Instance().remember_me()) {
    ui->le_login->setText(CSettingsManager::Instance().login());
    ui->le_password->setText(CSettingsManager::Instance().password());
    ui->cb_save_credentials->setChecked(true);
  }

  connect(ui->btn_ok, SIGNAL(released()), this, SLOT(btn_ok_released()));
  connect(ui->btn_cancel,  SIGNAL(released()), this, SLOT(btn_cancel_released()));
}

DlgLogin::~DlgLogin()
{
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

void DlgLogin::btn_ok_released()
{
  CSettingsManager::Instance().set_login(ui->le_login->text());
  CSettingsManager::Instance().set_password(ui->le_password->text());
  CSettingsManager::Instance().set_remember_me(ui->cb_save_credentials->checkState() == Qt::Checked);

  if (CRestWorker::login(ui->le_login->text(), ui->le_password->text()) == 0/* ||
      (ui->le_login->text() == "ff" && ui->le_password->text() == "ff")*/) {
    ui->lbl_status->setText("");
    ui->lbl_status->setVisible(false);
    if (CSettingsManager::Instance().remember_me())
      CSettingsManager::Instance().save_all();
    QDialog::accept();
  } else {
    ui->lbl_status->setVisible(true);
    ui->lbl_status->setText("<font color='red'>Wong login or password. Try again!</font>");
  }
}
////////////////////////////////////////////////////////////////////////////

void DlgLogin::btn_cancel_released()
{
  this->setResult(QDialog::Rejected);
  QDialog::reject();
}
////////////////////////////////////////////////////////////////////////////
