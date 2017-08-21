#include "DlgLogin.h"
#include "ui_DlgLogin.h"
#include "SettingsManager.h"
#include "HubController.h"
#include "RestWorker.h"
#include <QSplashScreen>
#include "OsBranchConsts.h"

DlgLogin::DlgLogin(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgLogin),
  m_login_count(0)
{
  ui->setupUi(this);
  ui->lbl_status->setText("");
  ui->lbl_status->setVisible(false);

  ui->lbl_register_link->setText(QString("<a href=\"%1\">Register</a>").arg(hub_register_url()));
  ui->lbl_register_link->setTextFormat(Qt::RichText);
  ui->lbl_register_link->setTextInteractionFlags(Qt::TextBrowserInteraction);
  ui->lbl_register_link->setOpenExternalLinks(true);
  ui->lbl_register_link->setAlignment(Qt::AlignCenter);

  if (CSettingsManager::Instance().remember_me()) {
    ui->le_login->setText(CSettingsManager::Instance().login());
    ui->le_password->setText(CSettingsManager::Instance().password());
    ui->cb_save_credentials->setChecked(true);
  }

  connect(ui->btn_ok, &QPushButton::released, this, &DlgLogin::btn_ok_released);
  connect(ui->btn_cancel, &QPushButton::released, this, &DlgLogin::btn_cancel_released);
  connect(ui->cb_show_pass, &QCheckBox::stateChanged, this, &DlgLogin::cb_show_pass_state_changed);
}

DlgLogin::~DlgLogin()
{
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

bool
DlgLogin::login() {
  CSettingsManager::Instance().set_login(ui->le_login->text());
  CSettingsManager::Instance().set_password(ui->le_password->text());
  CSettingsManager::Instance().set_remember_me(ui->cb_save_credentials->checkState() == Qt::Checked);

  int http_code, err_code, network_err;
  CHubController::Instance().set_current_user(ui->le_login->text());
  CHubController::Instance().set_current_pass(ui->le_password->text());

  CRestWorker::Instance()->login(CHubController::Instance().current_user(),
                                 CHubController::Instance().current_pass(),
                                 http_code,
                                 err_code,
                                 network_err);

  QString id = "";
  switch (err_code) {
    case RE_SUCCESS:
      if (CRestWorker::Instance()->get_user_id(id))
        CHubController::Instance().set_current_user_id(id);

      ui->lbl_status->setText("");
      ui->lbl_status->setVisible(false);
      if (CSettingsManager::Instance().remember_me())
        CSettingsManager::Instance().save_all();
      QDialog::accept();
      return true;
    case RE_LOGIN_OR_EMAIL:
      ui->lbl_status->setVisible(true);
      ui->lbl_status->setText("<font color='red'>Wrong login or password. Try again!</font>");
      break;
    case RE_HTTP:
      ui->lbl_status->setVisible(true);
      ui->lbl_status->setText(QString("<font color='red'>HTTP error. Code : %1!</font>").arg(http_code));
      break;
    case RE_TIMEOUT:
      ui->lbl_status->setVisible(true);
      ui->lbl_status->setText("<font color='red'>Timeout. Check internet connection, please!</font>");
      break;
    case RE_NETWORK_ERROR:
      ui->lbl_status->setVisible(true);
      ui->lbl_status->setText(QString("<font color='red'>Network error. Code: %1!</font>").
                              arg(CCommons::NetworkErrorToString(network_err)));
      break;
    default:
      ui->lbl_status->setVisible(true);
      ui->lbl_status->setText(QString("<font color='red'>Unknown error. Code: %1!</font>").arg(err_code));
      break;
  }
  return false;
}
////////////////////////////////////////////////////////////////////////////

void
DlgLogin::run_dialog(QSplashScreen* sc) {
  if (!CSettingsManager::Instance().remember_me()) {
    if(sc) sc->hide();
    exec();
    return;
  }

  if (!login()) {
    if(sc) sc->hide();
    exec();
    return;
  }

  emit login_success();
  if(sc) sc->hide();
}
////////////////////////////////////////////////////////////////////////////

void
DlgLogin::btn_ok_released() {
  if (login())
    emit login_success();
}
////////////////////////////////////////////////////////////////////////////

void
DlgLogin::btn_cancel_released()
{
  this->setResult(QDialog::Rejected);
  QDialog::reject();
}
////////////////////////////////////////////////////////////////////////////

void
DlgLogin::cb_show_pass_state_changed(int st)
{
  ui->le_password->setEchoMode(st == Qt::Checked ?
                                 QLineEdit::PasswordEchoOnEdit : QLineEdit::Password);
}
////////////////////////////////////////////////////////////////////////////
