#include "DlgEnvironment.h"
#include "ui_DlgEnvironment.h"
#include "P2PController.h"
#include <QToolTip>

DlgEnvironment::DlgEnvironment(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgEnvironment)
{
    ui->setupUi(this);
    qDebug() << "Environment dialog is initialized";

    this->setMinimumWidth(this->width());
    ui->cont_name->setAlignment(Qt::AlignHCenter);
    ui->cont_rhip->setAlignment(Qt::AlignHCenter);
    ui->cont_desktop_info->setAlignment(Qt::AlignHCenter);
    ui->cont_select->setAlignment(Qt::AlignHCenter);
}

/////////////////////////////////////////////////////////////////////////

void DlgEnvironment::addEnvironment(const CEnvironment *_env) {
  env= *_env;

  qDebug() << "Environment added env: " << env.name();

  for (auto cont : env.containers()) {
    addContainer(&cont);
  }

  for (auto cont : env.containers()) {
    // remote_acces(cont);
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

//////////////////////////////////////////////////////////////////////////

void DlgEnvironment::addContainer(const CHubContainer *cont) {
  qDebug() << QString("Adding container cont: %1, env: %2 ").arg(cont->name(), env.name());
  QLabel *cont_name = new QLabel(cont->name(), this);
  QLabel *cont_rhip_port = new QLabel(cont->rh_ip() + ":" + cont->port(), this);
  QLabel *cont_desktop_info = new QLabel(cont->is_desktop() ? cont->desk_env().isEmpty() ? "MATE" :  cont->desk_env()  : "No Desktop", this);
  QCheckBox *cont_select = new QCheckBox("READY" , this);

  cont_name->setTextInteractionFlags(Qt::TextSelectableByMouse);
  cont_rhip_port->setTextInteractionFlags(Qt::TextSelectableByMouse);
  cont_desktop_info->setTextInteractionFlags(Qt::TextSelectableByMouse);
  cont_select->setStyleSheet("QCheckBox {color : green;}");

  ui->cont_name->addWidget(cont_name);
  ui->cont_rhip->addWidget(cont_rhip_port);
  ui->cont_desktop_info->addWidget(cont_desktop_info);
  ui->cont_select->addWidget(cont_select);
  selected_conts[cont->id()] = cont_select;
}

/////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////

void DlgEnvironment::change_cont_status(const CHubContainer *cont, int status) {
  QCheckBox *cont_checkbox = selected_conts[cont->id()];
  if (status == 0)
  {
    cont_checkbox->setText(tr("READY"));
    cont_checkbox->setStyleSheet("QCheckBox {color : green;}");
  }
  else
  if (status == 1)
  {
    cont_checkbox->setText(tr("CONNECTING"));
    cont_checkbox->setStyleSheet("QCheckBox {color : yellow;}");
  }
  else
  if (status == 2){
    cont_checkbox->setText(tr("FAILED"));
    cont_checkbox->setStyleSheet("QCheckBox {color : red;}");
  }
}

void DlgEnvironment::check_container_status(const CHubContainer *cont) {
  qDebug() << "Checking the status of container: " << cont->name() << " in " << env.name();
  P2PController::P2P_CONNETION_STATUS
      cont_status = P2PController::Instance().is_ready(env, *cont);
  bool ready = (P2PController::Instance().is_ready(env, *cont) == P2PController::CONNECTION_SUCCESS);
  change_cont_status(cont, cont_status != P2PController::CONNECTION_SUCCESS);
}


/////////////////////////////////////////////////////////////////////////

void DlgEnvironment::check_environment_status() {
  qDebug()
      << "Checking the status of environment " << env.name();
  P2PController::P2P_CONNETION_STATUS
      swarm_status = P2PController::Instance().is_swarm_connected(env);

  bool connected_to_swarm = (env.healthy() & (swarm_status == P2PController::CONNECTION_SUCCESS));

  if (connected_to_swarm){
    for (auto cont : env.containers()){
      check_container_status(&cont);
    }
  }
  else {
    qDebug() << "Not connected to  swarm";
    for (auto cont : env.containers()) {
      change_cont_status(&cont, 2);
    }
  }
}



////////////////////////////////////////////////////////////////////////////

DlgEnvironment::~DlgEnvironment() {
  delete ui;
}
