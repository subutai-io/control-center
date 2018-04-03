#include "DlgPeer.h"
#include "ui_DlgPeer.h"
#include "SystemCallWrapper.h"
#include "DlgRegisterPeer.h"
#include "TrayControlWindow.h"

#include <QGroupBox>
#include <QLayout>
#include <QFormLayout>
#include <QLineEdit>
#include "OsBranchConsts.h"
#include "RhController.h"

#include "LibsshController.h"

DlgPeer::DlgPeer(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgPeer){
    qDebug() << "Peer dialog is initialized";
    //ui
    ui->setupUi(this);
    this->setMinimumWidth(this->width());
    ui->gr_peer_control->setStyleSheet("QGroupBox:title{max-width: 0; max-height: 0;}");
    this->ui->le_pass->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    ui->show_ssh->setChecked(true);
    ui->gr_ssh->setVisible(true);
    ui->gr_peer_control->setVisible(false);
    ui->show_peer_control->setChecked(false);
    ui->btn_launch_console->setEnabled(false);

    //slots
    connect(CRhController::Instance(), &CRhController::ssh_to_rh_finished,
          this, &DlgPeer::ssh_to_rh_finished_sl);
    connect(ui->show_ssh, &QCheckBox::toggled, [this](bool checked){
      this->ui->gr_ssh->setVisible(checked);
      this->adjustSize();
    });
    connect(ui->show_peer_control, &QCheckBox::toggled, [this](bool checked){
      this->ui->gr_peer_control->setVisible(checked);
      this->adjustSize();
    });
    //vars
    registration_dialog = nullptr;
    ssh_available = false;
    advanced = false;
}

void DlgPeer::addLocalPeer(std::pair<QString, QString> peer) {
    peer_fingerprint = peer.first;
    ssh_ip = peer.second;
    ssh_available = true;
    connect(ui->btn_launch_console, &QPushButton::clicked,
            [peer](){
        CHubController::Instance().launch_browser(QString("https://%1:8443").arg(peer.second));});
    ui->btn_launch_console->setEnabled(true);
}

void DlgPeer::addHubPeer(CMyPeerInfo peer) {
    connect(ui->btn_peer_on_hub, &QPushButton::clicked, [peer]() {
    CHubController::Instance().launch_peer_page(peer.id().toInt());
    });
    peer_name = peer.name();
    peer_fingerprint = peer.fingerprint();
    ssh_available = true;
    const std::vector<CMyPeerInfo::env_info> envs = peer.peer_environments();
    update_environments(envs);
}

void DlgPeer::addMachinePeer(CLocalPeer peer){
    if(peer.fingerprint() != "loading" || peer.fingerprint() != "undefined")
        peer_fingerprint = peer.fingerprint();
}

void DlgPeer::addPeer(CMyPeerInfo *hub_peer, std::pair<QString, QString> local_peer, std::vector<CLocalPeer> lp) {
    if(hub_peer != nullptr)
        addHubPeer(*hub_peer);
    else hideEnvs();
    if(!local_peer.first.isEmpty())
        addLocalPeer(local_peer);
    if(!lp.empty())
        addMachinePeer(lp[0]);
    if(ssh_available){ // parse SSH information
        if(ssh_ip.isEmpty())
            ssh_ip = CSettingsManager::Instance().rh_host(peer_fingerprint);
        if(ssh_ip.isEmpty())
            ssh_ip = QString("127.0.0.1");
        ssh_port = QString::number(CSettingsManager::Instance().rh_port(peer_fingerprint));
        if(ssh_port.isEmpty() || ssh_port == "0")
            ssh_port = QString("22");
        ssh_user = CSettingsManager::Instance().rh_user(peer_fingerprint);
        if(ssh_user.isEmpty())
            ssh_user = QString("subutai");
        ssh_pass = CSettingsManager::Instance().rh_pass(peer_fingerprint);
        if(ssh_pass.isEmpty())
            ssh_pass = QString("ubuntai");

        ui->le_ip->setText(ssh_ip); ui->le_ip->setReadOnly(true);
        ui->le_port->setText(ssh_port); ui->le_port->setReadOnly(true);
        ui->le_user->setText(ssh_user);
        ui->le_pass->setText(ssh_pass);

        CSettingsManager::Instance().set_rh_host(peer_fingerprint, this->ui->le_ip->text());
        CSettingsManager::Instance().set_rh_port(peer_fingerprint, this->ui->le_port->text().toInt());
        CSettingsManager::Instance().set_rh_user(peer_fingerprint, this->ui->le_user->text());
        CSettingsManager::Instance().set_rh_pass(peer_fingerprint, this->ui->le_pass->text());
        connect(ui->btn_ssh_peer, &QPushButton::clicked, [this]() {
            this->ui->btn_ssh_peer->setEnabled(false);
            this->ui->btn_ssh_peer->setText(tr("PROCESSING.."));
            emit this->ssh_to_rh_sig(this->peer_fingerprint);
        });
    }
    else
        hideSSH();
}

void DlgPeer::ssh_to_rh_finished_sl(const QString &peer_fingerprint, system_call_wrapper_error_t res, int libbssh_exit_code) {
  qDebug()<<"ssh to rh finished";
  UNUSED_ARG(res);
  UNUSED_ARG(libbssh_exit_code); // need to use this variables to give feedback to user
  if (QString::compare(peer_fingerprint, peer_fingerprint, Qt::CaseInsensitive) != 0)
      return;

  ui->btn_ssh_peer->setEnabled(true);
  ui->btn_ssh_peer->setText(tr("SSH into Peer"));
}

void DlgPeer::registerPeer(){
    ui->btn_register->setEnabled(false);
    DlgRegisterPeer* dlg_register = new DlgRegisterPeer(this);
    const QString ip_addr=ui->le_ip->text();
    dlg_register->setLocalAddr(ip_addr);
    dlg_register->setRegistrationMode();
    dlg_register->setWindowTitle(tr("Register peer: %1").arg(ip_addr));
    dlg_register->show();
    registration_dialog = dlg_register;
    connect (dlg_register, &QDialog::finished, this, &DlgPeer::regDlgClosed);
}

void DlgPeer::unregisterPeer(){
    ui->btn_unregister->setEnabled(false);
    DlgRegisterPeer* dlg_unregister = new DlgRegisterPeer(this);
    const QString ip_addr=ui->le_ip->text();
    dlg_unregister->setLocalAddr(ip_addr);
    dlg_unregister->setUnregistrationMode();
    dlg_unregister->setWindowTitle(tr("Unregister peer: %1").arg(peer_name));
    dlg_unregister->show();
    registration_dialog = dlg_unregister;
    connect (dlg_unregister, &QDialog::finished, this, &DlgPeer::regDlgClosed);
}

void DlgPeer::regDlgClosed(){
    ui->btn_register->setEnabled(true);
    ui->btn_unregister->setEnabled(true );
    if(registration_dialog == nullptr)
        return;
    registration_dialog->deleteLater();
}

void DlgPeer::hideSSH(){
    ui->label->hide();
    ui->label_3->hide();
    ui->label_2->hide();
    ui->label_4->hide();
    ui->le_ip->hide();
    ui->le_port->hide();
    ui->le_user->hide();
    ui->le_pass->hide();
    ui->btn_ssh_peer->hide();
    ui->show_ssh->toggle(); //lifehack
    ui->show_ssh->toggle();
    this->adjustSize();
}

void DlgPeer::hidePeer(){
    ui->lbl_name->hide();
    ui->lbl_state->hide();
    ui->le_name->hide();
    ui->le_state->hide();
    ui->btn_start->hide();
    ui->btn_stop->hide();
    ui->btn_register->hide();
    ui->btn_unregister->hide();
    this->adjustSize();
}

void DlgPeer::hideEnvs(){
    ui->gr_ssh->setTitle(tr("This peer doesn't have any \"Environments\""));
    ui->lbl_env->hide();
    ui->lbl_env_owner->hide();
    ui->lbl_env_status->hide();
    ui->line_1->hide();
    ui->line_2->hide();
    ui->line_3->hide();
    this->adjustSize();
}

void DlgPeer::stopPeer(){
    /*
    StopPeer *thread_init = new StopPeer(this);
    ui->btn_stop->setEnabled(false);
    ui->btn_destroy->setEnabled(false);
    ui->btn_register->setEnabled(false);
    ui->btn_unregister->setEnabled(false);
    ui->btn_stop->setText(tr("Trying to stop peer..."));
    thread_init->init(ui->le_dir->text());
    thread_init->startWork();
    connect(thread_init, &StopPeer::outputReceived, [this](system_call_wrapper_error_t res){
        if(this == nullptr)
            return;
        if(res == SCWE_SUCCESS){
            CNotificationObserver::Instance()->Info(tr("Peer \"%1\" succesfully stopped").arg(this->ui->le_name->text()), DlgNotification::N_NO_ACTION);
            this->close();
        }
        else{
            CNotificationObserver::Instance()->Error(tr("Sorry, could not stop peer \"%1\"").arg(this->ui->le_name->text()), DlgNotification::N_NO_ACTION);
            ui->btn_stop->setEnabled(true);
            ui->btn_register->setEnabled(true);
            ui->btn_unregister->setEnabled(true);
            ui->btn_destroy->setEnabled(true);
            ui->btn_stop->setText(tr("Stop"));
        }
    });
    */
}

void DlgPeer::startPeer(){
    /*
    StartPeer *thread_init = new StartPeer(this);
    ui->btn_start->setEnabled(false);
    ui->btn_register->setEnabled(false);
    ui->btn_destroy->setEnabled(false);
    ui->btn_unregister->setEnabled(false);
    ui->btn_stop->setText(tr("Trying to launch peer..."));
    thread_init->init(ui->le_dir->text());
    thread_init->startWork();
    connect(thread_init, &StartPeer::outputReceived, [this](system_call_wrapper_error_t res){
        if(this == nullptr)
            return;
        if(res == SCWE_SUCCESS){
            CNotificationObserver::Instance()->Info(tr("Starting process started. Don't close before it finished"), DlgNotification::N_NO_ACTION);
            this->close();
        }
        else{
            CNotificationObserver::Instance()->Error(tr("Sorry, could not stop peer \"%1\"").arg(this->ui->le_name->text()), DlgNotification::N_NO_ACTION);
            ui->btn_stop->setEnabled(true);
            ui->btn_register->setEnabled(true);
            ui->btn_unregister->setEnabled(true);
            ui->btn_destroy->setEnabled(true);
            ui->btn_stop->setText(tr("Start"));
        }
    });
    */
}

void DlgPeer::destroyPeer(){
    /*
    DestroyPeer *thread_init = new DestroyPeer(this);
    ui->btn_start->setEnabled(false);
    ui->btn_destroy->setEnabled(false);
    ui->btn_register->setEnabled(false);
    ui->btn_unregister->setEnabled(false);
    ui->btn_destroy->setText(tr("Trying to destroy peer..."));
    thread_init->init(ui->le_dir->text());
    thread_init->startWork();
    connect(thread_init, &DestroyPeer::outputReceived, [this](system_call_wrapper_error_t res){
        if(this == nullptr)
            return;
        if(res == SCWE_SUCCESS){
            CNotificationObserver::Instance()->Info(tr("Peer have been destroyed."), DlgNotification::N_NO_ACTION);
            this->close();
        }
        else{
            CNotificationObserver::Instance()->Error(tr("Sorry, could not destroy peer \"%1\"").arg(this->ui->le_name->text()), DlgNotification::N_NO_ACTION);
            ui->btn_stop->setEnabled(true);
            ui->btn_register->setEnabled(true);
            ui->btn_unregister->setEnabled(true);
            ui->btn_destroy->setEnabled(true);
            ui->btn_stop->setText(tr("Start"));
        }
    });
    */
}

void DlgPeer::update_environments(const std::vector<CMyPeerInfo::env_info> envs){
    if (!envs.empty()) {
      for (CMyPeerInfo::env_info env : envs) {
        QLabel *env_name = new QLabel(env.envName);
        QLabel *env_owner = new QLabel(QString("<a href=\"%1\">%2</a>")
                                       .arg(hub_user_profile_url() + QString::number(env.ownerId)).arg(env.ownerName));
        QLabel *env_status = new QLabel(env.status);

        env_name->setAlignment(Qt::AlignHCenter);
        env_name->setTextInteractionFlags(Qt::TextSelectableByMouse);

        env_owner->setAlignment(Qt::AlignHCenter);
        env_owner->setTextFormat(Qt::RichText);
        env_owner->setTextInteractionFlags(Qt::TextBrowserInteraction);
        env_owner->setOpenExternalLinks(true);

        env_status->setAlignment(Qt::AlignHCenter);
        env_status->setTextInteractionFlags(Qt::TextSelectableByMouse);

        ui->env_name->addWidget(env_name);
        ui->env_owner->addWidget(env_owner);
        ui->env_status->addWidget(env_status);
      }
      this->adjustSize();
    }
    else hideEnvs();
}

DlgPeer::~DlgPeer()
{
  qDebug() << "Deleting DlgPeer";
  delete ui;
}
