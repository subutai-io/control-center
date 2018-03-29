#include "DlgRegisterPeer.h"
#include "ui_DlgRegisterPeer.h"
#include <QDebug>
#include <QMovie>
#include <QLineEdit>
#include <QObject>

#include "SystemCallWrapper.h"
#include "TrayControlWindow.h"
#include "RhController.h"


DlgRegisterPeer::DlgRegisterPeer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgRegisterPeer){

    ui->setupUi(this);
    this->setMinimumWidth( this->width());
    connect(ui->btn_cancel, &QPushButton::clicked,
            [this](){ this->close(); });
}

DlgRegisterPeer::~DlgRegisterPeer()
{
    delete ui;
}

void DlgRegisterPeer::registerPeer(){
    qInfo()
            << "Register button pressed: "
            << ip_addr;

    ui->btn_register->setEnabled(false);
    ui->btn_cancel->setEnabled(false);

    const QString login = ui->lne_username->text();
    const QString password = ui->lne_password->text();
    QString token;
    static int err_code, http_code, network_error;
    CRestWorker::Instance()->get_peer_token(ip_addr, login, password,
                                            token, err_code, http_code, network_error);
    const QString peer_name = ui->lne_peername->text();
    const QString peer_scope = ui->cmb_peer_scope->currentText();

    qDebug()
            << "Register peer: Get token errors: "
            << "Http code " << http_code
            << "Error code " << err_code
            << "Network Error " << network_error;

    bool kill_me=false;

    switch (err_code) {
      case RE_SUCCESS:
        kill_me=true;
        break;
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
        ui->lbl_info->setText(QString("<font color='red'>%1 : %2</font>").
                                arg(tr("Unknown error. Code")).
                                arg(err_code));
        break;
    }
    if(kill_me){
        CRestWorker::Instance()->register_peer(ip_addr, token,
                                               CSettingsManager::Instance().login(), CSettingsManager::Instance().password(),
                                               peer_name, peer_scope,
                                               err_code, http_code, network_error);
        kill_me = false;
        switch (err_code) {
          case RE_SUCCESS:
            kill_me = true;
            break;
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
                                    arg(tr("Can't register peer.")).
                                    arg(err_code));
            break;
        }
        if (kill_me){
            CHubController::Instance().force_refresh();
            this->close();
        }
    }
    ui->btn_cancel->setEnabled(true);
    ui->btn_register->setEnabled(true);
}

void DlgRegisterPeer::unregisterPeer(){
    qInfo()
            << "Unregister button pressed: "
            << ip_addr;

    const QString login=ui->lne_username->text();
    const QString password=ui->lne_password->text();
    QString token;
    static int err_code, http_code, network_error;
    ui->btn_cancel->setEnabled(false);
    ui->btn_unregister->setEnabled(false);

    CRestWorker::Instance()->get_peer_token(ip_addr, login, password,
                                            token, err_code, http_code, network_error);
    qDebug()
            << "Unregister peer: Get token errors: "
            << "Http code " << http_code
            << "Error code " << err_code
            << "Network Error " << network_error;

    bool kill_me=false;
    switch (err_code) {
      case RE_SUCCESS:
        kill_me=true;
        break;
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
        ui->lbl_info->setText(QString("<font color='red'>%1 : %2</font>").
                                arg(tr("Unknown error. Code")).
                                arg(err_code));
        break;
    }
    if(kill_me){
        CRestWorker::Instance()->unregister_peer(ip_addr, token, err_code, http_code, network_error);
        kill_me = false;
        switch (err_code) {
          case RE_SUCCESS:
            kill_me = true;
            break;
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
                                    arg(tr("Can't unregister peer.")).
                                    arg(err_code));
            break;
        }
        if (kill_me){
            CHubController::Instance().force_refresh();
            this->close();
        }
    }
    ui->btn_cancel->setEnabled(true);
    ui->btn_unregister->setEnabled(true);
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
