#include "DlgRegisterPeer.h"
#include "ui_DlgRegisterPeer.h"
#include <QDebug>
#include <QMovie>
#include <QLineEdit>
#include <QObject>

#include "SystemCallWrapper.h"
#include "TrayControlWindow.h"
#include "RhController.h"

int DlgRegisterPeer::dialog_used [10000] = {0};
int DlgRegisterPeer::dialog_running [10000] = {0};

DlgRegisterPeer::DlgRegisterPeer(QWidget *parent) :
    QDialog(parent),
    m_password_state(0),
    ui(new Ui::DlgRegisterPeer){

    ui->setupUi(this);
    this->setMinimumWidth( this->width());
    connect(ui->btn_cancel, &QPushButton::clicked,
            [this](){ this->close(); });
    ui->lne_username->setText("admin");
    m_invalid_chars.setPattern("\\W");

    // QLineEdit Show password action
    static QIcon show_password_icon(":/hub/show-password.png");
    static QIcon hide_password_icon(":/hub/hide-password.png");
    this->m_show_password_action = ui->lne_password->addAction(show_password_icon,
                                                                QLineEdit::TrailingPosition);

    connect(this->m_show_password_action, &QAction::triggered, [this]() {
       this->m_password_state ^= 1;
       this->m_show_password_action->setIcon(
            this->m_password_state ? hide_password_icon : show_password_icon);
       ui->lne_password->setEchoMode(m_password_state ? QLineEdit::Normal : QLineEdit::Password);
    });
}

DlgRegisterPeer::~DlgRegisterPeer()
{
    dialog_used[ip_addr.toInt() - 9999] = 0;
    delete ui;
}

void DlgRegisterPeer::registerPeer() {
    dialog_running[ip_addr.toInt() - 9999] = 1;
    qInfo()
            << "Register button pressed: "
            << ip_addr;
    ui->lbl_info->setVisible(false);
    if (ui->lne_peername->text().isEmpty()) {
        ui->lne_peername->setFocus();
        ui->lbl_info->setVisible(true);
        ui->lbl_info->setText(QString("<font color='red'>%1</font>").
                                arg(tr("Peer name can't be empty")));
        return;
    } else if (ui->lne_peername->text().contains(m_invalid_chars) ||
              ui->lne_peername->text().contains("_")) {
        ui->lne_peername->setFocus();
        ui->lbl_info->setVisible(true);
        ui->lbl_info->setText(tr("You can use only letters and numbers"));
        ui->lbl_info->setStyleSheet("QLabel {color : red}");
        return;
    }

    ui->btn_register->setEnabled(false);
    ui->btn_cancel->setEnabled(false);
    ui->lne_password->setEnabled(false);
    ui->lne_peername->setEnabled(false);
    ui->lne_username->setEnabled(false);
    ui->cmb_peer_scope->setEnabled(false);

    const QString login = ui->lne_username->text();
    const QString password = ui->lne_password->text();
    QString token;
    static int err_code, http_code, network_error;
    CRestWorker::Instance()->peer_token(m_url_management, login, password,
                                        token, err_code, http_code, network_error);
    const QString peer_name = ui->lne_peername->text();
    const QString peer_scope = ui->cmb_peer_scope->currentText();

    qDebug()
            << "Register peer: Get token errors: "
            << "Http code " << http_code
            << "Error code " << err_code
            << "Network Error " << network_error;
    if(!dialog_used[ip_addr.toInt() - 9999]) return;
    bool kill_me = check_errors(err_code, http_code, network_error);
    if(kill_me){
        CRestWorker::Instance()->peer_register(ip_addr, token,
                                               CHubController::Instance().current_email(), CHubController::Instance().current_pass(),
                                               peer_name, peer_scope,
                                               err_code, http_code, network_error);
        if(!dialog_used[ip_addr.toInt() - 9999]) return;
        bool kill_me = check_errors(err_code, http_code, network_error);
        if (kill_me){
            CHubController::Instance().force_refresh();
            emit register_finished();
            dialog_running[ip_addr.toInt() - 9999] = 0;
            this->close();
        }
    }
    dialog_running[ip_addr.toInt() - 9999] = 0;
    ui->btn_cancel->setEnabled(true);
    ui->btn_register->setEnabled(true);
    ui->lne_password->setEnabled(true);
    ui->lne_peername->setEnabled(true);
    ui->lne_username->setEnabled(true);
    ui->cmb_peer_scope->setEnabled(true);
}

void DlgRegisterPeer::unregisterPeer(){
    dialog_running[ip_addr.toInt() - 9999] = 1;
    qInfo()
            << "Unregister button pressed: "
            << ip_addr;

    const QString login=ui->lne_username->text();
    const QString password=ui->lne_password->text();
    QString token;
    static int err_code, http_code, network_error;
    ui->btn_cancel->setEnabled(false);
    ui->btn_unregister->setEnabled(false);
    ui->lne_password->setEnabled(false);
    ui->lne_username->setEnabled(false);

    CRestWorker::Instance()->peer_token(m_url_management, login, password,
                                            token, err_code, http_code, network_error);
    qDebug()
            << "Unregister peer: Get token errors: "
            << "Http code " << http_code
            << "Error code " << err_code
            << "Network Error " << network_error;

    if(!dialog_used[ip_addr.toInt() - 9999]) return;
    bool kill_me = check_errors(err_code, http_code, network_error);
    if(kill_me){
        CRestWorker::Instance()->peer_unregister(m_url_management, token, err_code, http_code, network_error);
        if(!dialog_used[ip_addr.toInt() - 9999]) return;
        kill_me = check_errors(err_code, http_code, network_error);
        if (kill_me){
            dialog_running[ip_addr.toInt() - 9999] = 0;
            CHubController::Instance().force_refresh();
            emit register_finished();
            this->close();
        }
    }
    dialog_running[ip_addr.toInt() - 9999] = 0;
    ui->btn_cancel->setEnabled(true);
    ui->btn_unregister->setEnabled(true);
    ui->lne_password->setEnabled(true);
    ui->lne_username->setEnabled(true);
}

void DlgRegisterPeer::setUnregistrationMode(){
    qInfo()
        << "Unregister peer dialog created: "
        << ip_addr;

    ui->btn_register->hide();
    ui->btn_unregister->show();

    ui->btn_register->setDefault(false);
    ui->btn_unregister->setDefault(true);

    ui->lne_peername->hide();
    ui->lbl_peer_name->hide();
    ui->lbl_peer_scope->hide();
    ui->lne_password->setEchoMode(QLineEdit::Password);
    ui->cmb_peer_scope->hide();
    ui->lbl_info->hide();

    this->adjustSize();

    connect(ui->btn_unregister, &QPushButton::released,
            this, &DlgRegisterPeer::unregisterPeer);
}

void DlgRegisterPeer::setRegistrationMode(){
    qInfo()
        << "Register peer dialog created: "
        << ip_addr;

    ui->btn_unregister->hide();
    ui->btn_register->show();

    ui->btn_register->setDefault(true);
    ui->btn_unregister->setDefault(false);

    ui->lne_password->setEchoMode(QLineEdit::Password);
    ui->lbl_info->hide();

    this->adjustSize();

    connect(ui->btn_register, &QPushButton::clicked,
            this, &DlgRegisterPeer::registerPeer);
}

bool DlgRegisterPeer::check_errors(const int &err_code,
                                   const int &http_code,
                                   const int &network_error){
  switch (err_code) {
    case RE_SUCCESS:
      return true;
    case RE_LOGIN_OR_EMAIL:
      ui->lbl_info->setVisible(true);
      ui->lbl_info->setText(QString("<font color='red'>%1</font>").
                              arg(tr("Wrong login or password. Try again!")));
      break;
    case RE_HTTP:
      ui->lbl_info->setVisible(true);
      ui->lbl_info->setText(QString("<font color='red'>%1 : %2</font>").
                              arg(tr("HTTP error. Code")).arg(http_code));
      break;
    case RE_TIMEOUT:
      ui->lbl_info->setVisible(true);
      ui->lbl_info->setText(QString("<font color='red'>%1</font>").
                              arg(tr("Timeout. Check internet connection, please!")));
      break;
    case RE_NETWORK_ERROR:
      ui->lbl_info->setVisible(true);
      ui->lbl_info->setText(QString("<font color='red'>%1 : %2</font>").
                              arg(tr("Network error. Code")).
                              arg(CCommons::NetworkErrorToString(network_error)));
      break;
    default:
      ui->lbl_info->setVisible(true);
      ui->lbl_info->setText(QString("<font color='red'>%1</font>").
                              arg(tr("Cannot unregister peer.")).
                              arg(err_code));
      break;
  }
  return false;
}

void DlgRegisterPeer::init(const QString local_ip,
                           const QString name) {
  QStringList ip_split;
  ip_addr = local_ip.toStdString().c_str();
  peer_name = name.toStdString().c_str();
  ui->lne_password->setText(CSettingsManager::Instance().peer_pass(peer_name));
  ui->lne_peername->setText(this->peer_name);

  switch(VagrantProvider::Instance()->CurrentProvider()) {
  case VagrantProvider::HYPERV:
    m_url_management = ip_addr + ":8443";
    ip_split = ip_addr.split(".");
    ip_addr = ip_split.takeLast();
    ip_addr = ip_addr.toInt() + 9999;
    break;
  default:
    m_url_management = "localhost:" + ip_addr;
    break;
  }

  dialog_used[ip_addr.toInt() - 9999] = 1;
}
