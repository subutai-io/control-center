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
  ui->tbl_envs->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  this->layout()->setSizeConstraint(QLayout::SetMinimumSize);
  ui->btn_launch_console->setEnabled(false);
}



void DlgPeer::addLocalPeer(const QString ip_addr){
  ui->btn_launch_console->setEnabled(true);
  connect(ui->btn_launch_console, &QPushButton::clicked, [ip_addr](){
    CHubController::Instance().launch_browser(QString("https://%1:8443").arg(ip_addr));
  });
}

void DlgPeer::addPeer(CMyPeerInfo &peer) {
  ui->lbl_peer_name->setText(peer.name());
  ui->lbl_peer_status->setText(peer.status());

  const std::vector<CMyPeerInfo::env_info> envs = peer.peer_environments();
  CNotificationObserver::Info(QString("Env size is: %1").arg(envs.size()), DlgNotification::N_NOTF_HISTORY);
  ui->tbl_envs->setRowCount(envs.size());
  int row = 0;
  for (CMyPeerInfo::env_info env : envs) {
    ui->tbl_envs->setItem(row, 0, new QTableWidgetItem(env.envName));
    ui->tbl_envs->setItem(row, 1, new QTableWidgetItem(env.status));
    ui->tbl_envs->setItem(row, 2, new QTableWidgetItem(env.ownerName));
    row ++;
  }
  connect(ui->btn_peer_on_hub, &QPushButton::clicked, [peer](){
    CHubController::Instance().launch_peer_page(peer.id().toInt());
  });
}


DlgPeer::~DlgPeer()
{
  delete ui;
}
