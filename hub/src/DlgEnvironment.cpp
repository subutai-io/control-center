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

  this->setMinimumWidth(this->width());/**
  ui->cont_name->setAlignment(Qt::AlignHCenter);
  ui->cont_rhip->setAlignment(Qt::AlignHCenter);
  ui->cont_desktop_info->setAlignment(Qt::AlignHCenter);
  ui->cont_select->setAlignment(Qt::AlignHCenter);**/
  connect(ui->btn_desktop_selected, &QPushButton::clicked,
          this, &DlgEnvironment::desktop_selected);
  connect(ui->btn_ssh_selected, &QPushButton::clicked,
          this, &DlgEnvironment::ssh_selected);
  connect(ui->btn_upload_selected, &QPushButton::clicked,
          this, &DlgEnvironment::upload_selected);
  connect(ui->select_all, &QCheckBox::toggled,
          this, &DlgEnvironment::select_all);
}

/////////////////////////////////////////////////////////////////////////

void DlgEnvironment::select_all(bool checked) {
  for (CHubContainer cont : env.containers()) {
    if(selected_conts.find(cont.id()) == selected_conts.end())
        continue;
    QCheckBox *current_check_box = selected_conts[cont.id()];
    current_check_box->setChecked(checked);
  }
}

/////////////////////////////////////////////////////////////////////////

void DlgEnvironment::upload_selected() {
  ui->btn_upload_selected->setEnabled(false);
  ui->btn_upload_selected->setText(tr("Loading.."));
  for (CHubContainer cont : env.containers()) {
    if(selected_conts.find(cont.id()) == selected_conts.end())
      continue;
    QCheckBox *current_check_box = selected_conts[cont.id()];
    if (current_check_box->isChecked())
    {
      emit upload_to_container_sig(env, cont);
    }
  }

  QTimer::singleShot(2000, this, [this](){
    this->ui->btn_upload_selected->setEnabled(true);
    this->ui->btn_upload_selected->setText(tr("Transfer File"));
  });
}

/////////////////////////////////////////////////////////////////////////

void DlgEnvironment::desktop_selected() {
  ui->btn_desktop_selected->setEnabled(false);
  ui->btn_desktop_selected->setText(tr("Opening X2Go-Client.."));
  for (CHubContainer cont : env.containers()) {
    if(selected_conts.find(cont.id()) == selected_conts.end())
        continue;
    QCheckBox *current_check_box = selected_conts[cont.id()];
    if (current_check_box->isChecked())
    {
      emit desktop_to_container_sig(env, cont);
    }
  }
  QTimer::singleShot(2000, this, [this](){
    this->ui->btn_desktop_selected->setEnabled(true);
    this->ui->btn_desktop_selected->setText(tr("Remote Desktop"));
  });
}

/////////////////////////////////////////////////////////////////////////

void DlgEnvironment::ssh_selected() {
  ui->btn_ssh_selected->setEnabled(false);
  ui->btn_ssh_selected->setText(tr("Running SSH commands.."));
  for (CHubContainer cont : env.containers()) {
    if(selected_conts.find(cont.id()) == selected_conts.end())
        continue;
    QCheckBox *current_check_box = selected_conts[cont.id()];
    if (current_check_box->isChecked())
    {
      emit ssh_to_container_sig(env, cont);
    }
  }
  QTimer::singleShot(2000, this, [this](){
    this->ui->btn_ssh_selected->setEnabled(true);
    this->ui->btn_ssh_selected->setText(tr("SSH"));
  });
}

/////////////////////////////////////////////////////////////////////////

void DlgEnvironment::addEnvironment(const CEnvironment *_env) {
  env= *_env;

  qDebug() << "Environment added env: " << env.name();

  int row = 0;
  for (auto cont : env.containers()) {
    addContainer(&cont, row++);
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
  QTimer *timer = new QTimer(this);
  timer->setInterval(7000);
  connect(timer, &QTimer::timeout, this, &DlgEnvironment::check_environment_status);
  timer->start();
  check_environment_status();
  timers.push_back(timer);
}

//////////////////////////////////////////////////////////////////////////

void DlgEnvironment::addContainer(const CHubContainer *cont, int row) {
  qDebug() << QString("Adding container cont: %1, env: %2 ").arg(cont->name(), env.name());
  QLabel *cont_name = new QLabel(cont->name(), this);
  QLabel *cont_rhip_port = new QLabel(cont->rh_ip() + ":" + cont->port(), this);
  QLabel *cont_desktop_info = new QLabel(cont->is_desktop() ? cont->desk_env().isEmpty() ? "MATE" :  cont->desk_env()  : "No Desktop", this);
  QCheckBox *cont_select = new QCheckBox("READY" , this);

  labels.push_back(cont_name);
  labels.push_back(cont_rhip_port);
  labels.push_back(cont_desktop_info);
  checkboxs.push_back(cont_select);

  cont_name->setTextInteractionFlags(Qt::TextSelectableByMouse);
  cont_rhip_port->setTextInteractionFlags(Qt::TextSelectableByMouse);
  cont_desktop_info->setTextInteractionFlags(Qt::TextSelectableByMouse);
  cont_select->setStyleSheet("QCheckBox {color : green;}");

  if (row == 0) {
    for (int i = 0; i < 4; i++) {
      ui->cont_data->setColumnStretch(i, 1);
    }
  }
  ui->cont_data->setRowStretch(row, 1);

  ui->cont_data->addWidget(cont_name, row, 0, Qt::AlignCenter);
  ui->cont_data->addWidget(cont_rhip_port, row, 1, Qt::AlignCenter);
  ui->cont_data->addWidget(cont_desktop_info, row, 2, Qt::AlignCenter);
  ui->cont_data->addWidget(cont_select, row, 3, Qt::AlignCenter);

  selected_conts[cont->id()] = cont_select;
  desktops_info[cont->id()] = cont_desktop_info;
  connect(cont_select, &QCheckBox::stateChanged, [this](){
      this->check_buttons();
  });
}

/////////////////////////////////////////////////////////////////////////

void DlgEnvironment::change_cont_status(const CHubContainer *cont, int status) {
  if(selected_conts.find(cont->id()) == selected_conts.end())
      return;
  QCheckBox *cont_checkbox = selected_conts[cont->id()];
  if (status == 0) {
    cont_checkbox->setText(tr("READY"));
    cont_checkbox->setStyleSheet("QCheckBox {color : green;}");
    cont_checkbox->setToolTip(P2PController::p2p_connection_status_to_str(P2PController::CONNECTION_SUCCESS));
  }
  else
  if (status == 1) {
    cont_checkbox->setText(tr("CONNECTING"));
    cont_checkbox->setStyleSheet("QCheckBox {color : blue;}");
    cont_checkbox->setToolTip(P2PController::p2p_connection_status_to_str(P2PController::CANT_CONNECT_CONT));
  }
  else
  if (status == 2) {
    cont_checkbox->setText(tr("WAITING"));
    cont_checkbox->setStyleSheet("QCheckBox {color : blue;}");
    cont_checkbox->setToolTip(tr("Environment is UNDER_MODIFICATION"));
  }
  else
  if (status == 4) {
    cont_checkbox->setText(tr("FAILED"));
    cont_checkbox->setStyleSheet("QCheckBox {color : red;}");
    cont_checkbox->setToolTip(tr("Environment is not HEALTHY"));
  }
  else {
    if (!cont_checkbox->text().contains("INITIALIZING") && cont_checkbox->text() != "FAILED") {
      cont_checkbox->setText(tr("INITIALIZING"));
      cont_checkbox->setStyleSheet("QCheckBox {color : orange;}");
      cont_checkbox->setToolTip(tr("Trying to initialize a connection"));
    }
    else
    if (!cont_checkbox->text().contains("INITIALIZING.") && cont_checkbox->text() != "FAILED") {
      cont_checkbox->setText(tr("INITIALIZING."));
    }
    else
    if (!cont_checkbox->text().contains("INITIALIZING..") && cont_checkbox->text() != "FAILED") {
      cont_checkbox->setText(tr("INITIALIZING.."));
    }
    else
    if (!cont_checkbox->text().contains("INITIALIZING...") && cont_checkbox->text() != "FAILED") {
      cont_checkbox->setText(tr("INITIALIZING..."));
    }
    else
    if (status == 3) {
      cont_checkbox->setText(tr("FAILED"));
      cont_checkbox->setStyleSheet("QCheckBox {color : red;}");
      cont_checkbox->setToolTip(P2PController::p2p_connection_status_to_str(P2PController::CANT_JOIN_SWARM));
    }
    else
    if (status == 5) {
      cont_checkbox->setText(tr("FAILED"));
      cont_checkbox->setStyleSheet("QCheckBox {color : red;}");
      cont_checkbox->setToolTip(tr("P2P is not running"));
    }
  }
}

/////////////////////////////////////////////////////////////////////////

void DlgEnvironment::check_container_status(const CHubContainer *cont) {
  qDebug() << "Checking the status of container: " << cont->name() << " in " << env.name();
  P2PController::P2P_CONNECTION_STATUS
      cont_status = P2PController::Instance().is_ready(env, *cont);
  change_cont_status(cont, cont_status != P2PController::CONNECTION_SUCCESS);
  check_buttons();
}

/////////////////////////////////////////////////////////////////////////

void DlgEnvironment::check_environment_status() {
  qDebug()
      << "Checking the status of environment " << env.name();
  static int state_all;
  if(TrayControlWindow::Instance()->environments_table.find(env.id()) == TrayControlWindow::Instance()->environments_table.end()){
      this->close();
  }
  CEnvironment update_env = TrayControlWindow::Instance()->environments_table[env.id()];
  if(update_env.status() != env.status())
      this->close();
  if(update_env.name() != env.name())
      this->close();
  env = update_env;

  if (env.status() == "UNDER_MODIFICATION")
    state_all = 2;
  else if(!env.healthy())
    state_all = 4;
  else if(TrayControlWindow::Instance()->p2p_current_status != P2PStatus_checker::P2P_RUNNING)
    state_all = 5;
  else {
    P2PController::P2P_CONNECTION_STATUS
      swarm_status = P2PController::Instance().is_swarm_connected(env);

    bool connected_to_swarm = (swarm_status == P2PController::CONNECTION_SUCCESS);

    if (connected_to_swarm) {
      for (auto cont : env.containers()) {
        if(desktops_info.find(cont.id()) == desktops_info.end())
          return;
        desktops_info[cont.id()]->setText(QString(cont.is_desktop() ? cont.desk_env().isEmpty() ? "MATE" :  cont.desk_env()  : "No Desktop"));
        check_container_status(&cont);
      }
      return;
    }
    state_all = 3;
  }
  for (auto cont : env.containers())
    change_cont_status(&cont, state_all);
  check_buttons();
}

////////////////////////////////////////////////////////////////////////////
void DlgEnvironment::check_buttons() {
    qDebug()
        << "Checking which button I can press" << env.name();
    bool not_empty = false;
    bool upload = true, ssh = true, desktop =  true;
    P2PController::P2P_CONNECTION_STATUS
        swarm_status = P2PController::Instance().is_swarm_connected(env);
    bool connected_to_swarm = (env.healthy() & (swarm_status == P2PController::CONNECTION_SUCCESS));
    if(connected_to_swarm){
        for (CHubContainer cont : env.containers()) {
            if(selected_conts.find(cont.id()) == selected_conts.end())
                continue;
            QCheckBox *current_check_box = selected_conts[cont.id()];
            if(current_check_box->checkState() == Qt::Checked){
              if(current_check_box->text() != "READY"){
                  upload =  ssh = desktop = false;
                  break;
                }
              if(!cont.is_desktop())
                  desktop = false;
              not_empty = true;
              }
        }
    }
    else{
        upload = ssh = desktop = false;
    }
    ui->btn_desktop_selected->setEnabled(desktop & not_empty);
    ui->btn_upload_selected->setEnabled(upload & not_empty);
    ui->btn_ssh_selected->setEnabled(ssh & not_empty);
}
////////////////////////////////////////////////////////////////////////////

DlgEnvironment::~DlgEnvironment() {
  for (size_t counter = 0; counter < timers.size(); counter++)
      delete timers[counter];
  for (size_t counter = 0; counter < labels.size(); counter++)
      delete labels[counter];
  for (size_t counter = 0; counter < checkboxs.size(); counter++)
      delete checkboxs[counter];
  delete ui;
}
