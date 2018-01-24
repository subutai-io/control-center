#include "DlgEnvironment.h"
#include "ui_DlgEnvironment.h"
#include "P2PController.h"
#include <QToolTip>

DlgEnvironment::DlgEnvironment(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgEnvironment)
{
    ui->setupUi(this);
    this->setMinimumWidth(this->width());
    qDebug() << "Environment dialog is initialized";
}

/////////////////////////////////////////////////////////////////////////

void DlgEnvironment::addEnvironment(const CEnvironment *_env) {
  env= *_env;

  qDebug() << "Environment added env: " << env.name();

  for (auto cont : env.containers()) {
    addContainer(&cont);
  }

  for (auto cont : env.containers()) {
    remote_acces(cont, dct_cont_btn[cont.id()]);
  }

  ui->le_env_hash->setText(env.hash());
  ui->le_env_key->setText(env.key());
  ui->le_env_status->setText(env.status_description());
  ui->le_env_id->setText(env.id());

  ui->le_env_hash->setReadOnly(true);
  ui->le_env_key->setReadOnly(true);
  ui->le_env_status->setReadOnly(true);
  ui->le_env_id->setReadOnly(true);

  connect(ui->cb_details, &QCheckBox::toggled, [this](bool checked){
    this->ui->gr_details->setVisible(checked);
    this->adjustSize();
  });

  ui->cb_details->toggled(false);

  connect(ui->btn_open_hub, &QPushButton::clicked, [this](){
    CHubController::Instance().launch_environment_page(this->env.hub_id());
  });


  if (env.healthy()) { // check status with timer
    QTimer *timer = new QTimer(this);
    timer->setInterval(7000);
    connect(timer, &QTimer::timeout, this, &DlgEnvironment::check_environment_status);
    timer->start();
  }

  check_environment_status();
}

/////////////////////////////////////////////////////////////////////////

void DlgEnvironment::remote_acces(const CHubContainer &cont, std::pair<QPushButton*, QPushButton*> btns) {
  connect(ui->btn_ssh_all, &QPushButton::clicked, btns.first, &QPushButton::click);
  connect(btns.first, &QPushButton::clicked, [btns, this, cont](){
    emit this->ssh_to_container_sig(&this->env, &cont, (void *)btns.first);
  });

  connect(ui->btn_desktop_all, &QPushButton::clicked, btns.second, &QPushButton::click);
  connect(btns.second, &QPushButton::clicked, [btns, this, cont](){
    emit this->desktop_to_container_sig(&this->env, &cont, (void *)btns.first);
  });
}

//////////////////////////////////////////////////////////////////////////

void DlgEnvironment::addContainer(const CHubContainer *cont) {
  qDebug()
      << QString("Adding container cont: %1, env: %2 ").arg(cont->name(), env.name());
  QLabel *cont_name = new QLabel(cont->name(), this);
  QLabel *cont_ip = new QLabel(cont->ip(), this);
  QLabel *cont_rhip_port = new QLabel(cont->rh_ip() + ":" + cont->port(), this);
  QPushButton *btn_ssh = new QPushButton(tr("SSH"), this),
              *btn_desktop = new QPushButton(tr("DESKTOP"), this);

  cont_name->setAlignment(Qt::AlignHCenter);
  cont_ip->setAlignment(Qt::AlignHCenter);
  cont_rhip_port->setAlignment(Qt::AlignHCenter);

  cont_name->setTextInteractionFlags(Qt::TextSelectableByMouse);
  cont_ip->setTextInteractionFlags(Qt::TextSelectableByMouse);
  cont_rhip_port->setTextInteractionFlags(Qt::TextSelectableByMouse);

  ui->cont_name->addWidget(cont_name);
  ui->cont_ip_port->addWidget(cont_ip);
  ui->cont_rhip->addWidget(cont_rhip_port);
  ui->cont_remote->addRow(btn_ssh, btn_desktop);

  dct_cont_btn[cont->id()] = std::make_pair(btn_ssh, btn_desktop);
}

/////////////////////////////////////////////////////////////////////////

void DlgEnvironment::change_btn(QPushButton *btn, const QString tt_msg, bool enabled) {
  btn->setEnabled(enabled);
  btn->setToolTip(tt_msg);
}

/////////////////////////////////////////////////////////////////////////

void DlgEnvironment::check_container_status(const CHubContainer *cont, bool &ssh_all, bool &desktop_all) {
  qDebug()
      << "Checking the status of container: " << cont->name() << " in " << env.name();

  static QString
      no_desktop = "This container doesn't have desktop";

  P2PController::P2P_CONNETION_STATUS
      cont_status = P2PController::Instance().is_ready(env, *cont);
  bool ready = (P2PController::Instance().is_ready(env, *cont) == P2PController::CONNECTION_SUCCESS);

  change_btn(dct_cont_btn[cont->id()].first, P2PController::Instance().p2p_connection_status_to_str(cont_status), ready);

  if (cont->is_desktop())
    change_btn(dct_cont_btn[cont->id()].second, P2PController::Instance().p2p_connection_status_to_str(cont_status), ready);
  else
    change_btn(dct_cont_btn[cont->id()].second, no_desktop, false);

  ssh_all |= ready;
  desktop_all |= (ready & cont->is_desktop());
}


/////////////////////////////////////////////////////////////////////////

void DlgEnvironment::check_environment_status() {
  qDebug()
      << "Checking the status of environment " << env.name();
  P2PController::P2P_CONNETION_STATUS
      swarm_status = P2PController::Instance().is_swarm_connected(env);

  bool connected_to_swarm = (env.healthy() & (swarm_status == P2PController::CONNECTION_SUCCESS));

  bool ssh_all = false; // it becomes true, when there is at least one button enabled
  bool desktop_all = false; // same as ssh_all

  if (connected_to_swarm){
    for (auto cont : env.containers()){
      check_container_status(&cont, ssh_all, desktop_all);
    }
  }
  else {
    qDebug() << "Not connected to  swarm";
    for (auto cont : env.containers()) {
      change_btn(dct_cont_btn[cont.id()].first, P2PController::Instance().p2p_connection_status_to_str(swarm_status), false);
      change_btn(dct_cont_btn[cont.id()].second, P2PController::Instance().p2p_connection_status_to_str(swarm_status), false);
    }
  }

  ui->btn_ssh_all->setEnabled(connected_to_swarm & ssh_all);
  ui->btn_desktop_all->setEnabled(connected_to_swarm & desktop_all);
}



////////////////////////////////////////////////////////////////////////////

DlgEnvironment::~DlgEnvironment() {
  delete ui;
}
