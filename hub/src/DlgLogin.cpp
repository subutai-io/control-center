#include "DlgLogin.h"
#include "ui_DlgLogin.h"
#include "SettingsManager.h"
#include "HubController.h"
#include "RestWorker.h"
#include <QSplashScreen>
#include <QAction>
#include "OsBranchConsts.h"
#include "NotificationObserver.h"

DlgLogin::DlgLogin(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgLogin),
  m_login_count(0),
  m_password_state(0)
{
  ui->setupUi(this);
  ui->lbl_status->setText("");
  ui->lbl_status->setVisible(false);
  ui->btn_resolve->hide();
  ui->btn_ok->setEnabled(true);

  ui->lbl_register_link->setText(QString("<a href=\"%1\" style=\"color: inherit;\">%2</a>").arg(hub_register_url()).arg(tr("Register for Bazaar account")));
  ui->lbl_register_link->setTextFormat(Qt::RichText);
  ui->lbl_register_link->setTextInteractionFlags(Qt::TextBrowserInteraction);
  ui->lbl_register_link->setOpenExternalLinks(true);
  static QIcon show_password_icon(":/hub/show-password.png");
  static QIcon hide_password_icon(":/hub/hide-password.png");

  this->m_show_password_action = ui->le_password->addAction(show_password_icon,
                                                            QLineEdit::TrailingPosition);

  connect(this->m_show_password_action, &QAction::triggered, [this]() {
      this->m_password_state ^= 1; // xor
      this->m_show_password_action->setIcon(
          this->m_password_state ? hide_password_icon : show_password_icon);
      ui->le_password->setEchoMode(this->m_password_state ? QLineEdit::Normal : QLineEdit::Password);
  });

  if (CSettingsManager::Instance().remember_me()) {
    ui->le_login->setText(CSettingsManager::Instance().login());
    ui->le_password->setText(CSettingsManager::Instance().password());
    ui->cb_save_credentials->setChecked(true);
  }

  connect(ui->btn_ok, &QPushButton::released, this, &DlgLogin::btn_ok_released);
  connect(ui->btn_cancel, &QPushButton::released, this, &DlgLogin::btn_cancel_released);
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
  qDebug() << "Username " << ui->le_login->text();
  qDebug() << "Remember Me " << ui->cb_save_credentials->text();

  int http_code, err_code, network_err;
  CHubController::Instance().set_current_user(ui->le_login->text());
  CHubController::Instance().set_current_pass(ui->le_password->text());

  CRestWorker::Instance()->login(CHubController::Instance().current_user(),
                                 CHubController::Instance().current_pass(),
                                 http_code,
                                 err_code,
                                 network_err);

  QString id = "";
  QString email = "";
  switch (err_code) {
    case RE_SUCCESS:
      if (CRestWorker::Instance()->get_user_info("id", id))
        CHubController::Instance().set_current_user_id(id);

      if (CRestWorker::Instance()->get_user_info("email", email))
          CHubController::Instance().set_current_email(email);

      ui->lbl_status->setText("");
      ui->lbl_status->setVisible(false);
      if (CSettingsManager::Instance().remember_me())
        CSettingsManager::Instance().save_all();
      QDialog::accept();
      return true;
    case RE_LOGIN_OR_EMAIL:
      ui->lbl_status->setVisible(true);
      ui->lbl_status->setText(QString("<font color='red'>%1</font>").
                              arg(tr("Wrong username or password. Log in with your correct credentials.")));
      break;
    case RE_HTTP:
      ui->lbl_status->setVisible(true);
      ui->lbl_status->setText(QString("<font color='red'>%1 : %2</font>").
                              arg(tr("HTTP error. Code")).arg(http_code));
      break;
    case RE_TIMEOUT:
      ui->lbl_status->setVisible(true);
      ui->lbl_status->setText(QString("<font color='red'>%1</font>").
                              arg(tr("The session has timed out. Check your Internet connection, and then try again.")));
      break;
    case RE_NETWORK_ERROR:
      ui->lbl_status->setVisible(true);
      if (!QSslSocket::supportsSsl()) {
        ui->lbl_status->setText(QString("<font color='red'>%1 %2</font>").
                                arg(tr("There is an OpenSSL compatibility problem.")).
                                arg(tr("Press \"Resolve\" to install proper OpenSSL version.")));
        solve_libssl();
      } else {
        ui->lbl_status->setText(QString("<font color='red'>%1 : %2</font>").
                                arg(tr("Network error. Code")).
                                arg(CCommons::NetworkErrorToString(network_err)));
      }
      break;
    default:
      ui->lbl_status->setVisible(true);
      ui->lbl_status->setText(QString("<font color='red'>%1 : %2</font>").
                              arg(tr("Unknown error. Code")).
                              arg(err_code));
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

void DlgLogin::solve_libssl(){
    std::vector<std::pair<QString, QString> > info;
    current_os_info(info);
    if(info.empty())
        return;
    if(info.size() < 2)
        return;
    ui->btn_resolve->show();
    connect(ui->btn_resolve, &QPushButton::released, [this](){
        this->ui->btn_resolve->setText(tr("Resolving..."));
        system_call_wrapper_error_t res = CSystemCallWrapper::install_libssl();
        if(res == SCWE_SUCCESS)
            CCommons::RestartTray();
        this->ui->btn_resolve->setText(tr("Resolve"));
        ui->btn_resolve->hide();
    });
}
