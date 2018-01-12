#include "DlgPeer.h"
#include "ui_DlgPeer.h"
#include "SystemCallWrapper.h"
#include "TrayControlWindow.h"

#include <QGroupBox>
#include <QLayout>
#include <QFormLayout>
#include <QLineEdit>
#include "OsBranchConsts.h"

DlgPeer::DlgPeer(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgPeer)
{
  ui->setupUi(this);
  this->setMinimumWidth(this->width());
  this->ui->le_pass->setEchoMode(QLineEdit::PasswordEchoOnEdit);
  this->adjustSize();
  qDebug() << "Peer dialog is initialized";
}



void DlgPeer::addLocalPeer(const QString local_ip) {
  qDebug() << "Adding a local peer with ip address: " << local_ip;

  connect(ui->btn_launch_console, &QPushButton::clicked, [local_ip](){
    CHubController::Instance().launch_browser(QString("https://%1:8443").arg(local_ip));
  });

  ui->le_ip->setText(local_ip);
  ui->le_ip->setReadOnly(true);
}

void DlgPeer::addHubPeer(CMyPeerInfo peer) {
  qDebug() << "Adding a hub peer with name: " << peer.name();

  connect(ui->btn_peer_on_hub, &QPushButton::clicked, [peer]() {
    CHubController::Instance().launch_peer_page(peer.id().toInt());
  });

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
  }
}

void DlgPeer::addPeer(CMyPeerInfo *hub_peer, std::pair<QString, QString> local_peer) { // local_peer - pair of fingerprint and local_ip adress of Peer

  QString peer_fingerprint = hub_peer == NULL ? local_peer.first : hub_peer->fingerprint();
  QString local_ip_addr = local_peer.second;

  QString default_ip = CSettingsManager::Instance().rh_host(peer_fingerprint);
  QString default_port = QString::number(CSettingsManager::Instance().rh_port(peer_fingerprint));
  QString default_user = CSettingsManager::Instance().rh_user(peer_fingerprint);
  QString default_pass = CSettingsManager::Instance().rh_pass(peer_fingerprint);

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
    ui->le_user->setText("ubuntu");

  if (!default_pass.isEmpty())
    ui->le_pass->setText(default_pass);
  else
    ui->le_pass->setText("ubuntu");


  connect(ui->show_ssh, &QCheckBox::toggled, [this](bool checked){
    this->ui->gr_ssh->setVisible(checked);
    this->adjustSize();
  });


  connect(ui->btn_ssh_peer, &QPushButton::clicked, [peer_fingerprint, this](){
    CSettingsManager::Instance().set_rh_host(peer_fingerprint, this->ui->le_ip->text());
    CSettingsManager::Instance().set_rh_port(peer_fingerprint, this->ui->le_port->text().toInt());
    CSettingsManager::Instance().set_rh_user(peer_fingerprint, this->ui->le_user->text());
    CSettingsManager::Instance().set_rh_pass(peer_fingerprint, this->ui->le_pass->text());
    emit this->ssh_to_rh_sig(peer_fingerprint, (void *)this->ui->btn_ssh_peer);
  });


  if (!local_ip_addr.isEmpty()) {
    addLocalPeer(local_ip_addr);
  }
  else {
    ui->btn_launch_console->setEnabled(false);
  }

  if (hub_peer != NULL) {
    addHubPeer(*hub_peer);
  }
  else {
    ui->btn_peer_on_hub->setEnabled(false);
    ui->gr_ssh->setTitle("This Peer is in your local network.");
    ui->show_ssh->setChecked(true);
    ui->show_ssh->toggled(true);
    ui->lbl_env->hide();
    ui->lbl_env_owner->hide();
    ui->lbl_env_status->hide();
    ui->line_1->hide();
    ui->line_2->hide();
    ui->line_3->hide();
  }

}


DlgPeer::~DlgPeer()
{
  delete ui;
}
