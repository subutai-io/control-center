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
  p2p_message_res_t res = P2PController::Instance().status(env, cont);

  btn_ssh->setToolTip(res.btn_ssh_message);
  btn_desktop->setToolTip(res.btn_desktop_message);
  btn_ssh->setEnabled(res.btn_ssh_enabled);
  btn_desktop->setEnabled(res.btn_desktop_enabled);

  if (!cont->is_desktop()) {
    btn_desktop->setToolTip(p2p_messages[NO_DESKTOP]);
    btn_desktop->setEnabled(false);
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
