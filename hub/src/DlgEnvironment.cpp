#include "DlgEnvironment.h"
#include "ui_DlgEnvironment.h"
#include "P2PController.h"
#include <QToolTip>

DlgEnvironment::DlgEnvironment(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgEnvironment)
{
    ui->setupUi(this);
    this->layout()->setSizeConstraint(QLayout::SetDefaultConstraint);
}
/////////////////////////////////////////////////////////////////////////

void DlgEnvironment::addEnvironment(const CEnvironment *env){
  for (auto cont = env->containers().begin() ; cont != env->containers().end() ; cont ++) {
    addContainer(&(*cont));
    addRemoteAccess(env, &(*cont));
  }
  ui->btn_ssh_all->setEnabled(env->healthy());
  ui->btn_desktop_all->setEnabled(env->healthy());

  connect(ui->btn_open_hub, &QPushButton::clicked, [env](){
    CHubController::Instance().launch_environment_page(env->hub_id());
  });
}
/////////////////////////////////////////////////////////////////////////

void DlgEnvironment::addContainer(const CHubContainer *cont) {
  QLabel *cont_name = new QLabel(cont->name(), this);
  QLabel *cont_ip = new QLabel(cont->ip(), this);
  QLabel *cont_rhip_port = new QLabel(cont->rh_ip() + ":" + cont->port(), this);
  cont_name->setAlignment(Qt::AlignHCenter);
  cont_ip->setAlignment(Qt::AlignHCenter);
  cont_rhip_port->setAlignment(Qt::AlignHCenter);
  cont_name->setTextInteractionFlags(Qt::TextSelectableByMouse);
  cont_ip->setTextInteractionFlags(Qt::TextSelectableByMouse);
  cont_rhip_port->setTextInteractionFlags(Qt::TextSelectableByMouse);
  ui->cont_name->addWidget(cont_name);
  ui->cont_ip_port->addWidget(cont_ip);
  ui->cont_rhip->addWidget(cont_rhip_port);
}
/////////////////////////////////////////////////////////////////////////

void DlgEnvironment::check_status(QPushButton *btn_ssh, QPushButton *btn_desktop, const CEnvironment *env, const CHubContainer *cont) {
  P2PController::P2P_CONNETION_STATUS
      p2p_status = P2PController::Instance().is_ready(*env, *cont);

  QString str_status
      = P2PController::Instance().p2p_connection_status_to_str(p2p_status);


  btn_ssh->setToolTip(str_status);
  btn_desktop->setToolTip(str_status);
  btn_ssh->setEnabled(p2p_status == P2PController::CONNECTION_SUCCESS);
  btn_desktop->setEnabled(p2p_status == P2PController::CONNECTION_SUCCESS);

  if (p2p_status == P2PController::CANT_JOIN_SWARM) {
    ui->btn_ssh_all->setEnabled(false);
    ui->btn_desktop_all->setEnabled(false);

    ui->btn_ssh_all->setToolTip(str_status);
    ui->btn_desktop_all->setToolTip(str_status);
  }
  else {
    ui->btn_ssh_all->setEnabled(true);
    ui->btn_desktop_all->setEnabled(true);

    ui->btn_ssh_all->setToolTip("Press to SSH into ready containers.");
    ui->btn_desktop_all->setToolTip("Press to DESKTOP into ready containers.");
  }

  if (!cont->is_desktop()) {
    btn_desktop->setEnabled(false);
    btn_desktop->setToolTip("Container doesn't have desktop");
  }
}
/////////////////////////////////////////////////////////////////////////

void DlgEnvironment::button_enhancement(QPushButton *btn) {
  QFont *font = new QFont("Arial", 10, QFont::AllUppercase);
  btn->setFont(*font);
}
/////////////////////////////////////////////////////////////////////////

void DlgEnvironment::addRemoteAccess(const CEnvironment *env, const CHubContainer *cont) {
  QPushButton* btn_ssh = new QPushButton(tr("SSH"), this);
  QPushButton* btn_desktop = new QPushButton(tr("DESKTOP"), this);

  button_enhancement(btn_ssh);
  button_enhancement(btn_desktop);

  ui->cont_remote->addRow(btn_ssh, btn_desktop);

  QTimer *timer = new QTimer(this);

  CEnvironment copy_env = *env;
  CHubContainer copy_cont = *cont;

  connect(timer, &QTimer::timeout, [btn_ssh, btn_desktop, copy_env, copy_cont, this]() {
    this->check_status(btn_ssh, btn_desktop, &copy_env, &copy_cont);
  });

  check_status(btn_ssh, btn_desktop, env, cont);
  timer->start(10000);

  connect(ui->btn_ssh_all, &QPushButton::clicked, btn_ssh, &QPushButton::click);

  connect(btn_ssh, &QPushButton::clicked, [this, copy_env, copy_cont, btn_ssh]() {
    emit this->ssh_to_container_sig(&copy_env, &copy_cont, (void *)btn_ssh);
  });

  connect(ui->btn_desktop_all, &QPushButton::clicked, btn_desktop, &QPushButton::click);

  connect(btn_desktop, &QPushButton::clicked, [this, copy_env, copy_cont, btn_desktop]() {
    emit this->desktop_to_container_sig(&copy_env, &copy_cont, (void *)btn_desktop);
  });
}
////////////////////////////////////////////////////////////////////////////

DlgEnvironment::~DlgEnvironment() {
  delete ui;
}
