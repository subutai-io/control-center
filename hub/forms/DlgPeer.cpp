#include "DlgPeer.h"
#include "ui_DlgPeer.h"
#include "SystemCallWrapper.h"
#include "TrayControlWindow.h"


DlgPeer::DlgPeer(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgPeer)
{
  ui->setupUi(this);
  QStringList headers;
  headers << "Name" << "Status" << "Owner";
  ui->tbl_envs->setColumnCount(3);
  ui->tbl_envs->setHorizontalHeaderLabels(headers);
  ui->tbl_envs->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui->tbl_envs->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tbl_envs->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  this->layout()->setSizeConstraint(QLayout::SetFixedSize);

}



void DlgPeer::addLocalPeer(const QString id, const QString ip_addr){
  QString ip = CSettingsManager::Instance().rh_host(id);
  quint16 port = CSettingsManager::Instance().rh_port(id);
  QString user = CSettingsManager::Instance().rh_user(id);
  QString pass = CSettingsManager::Instance().rh_pass(id);

  if (ip.isNull() || ip.isEmpty()) ui->le_local_ip->setText(ip_addr);
  else ui->le_local_ip->setText(ip_addr);

  if (port == 0) ui->le_port->setText("22");
  else ui->le_port->setText(QString("%1").arg(port));

  if (pass.isNull() || pass.isNull()) ui->le_pass->setText("subutai");
  else ui->le_pass->setText(pass);

  if (user.isNull() || user.isNull()) ui->le_user->setText("ubuntu");
  else ui->le_user->setText(user);

  connect(ui->btn_launch_console, &QPushButton::clicked, [id, this](){
    CSettingsManager::Instance().set_rh_host(id, this->ui->le_local_ip->text());
    CSettingsManager::Instance().set_rh_port(id, this->ui->le_port->text().toInt());
    CSettingsManager::Instance().set_rh_user(id, this->ui->le_user->text());
    CSettingsManager::Instance().set_rh_pass(id, this->ui->le_pass->text());
    emit this->launch_ss_console(id);
  });
}

void DlgPeer::noPeer() {
  ui->gr_peer_info->hide();
  ui->lbl_envs->hide();
  ui->tbl_envs->hide();
}

void DlgPeer::noLocalPeer() {
  ui->gr_launch_ss->hide();
}


void DlgPeer::addPeer(CMyPeerInfo &peer) {
  ui->le_ip_address->setText(peer.ip_address());
  ui->le_isp->setText(peer.isp());
  ui->le_name->setText(peer.name());
  ui->le_status->setText(peer.status());

  const std::vector<CMyPeerInfo::env_info> envs = peer.peer_environments();
  int row = 0;
  for (CMyPeerInfo::env_info env : envs) {
    ui->tbl_envs->setItem(row, 0, new QTableWidgetItem(env.envName));
    ui->tbl_envs->setItem(row, 1, new QTableWidgetItem(env.status));
    ui->tbl_envs->setItem(row, 2, new QTableWidgetItem(env.ownerName));
    row ++;
  }
}


DlgPeer::~DlgPeer()
{
  delete ui;
}
