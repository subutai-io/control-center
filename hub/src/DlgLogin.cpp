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
  connect(ui->cb_show_pass, SIGNAL(stateChanged(int)), this, SLOT(cb_show_pass_state_changed(int)));
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

  int login_res = CRestWorker::login(ui->le_login->text(), ui->le_password->text());

  switch (login_res) {
    case EL_SUCCESS:
      ui->lbl_status->setText("");
      ui->lbl_status->setVisible(false);
      if (CSettingsManager::Instance().remember_me())
        CSettingsManager::Instance().save_all();
      QDialog::accept();
      break;
    case EL_LOGIN_OR_EMAIL:
      ui->lbl_status->setVisible(true);
      ui->lbl_status->setText("<font color='red'>Wrong login or password. Try again!</font>");
      break;
    case EL_HTTP:
      ui->lbl_status->setVisible(true);
      ui->lbl_status->setText("<font color='red'>HTTP error. Check settings, please!</font>");
      break;
    case EL_TIMEOUT:
      ui->lbl_status->setVisible(true);
      ui->lbl_status->setText("<font color='red'>Timeout. Check internet connection, please!</font>");
      break;
    default:
      ui->lbl_status->setVisible(true);
      ui->lbl_status->setText(QString("<font color='red'>Network error. Code: %1!</font>").arg(login_res));
      break;
  }
}
////////////////////////////////////////////////////////////////////////////

void DlgLogin::btn_cancel_released()
{
  this->setResult(QDialog::Rejected);
  QDialog::reject();
}
////////////////////////////////////////////////////////////////////////////

void DlgLogin::cb_show_pass_state_changed(int st)
{
  ui->le_password->setEchoMode(st == Qt::Checked ?
                                 QLineEdit::PasswordEchoOnEdit : QLineEdit::Password);
}
////////////////////////////////////////////////////////////////////////////
