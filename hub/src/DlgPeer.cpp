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
  ui(new Ui::DlgPeer)
{
  ui->setupUi(this);
  this->setMinimumWidth(this->width());
  this->ui->le_pass->setEchoMode(QLineEdit::PasswordEchoOnEdit);
  qDebug() << "Peer dialog is initialized";
  connect(CRhController::Instance(), &CRhController::ssh_to_rh_finished,
          this, &DlgPeer::ssh_to_rh_finished_sl);
  registration_dialog = nullptr;
  ui->lbl_connection_status->hide();
}

void DlgPeer::addLocalPeer(const QString local_ip) {
  qDebug() << "Adding a local peer with ip address: " << local_ip;

  connect(ui->btn_launch_console, &QPushButton::clicked, [local_ip](){
    CHubController::Instance().launch_browser(QString("https://%1:8443").arg(local_ip));
  });

  ui->le_ip->setText(local_ip);
  ui->le_ip->setReadOnly(true);
  ui->btn_register->show();
  ui->btn_unregister->hide();
  connect(ui->btn_register, &QPushButton::clicked,this, &DlgPeer::registerPeer);
}

void DlgPeer::addHubPeer(CMyPeerInfo peer) {
  qDebug() << "Adding a hub peer with the name: " << peer.name() << " " << peer.id();

  connect(ui->btn_peer_on_hub, &QPushButton::clicked, [peer]() {
    CHubController::Instance().launch_peer_page(peer.id().toInt());
  });

  current_peer_name = peer.name();

  const std::vector<CMyPeerInfo::env_info> envs = peer.peer_environments();

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
    ui->show_ssh->setChecked(false);
    ui->show_ssh->toggled(false);
    ui->btn_unregister->setEnabled(false);
    this->adjustSize();
  }
  else {
    ui->gr_ssh->setTitle("This peer doesn't have any \"Environments\" yet.");
    ui->lbl_env->hide();
    ui->lbl_env_owner->hide();
    ui->lbl_env_status->hide();
    ui->line_1->hide();
    ui->line_2->hide();
    ui->line_3->hide();

    ui->show_ssh->setChecked(true);
    ui->show_ssh->toggled(true);
    ui->btn_unregister->setEnabled(true);
    this->adjustSize();
  }
  ui->btn_unregister->show();
  ui->btn_register->hide();
  connect(ui->btn_unregister, &QPushButton::released, this, &DlgPeer::unregisterPeer);
}

void DlgPeer::addPeer(CMyPeerInfo *hub_peer, std::pair<QString, QString> local_peer, std::vector<CLocalPeer> lp) { // local_peer - pair of fingerprint and local_ip adress of Peer
    if(lp.empty()){

        QString peer_fingerprint = hub_peer == NULL ? local_peer.first : hub_peer->fingerprint();
        QString local_ip_addr = local_peer.second;

        QString default_ip = CSettingsManager::Instance().rh_host(peer_fingerprint);
        QString default_port = QString::number(CSettingsManager::Instance().rh_port(peer_fingerprint));
        QString default_user = CSettingsManager::Instance().rh_user(peer_fingerprint);
        QString default_pass = CSettingsManager::Instance().rh_pass(peer_fingerprint);

        current_peer_id = peer_fingerprint;


        if (!default_ip.isEmpty())
        ui->le_ip->setText(default_ip);
        else
        ui->le_ip->setText("127.0.0.1");


        if (!default_port.isEmpty() && default_port != "0")
        ui->le_port->setText(default_port);
        else
        ui->le_port->setText("22");

        if (!default_user.isEmpty())
        ui->le_user->setText(default_user);
        else
        ui->le_user->setText("subutai");

        if (!default_pass.isEmpty())
        ui->le_pass->setText(default_pass);
        else
        ui->le_pass->setText("ubuntai");

        ui->show_peer_control->setChecked(false);
        this->ui->gr_peer_control->setVisible(false);
        this->adjustSize();

        if (!local_ip_addr.isEmpty()) {
            addLocalPeer(local_ip_addr);
            ui->btn_unregister->setEnabled(true);
            ui->btn_register->setEnabled(true);
        }
        else {
            ui->btn_launch_console->setEnabled(false);
            ui->btn_unregister->setEnabled(false);
            ui->btn_register->setEnabled(false);
        }

        if (hub_peer != NULL) {
            addHubPeer(*hub_peer);
        }
        else {
            ui->btn_peer_on_hub->setEnabled(false);
            ui->gr_ssh->setTitle("This Peer is not registered to Bazaar.");
            ui->show_ssh->setChecked(true);
            ui->show_ssh->toggled(true);
            ui->lbl_env->hide();
            ui->lbl_env_owner->hide();
            ui->lbl_env_status->hide();
            ui->line_1->hide();
            ui->line_2->hide();
            ui->line_3->hide();
            this->adjustSize();
        }
        ui->lbl_name->hide();
        ui->lbl_state->hide();
        ui->le_name->hide();
        ui->le_state->hide();
        ui->btn_register->hide();
        ui->btn_unregister->hide();
        ui->btn_start->hide();
        ui->btn_stop->hide();
        ui->gr_peer_control->setTitle("This peer is not your machine");
    }
    else{
        ui->gr_peer_control->setTitle("This peer is in your machine");
        ui->le_state->setText(lp[0].status());
        ui->le_name->setText(lp[0].name());
        if(lp[0].status() == "Running")
            ui->le_state->setStyleSheet("QLineEdit { color : green; }");
        else
            ui->le_state->setStyleSheet("QLineEdit { color : red; }");
        QString peer_fingerprint = hub_peer != NULL ? hub_peer->fingerprint() : lp[0].fingerprint();
        QString default_ip = lp[0].ip();

        QString default_port = QString::number(CSettingsManager::Instance().rh_port(peer_fingerprint));
        QString default_user = CSettingsManager::Instance().rh_user(peer_fingerprint);
        QString default_pass = CSettingsManager::Instance().rh_pass(peer_fingerprint);

        current_peer_id = peer_fingerprint;


        if (!default_ip.isEmpty())
            ui->le_ip->setText(default_ip);
        else
            ui->le_ip->setText("127.0.0.1");


        if (!default_port.isEmpty() && default_port != "0")
            ui->le_port->setText(default_port);
        else
            ui->le_port->setText("22");

        if (!default_user.isEmpty())
            ui->le_user->setText(default_user);
        else
            ui->le_user->setText("subutai");

        if (!default_pass.isEmpty())
            ui->le_pass->setText(default_pass);
        else
            ui->le_pass->setText("ubuntai");
        if(ui->le_state->text() == "Running")
            addLocalPeer(default_ip);
        ui->btn_unregister->setEnabled(true);
        ui->btn_register->setEnabled(true);
        if (hub_peer != NULL) {
            addHubPeer(*hub_peer);
        }
        else{
            ui->btn_peer_on_hub->setEnabled(false);
            ui->gr_ssh->setTitle("This Peer is not registered to Bazaar.");
            ui->show_ssh->setChecked(true);
            ui->show_ssh->toggled(true);
            ui->lbl_env->hide();
            ui->lbl_env_owner->hide();
            ui->lbl_env_status->hide();
            ui->line_1->hide();
            ui->line_2->hide();
            ui->line_3->hide();
            this->adjustSize();
        }
        ui->btn_start->hide();

        ui->show_peer_control->setChecked(false);
        this->ui->gr_peer_control->setVisible(false);
        this->adjustSize();
    }
    ui->le_id->hide();
    ui->le_dir->hide();
    ui->lbl_id->hide();
    ui->lbl_dir->hide();
    connect(ui->show_ssh, &QCheckBox::toggled, [this](bool checked){
        this->ui->gr_ssh->setVisible(checked);
        this->adjustSize();
    });
    connect(ui->show_peer_control, &QCheckBox::toggled, [this](bool checked){
        this->ui->gr_peer_control->setVisible(checked);
        this->adjustSize();
    });
    connect(ui->btn_ssh_peer, &QPushButton::clicked, [this]() {
    CSettingsManager::Instance().set_rh_host(this->current_peer_id, this->ui->le_ip->text());
    CSettingsManager::Instance().set_rh_port(this->current_peer_id, this->ui->le_port->text().toInt());
    CSettingsManager::Instance().set_rh_user(this->current_peer_id, this->ui->le_user->text());
    CSettingsManager::Instance().set_rh_pass(this->current_peer_id, this->ui->le_pass->text());
    this->ui->btn_ssh_peer->setEnabled(false);
    this->ui->btn_ssh_peer->setText("PROCESSING..");
    emit this->ssh_to_rh_sig(this->current_peer_id);
    });
}

void DlgPeer::ssh_to_rh_finished_sl(const QString &peer_fingerprint, system_call_wrapper_error_t res, int libbssh_exit_code) {
  qDebug()<<"ssh to rh finished";
  UNUSED_ARG(res);
  UNUSED_ARG(libbssh_exit_code); // need to use this variables to give feedback to user
  if (QString::compare(current_peer_id, peer_fingerprint, Qt::CaseInsensitive) != 0)
      return;

  ui->btn_ssh_peer->setEnabled(true);
  ui->btn_ssh_peer->setText("SSH into Peer");
  ui->lbl_connection_status->show();

  if (res != SCWE_SUCCESS){
      if(libbssh_exit_code!=0){
        qDebug() << "Peer is not accessible";
        ui->lbl_connection_status->setText(QString("Peer is not accessible. Error: %1").arg(CLibsshController::run_libssh2_error_to_str((run_libssh2_error_t)libbssh_exit_code)));
      }
      else{
          qDebug() << "Can't run terminal via SSH";
          ui->lbl_connection_status->setText(QString("Can't run terminal to ssh into peer. Error: %1").arg(CSystemCallWrapper::scwe_error_to_str(res)));}
      ui->lbl_connection_status->setStyleSheet("QLabel { font-weight: bold; color : red; }");
  }
  else{
      qDebug() <<"connected to peer";
      ui->lbl_connection_status->setText(QString("Successfully connected"));
      ui->lbl_connection_status->setStyleSheet("QLabel {font-weight: bold; color : green; }");
  }
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
    dlg_unregister->setWindowTitle(tr("Unregister peer: %1").arg(current_peer_name));
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

DlgPeer::~DlgPeer()
{
  qDebug() << "Deleting DlgPeer";
  delete ui;
}
