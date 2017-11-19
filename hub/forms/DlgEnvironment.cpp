#include "DlgEnvironment.h"
#include "ui_DlgEnvironment.h"

DlgEnvironment::DlgEnvironment(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgEnvironment)
{
    ui->setupUi(this);
}


#include <QString>

void DlgEnvironment::addEnvironment(const CEnvironment *env){
  for (auto cont = env->containers().cbegin() ; cont != env->containers().cend() ; cont ++){
    addContainer(&(*cont));
  }
}

void DlgEnvironment::addContainer(const CHubContainer *cont){
  QLabel *cont_name = new QLabel(cont->name(), this);
  QLabel *cont_ip = new QLabel(cont->ip(), this);
  QLabel *cont_port = new QLabel(cont->port(), this);
  QLabel *cont_rhip = new QLabel(cont->rh_ip(), this);
  cont_name->setAlignment(Qt::AlignHCenter);
  cont_ip->setAlignment(Qt::AlignHCenter);
  cont_port->setAlignment(Qt::AlignHCenter);
  cont_rhip->setAlignment(Qt::AlignHCenter);
  cont_name->setTextInteractionFlags(Qt::TextSelectableByMouse);
  cont_ip->setTextInteractionFlags(Qt::TextSelectableByMouse);
  cont_port->setTextInteractionFlags(Qt::TextSelectableByMouse);
  cont_rhip->setTextInteractionFlags(Qt::TextSelectableByMouse);
  ui->cont_name->addWidget(cont_name);
  ui->cont_ip->addWidget(cont_ip);
  ui->cont_port->addWidget(cont_port);
  ui->cont_rhip->addWidget(cont_rhip);
}

void DlgEnvironment::set_button_ssh(QPushButton *btn) {
  btn->setParent(this);
  btn->setMaximumHeight(17);
  QPushButton *tt = new QPushButton("EZ-DESKTOP",this);
  tt->setMaximumHeight(17);
  ui->cont_ssh->addRow(btn, tt);
}

DlgEnvironment::~DlgEnvironment() {
  delete ui;
}
