#include "DlgEnvironment.h"
#include "ui_DlgEnvironment.h"

DlgEnvironment::DlgEnvironment(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgEnvironment)
{
    ui->setupUi(this);
}

/////////////////////////////////////////////////////////////////////////


void DlgEnvironment::addEnvironment(const CEnvironment *env){
  for (auto cont = env->containers().begin() ; cont != env->containers().end() ; cont ++){
    addContainer(&(*cont));
    addRemoteAccess(env, &(*cont));
  }
  ui->btn_ssh_all->setEnabled(env->healthy());
  ui->btn_desktop_all->setEnabled(env->healthy());
}

/////////////////////////////////////////////////////////////////////////


void DlgEnvironment::addContainer(const CHubContainer *cont){
  QLabel *cont_name = new QLabel(cont->name(), this);
  QLabel *cont_ip_port = new QLabel(cont->ip() + ":" + cont->port(), this);
  QLabel *cont_rhip = new QLabel(cont->rh_ip(), this);
  cont_name->setAlignment(Qt::AlignHCenter);
  cont_ip_port->setAlignment(Qt::AlignHCenter);
  cont_rhip->setAlignment(Qt::AlignHCenter);
  cont_name->setTextInteractionFlags(Qt::TextSelectableByMouse);
  cont_ip_port->setTextInteractionFlags(Qt::TextSelectableByMouse);
  cont_rhip->setTextInteractionFlags(Qt::TextSelectableByMouse);
  ui->cont_name->addWidget(cont_name);
  ui->cont_ip_port->addWidget(cont_ip_port);
  ui->cont_rhip->addWidget(cont_rhip);
}

/////////////////////////////////////////////////////////////////////////

void DlgEnvironment::addRemoteAccess(const CEnvironment *env, const CHubContainer *cont)
{
  QFont *font = new QFont();
  font->setPointSize(5);

  QPushButton *btn_ssh = new QPushButton("SSH", this);

  ui->cont_remote->addWidget(btn_ssh);

  btn_ssh->setMaximumHeight(14);

  btn_ssh->setFont(*font);

  if (!env->healthy()) { // if UNHEALTHY
    btn_ssh->setEnabled(false);
    return;
  }

  connect(ui->btn_ssh_all, &QPushButton::clicked, btn_ssh, &QPushButton::click);

  connect(btn_ssh, &QPushButton::clicked, [this, env, cont, btn_ssh](){
    QAction *act = new QAction();
    emit this->ssh_to_container_sig(env, cont, (void *)act);
    btn_ssh->setEnabled(false);
    this->ui->btn_ssh_all->setEnabled(false);
  });
}

/////////////////////////////////////////////////////////////////////////

DlgEnvironment::~DlgEnvironment() {
  delete ui;
}

/////////////////////////////////////////////////////////////////////////
