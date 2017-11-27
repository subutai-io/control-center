#include "DlgEnvironment.h"
#include "ui_DlgEnvironment.h"

DlgEnvironment::DlgEnvironment(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgEnvironment)
{
    ui->setupUi(this);
}

void DlgEnvironment::addEnvironment(const CEnvironment *env){
  for (auto cont = env->containers().cbegin() ; cont != env->containers().cend() ; cont ++){
    addContainer(&(*cont));
  }
  ui->btn_ssh_all->setEnabled(env->healthy());
  connect(ui->btn_ssh_all, &QPushButton::clicked,
          this, &DlgEnvironment::btn_ssh_all_clicked_sl);
}

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

void DlgEnvironment::set_button_ssh(QAction *act) {
  QPushButton *btn = new QPushButton("SSH" , this);
  connect(btn, &QPushButton::clicked, [act, btn](){
    act->trigger();
    btn->setEnabled(false);
  });

  connect(act, &QAction::changed, [btn](){
      btn->setEnabled(true);
  });

  QFont font = btn->font();
  font.setPointSize(5);
  btn->setParent(this);
  btn->setMaximumHeight(14);
  btn->setFont(font);
  ui->cont_remote->addWidget(btn);
}




DlgEnvironment::~DlgEnvironment() {
  delete ui;
}

void DlgEnvironment::btn_ssh_all_clicked_sl(){
  emit btn_ssh_all_clicked();
}
