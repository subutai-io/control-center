#include "DlgPeer.h"
#include "DlgRegisterPeer.h"
#include "SystemCallWrapper.h"
#include "TrayControlWindow.h"
#include "ui_DlgPeer.h"

#include <QDir>
#include <QFormLayout>
#include <QGroupBox>
#include <QLayout>
#include <QLineEdit>
#include "OsBranchConsts.h"
#include "RhController.h"

#include "LibsshController.h"

DlgPeer::DlgPeer(QWidget *parent, QString peer_id) : QDialog(parent), ui(new Ui::DlgPeer) {
  peer_fingerprint = peer_id;
  qDebug() << "Peer dialog is initialized";
  // ui
  ui->setupUi(this);
  this->setMinimumWidth(this->width());
  this->ui->le_pass->setEchoMode(QLineEdit::PasswordEchoOnEdit);
  ui->gr_ssh->setVisible(true);
  ui->gr_peer_control->setVisible(false);
  ui->btn_launch_console->setEnabled(false);
  ui->lbl_update_peeros->setWordWrap(true);
  ui->lbl_env_info->setWordWrap(true);

  ui->le_name->setReadOnly(true);
  ui->le_cpu->setReadOnly(true);
  ui->le_ram->setReadOnly(true);
  ui->le_disk->setReadOnly(true);

  ui->cmb_bridge->setEnabled(false);
  ui->le_ram->setEnabled(false);
  ui->le_disk->setEnabled(false);
  ui->le_cpu->setEnabled(false);

  ui->le_cpu->setValidator(new QIntValidator(1, 16, this));
  ui->le_ram->setValidator(new QIntValidator(1, 100000, this));
  ui->le_disk->setValidator(new QIntValidator(1, 100000, this));

  QStringList bridges = CPeerController::Instance()->get_bridgedifs();
  ui->cmb_bridge->addItems(bridges);

  // slots
  connect(CRhController::Instance(), &CRhController::ssh_to_rh_finished, this,
          &DlgPeer::ssh_to_rh_finished_sl);

  this->ui->gr_ssh->setVisible(true);
  this->ui->gr_peer_control->setVisible(true);
  this->adjustSize();

  connect(ui->btn_launch_console, &QPushButton::clicked, [this]() {
    QString console_address =
        advanced ? "https://localhost:%1" : "https://%1:8443";
    CHubController::Instance().launch_browser(
        QString(console_address).arg(this->ssh_ip));
  });
  // vars
  registration_dialog = nullptr;
  ssh_available = false;
  advanced = false;
  hub_available = false;
}

void DlgPeer::addLocalPeer(std::pair<QString, QString> peer) {
  peer_fingerprint = peer.first;
  ssh_ip = peer.second;
  ssh_available = true;
  ui->btn_launch_console->setEnabled(true);
}
// when peer is registered to the bazaar
// CMyPeerInfo is peer information based on the bazaar
void DlgPeer::addHubPeer(CMyPeerInfo peer) {
  hub_available = true;
  connect(ui->btn_peer_on_hub, &QPushButton::clicked, [peer]() {
    CHubController::Instance().launch_peer_page(peer.id().toInt());
  });
  peer_name = peer.name();
  peer_fingerprint = peer.fingerprint();
  ui->le_status->setText(peer.status());
  const std::vector<CMyPeerInfo::env_info> envs = peer.peer_environments();
  update_environments(envs);
}
// add resource host, which should be management for full functionality
void DlgPeer::addMachinePeer(CLocalPeer peer) {
  advanced = true;
  rh_status = peer.status();
  rh_dir = peer.dir();
  rh_name = peer.name();
  management_ua = peer.update_available() == "true" ? true : false;
  // localhost port
  if (peer.ip() != "undefined" && !peer.ip().isEmpty() &&
      peer.ip() != "loading") {
    ssh_ip = peer.ip();
    ui->label->setText(tr("Host port:"));
    ui->btn_launch_console->setEnabled(true);
    ssh_available = true;
  } else {  // no port no management
    ui->btn_launch_console->setEnabled(false);
    if (rh_status == "running") {
      rh_status = "not ready";
    } else {  // if not running can't ssh
      ssh_available = false;
    }
  }
  // fingerprint
  if (peer.fingerprint() != "loading" && peer.fingerprint() != "undefined" &&
      !peer.ip().isEmpty()) {
    peer_fingerprint = peer.fingerprint();
  } else {  // if fingerprint is unknown means management is not ready yet
    ui->btn_launch_console->setEnabled(false);
    if (rh_status == "running") {
      rh_status = "not ready";
    } else {  // if not running can't ssh
      ssh_available = false;
    }
  }
  // management update
  if (management_ua) {
    ui->btn_update_peer->setText(tr("Update is available"));
    ui->btn_update_peer->setEnabled(true);
    ui->btn_update_peer->setToolTip(tr("Update for PeerOS is available"));
  } else {
    ui->btn_update_peer->setEnabled(false);
    if (peer.update_available() == "false") {
      ui->btn_update_peer->setText(tr("No updates available"));
      ui->btn_update_peer->setToolTip(tr("No updates for the PeerOS"));
    } else {
      ui->btn_update_peer->setText(tr("Updating..."));
      ui->btn_update_peer->setToolTip(
          tr("This peer is currently updating. Please wait"));
    }
  }
  // timer to refresh info about local rh
  timer_refresh_machine_peer = new QTimer(this);
  timer_refresh_machine_peer->setInterval(7000);
  connect(timer_refresh_machine_peer, &QTimer::timeout, this, [this]() {
    std::map<QString, CLocalPeer> local_peers =
        TrayControlWindow::Instance()->machine_peers_table;
    auto it_peer = local_peers.find(this->rh_name);
    if (it_peer == local_peers.end()) {
      this->close();
      return;
    }
    CLocalPeer peer = it_peer->second;
    this->management_ua = peer.update_available() == "true" ? true : false;
    if (management_ua) {
      ui->btn_update_peer->setText("Update is available");
      ui->btn_update_peer->setEnabled(true);
      ui->btn_update_peer->setToolTip(tr("Update for PeerOS is available"));
    } else {
      ui->btn_update_peer->setEnabled(false);
      if (peer.update_available() == "false") {
        ui->btn_update_peer->setText(tr("No updates available"));
        ui->btn_update_peer->setToolTip(tr("No updates for the PeerOS"));
      } else {
        ui->btn_update_peer->setText(tr("Updating..."));
        ui->btn_update_peer->setToolTip(
            tr("This peer is currently updating. Please wait"));
      }
    }
  });
  timer_refresh_machine_peer->start();
  parse_yml();
}
// takes information about rh from configuration file
void DlgPeer::parse_yml() {
  QString filename = QString("%1/vagrant-subutai.yml").arg(rh_dir);
  QFile file(filename);
  if (file.open(QIODevice::ReadWrite)) {
    QTextStream stream(&file);
    QString output = QString(stream.readAll());
    QStringList vagrant_info = output.split("\n", QString::SkipEmptyParts);
    QString flag, value;
    bool reading_value;
    for (auto s : vagrant_info) {
      s = s.trimmed();
      flag = value = "";
      reading_value = false;
      for (int i = 0; i < s.size(); i++) {
        if (reading_value) {
          value += s[i];
          continue;
        }
        if (s[i] == ':') {
          flag = value;
          value = "";
          continue;
        }
        if (s[i] != ' ') {
          if (value == "" && !flag.isEmpty()) reading_value = true;
          value += s[i];
        }
      }
      if (flag == "SUBUTAI_RAM") rh_ram = value;
      if (flag == "SUBUTAI_CPU") rh_cpu = value;
      if (flag == "DISK_SIZE") rh_disk = value;
      if (flag == "BRIDGE") rh_bridge = value;
    }
  }
  file.close();
}

void DlgPeer::addPeer(CMyPeerInfo *hub_peer,
                      std::pair<QString, QString> local_peer,
                      std::vector<CLocalPeer> lp) {
  static QPalette *green_text = new QPalette();
  green_text->setColor(QPalette::Text, Qt::green);
  static QPalette *yellow_text = new QPalette();
  yellow_text->setColor(QPalette::Text, Qt::yellow);
  static QPalette *red_text = new QPalette();
  red_text->setColor(QPalette::Text, Qt::red);
  if (hub_peer != nullptr) {
    addHubPeer(*hub_peer);
  } else {
    ui->btn_peer_on_hub->setEnabled(false);
    hideEnvs();
  }
  if (!local_peer.first.isEmpty()) {
    addLocalPeer(local_peer);
  }
  if (!lp.empty()) {
    addMachinePeer(lp[0]);
  }
  if (ssh_available) {  // parse SSH information
    if (ssh_ip.isEmpty())
      ssh_ip = CSettingsManager::Instance().rh_host(peer_fingerprint);
    if (ssh_ip.isEmpty()) ssh_ip = QString("127.0.0.1");
    ssh_port =
        QString::number(CSettingsManager::Instance().rh_port(peer_fingerprint));
    if (ssh_port.isEmpty() || ssh_port == "0") ssh_port = QString("22");
    ssh_user = CSettingsManager::Instance().rh_user(peer_fingerprint);
    if (ssh_user.isEmpty()) ssh_user = QString("subutai");
    ssh_pass = CSettingsManager::Instance().rh_pass(peer_fingerprint);
    if (ssh_pass.isEmpty()) ssh_pass = QString("ubuntai");

    ui->lbl_ip->setText(ssh_ip);
    ui->lbl_port->setText(ssh_port);
    ui->le_user->setText(ssh_user);
    ui->le_pass->setText(ssh_pass);

    CSettingsManager::Instance().set_rh_host(peer_fingerprint,
                                             this->ui->lbl_ip->text());
    CSettingsManager::Instance().set_rh_port(
        peer_fingerprint, this->ui->lbl_port->text().toInt());
    CSettingsManager::Instance().set_rh_user(peer_fingerprint,
                                             this->ui->le_user->text());
    CSettingsManager::Instance().set_rh_pass(peer_fingerprint,
                                             this->ui->le_pass->text());
    // slots
    if (!advanced) {
      connect(ui->btn_ssh_peer, &QPushButton::clicked, [this]() {
        this->ui->btn_ssh_peer->setEnabled(false);
        this->ui->btn_ssh_peer->setText(tr("PROCESSING.."));
        emit this->ssh_to_rh_sig(this->peer_fingerprint);
      });
    } else {
      connect(ui->btn_ssh_peer, &QPushButton::clicked, this, &DlgPeer::rh_ssh);
    }
  } else
    hideSSH();
  if (advanced) {
    ui->gr_peer_control->setTitle(tr("Peer info"));
    ui->le_name->setText(rh_name);
    if (hub_available) {
      ui->btn_register_unregister->setText(tr("Unregister from Bazaar"));
      ui->btn_register_unregister->setToolTip(tr("Unregister peer from your Bazaar account"));
    }
    if (rh_status == "running") {
      ui->btn_start_stop->setText(tr("Stop peer"));
      ui->btn_start_stop->setToolTip(tr("Turnoff your peer"));
    } else {
      ui->btn_register_unregister->setEnabled(false);
      ui->btn_launch_console->setEnabled(false);
      if (rh_status == "not ready") {
        ui->btn_start_stop->setText(tr("Stop peer"));
        ui->btn_start_stop->setToolTip(tr("Turnoff your peer"));
      } else if (rh_status == "not_created") {
        ui->btn_reload->setEnabled(false);
        ui->btn_start_stop->setEnabled(false);
      } else if (rh_status == "poweroff") {
        ui->btn_reload->setEnabled(false);
      }
    }
    ui->le_status->setText(rh_status.toUpper());
    ui->le_status->setToolTip(
        CPeerController::Instance()->status_description(rh_status));
    configs();
    // slots
    connect(ui->btn_destroy, &QPushButton::clicked,
            [this]() { this->rh_destroy(); });
    connect(ui->btn_start_stop, &QPushButton::clicked,
            [this]() { this->rh_stop_or_start(); });
    connect(ui->btn_reload, &QPushButton::clicked,
            [this]() { this->rh_reload(); });
    connect(ui->btn_register_unregister, &QPushButton::clicked,
            [this]() { this->rh_register_or_unregister(); });
    connect(ui->btn_update_peer, &QPushButton::clicked, this,
            &DlgPeer::rh_update);
    connect(ui->change_confugre, &QCheckBox::toggled, [this](bool checked) {
      ui->le_cpu->setReadOnly(!checked);
      ui->le_ram->setReadOnly(!checked);
      ui->le_disk->setReadOnly(!checked);

      ui->cmb_bridge->setEnabled(checked);
      ui->le_disk->setEnabled(checked);
      ui->le_cpu->setEnabled(checked);
      ui->le_ram->setEnabled(checked);
      if (checked == false) {
        this->configs();
      }
      static std::vector<QString> states = {tr("Modify peer info"),
                                            tr("Modify peer info")};
      ui->change_confugre->setText(states[checked]);
    });
    this->adjustSize();
  } else
    hidePeer();
  this->adjustSize();
}

void DlgPeer::configs() {
  ui->le_cpu->setText(rh_cpu);
  ui->le_ram->setText(rh_ram);
  ui->le_disk->setText(rh_disk);
  QStringList bridges = CSystemCallWrapper::list_interfaces();
  int index_bridge = -1;
  for (int i = 0; i < bridges.size(); i++) {
    QString s = bridges[i];
    s += "\"";
    s = "\"" + s;
    if (s == rh_bridge) {
      index_bridge = i;
      break;
    }
  }
  if (index_bridge >= 0) ui->cmb_bridge->setCurrentIndex(index_bridge);
}

bool DlgPeer::check_configs() {
  static bool bool_me;
  static int base = 10;
  int ram = ui->le_ram->text().toInt(&bool_me, base);
  int cpu = ui->le_cpu->text().toInt(&bool_me, base);
  int disk = ui->le_disk->text().toInt(&bool_me, base);
  if (ram < 2048) {
    CNotificationObserver::Error(tr("RAM size cannot be less than 2048 MB."),
                                 DlgNotification::N_NO_ACTION);
    return false;
  }
  if (cpu < 1) {
    CNotificationObserver::Error(
        tr("CPU cores quantity cannot be less than 1."),
        DlgNotification::N_NO_ACTION);
    return false;
  }
  if (disk < rh_disk.toInt(&bool_me, base)) {
    CNotificationObserver::Error(tr("You can only increase disk size."),
                                 DlgNotification::N_NO_ACTION);
    return false;
  }
  return true;
}

bool DlgPeer::change_configs() {
  QString filename = QString("%1/vagrant-subutai.yml").arg(rh_dir);
  QFile file(filename);
  if (!check_configs()) return false;
  rh_ram = ui->le_ram->text();
  rh_cpu = ui->le_cpu->text();
  rh_disk = ui->le_disk->text();
  rh_bridge = ui->cmb_bridge->currentText();
  if (file.exists()) {
    if (file.remove() && file.open(QIODevice::ReadWrite)) {
      QTextStream stream(&file);
      stream << "SUBUTAI_RAM : " << rh_ram << endl;
      stream << "SUBUTAI_CPU : " << rh_cpu << endl;
      QString branch = current_branch_name();
      if (branch == "production")
        stream << "SUBUTAI_ENV : "
               << "prod" << endl;
      else if (branch == "development")
        stream << "SUBUTAI_ENV : "
               << "dev" << endl;
      else
        stream << "SUBUTAI_ENV : "
               << "master" << endl;
      stream << "DISK_SIZE : " << rh_disk << endl;
      stream << "BRIDGE : " << QString("\"%1\"").arg(rh_bridge) << endl;
    } else {
      CNotificationObserver::Error(
          tr("Failed to create a .yml configuration file for this peer. You "
             "may create one manually. "
             "You may also try again by restarting the Control Center first or "
             "reloading the peer."),
          DlgNotification::N_NO_ACTION);
      return false;
    }
    file.close();
  } else {
    CNotificationObserver::Error(
        tr("Unable to find the .yml configuration file for this peer. You may "
           "create one manually. "
           "You may also try again by restarting the Control Center first or "
           "reloading the peer."),
        DlgNotification::N_NO_ACTION);
    return false;
  }
  return true;
}

void DlgPeer::enabled_peer_buttons(bool state) {
  if (rh_status == "running") {
    ui->btn_start_stop->setEnabled(state);
    ui->btn_reload->setEnabled(state);
    ui->btn_register_unregister->setEnabled(state);
    ui->btn_destroy->setEnabled(state);
  } else if (rh_status == "not ready") {
    ui->btn_start_stop->setEnabled(state);
    ui->btn_reload->setEnabled(state);
    ui->btn_destroy->setEnabled(state);
  } else {
    ui->btn_start_stop->setEnabled(state);
    ui->btn_destroy->setEnabled(state);
  }
}

void DlgPeer::ssh_to_rh_finished_sl(const QString &peer_fingerprint,
                                    system_call_wrapper_error_t res,
                                    int libbssh_exit_code) {
  qDebug() << "ssh to rh finished";
  UNUSED_ARG(res);
  UNUSED_ARG(libbssh_exit_code);  // need to use this variables to give feedback
                                  // to user
  if (QString::compare(peer_fingerprint, this->peer_fingerprint,
                       Qt::CaseInsensitive) != 0)
    return;

  ui->btn_ssh_peer->setEnabled(true);
  ui->btn_ssh_peer->setText(tr("SSH into Peer"));
}

void DlgPeer::rh_register_or_unregister() {
  if (hub_available) {
    rh_unregister();
  } else {
    rh_register();
  }
}

void DlgPeer::rh_register() {
  ui->btn_register_unregister->setEnabled(false);
  DlgRegisterPeer *dlg_register = new DlgRegisterPeer(this);
  const QString ip_addr = ui->lbl_ip->text();
  dlg_register->init(ip_addr, this->rh_name);
  dlg_register->setRegistrationMode();
  dlg_register->setWindowTitle(tr("Register peer: %1").arg(rh_name));
  dlg_register->show();
  registration_dialog = dlg_register;
  connect(dlg_register, &QDialog::finished, this, &DlgPeer::regDlgClosed);
  connect(dlg_register, &DlgRegisterPeer::register_finished,
          [this]() { this->close(); });
}

void DlgPeer::rh_unregister() {
  if (envs_available) {
    CNotificationObserver::Error(
        tr("Peers connected to environments cannot be unregistered. "
           "Remove the peer from the environment first before unregistering it "
           "from Bazaar."),
        DlgNotification::N_GO_TO_HUB);
    return;
  }
  ui->btn_register_unregister->setEnabled(false);
  DlgRegisterPeer *dlg_unregister = new DlgRegisterPeer(this);
  const QString ip_addr = ui->lbl_ip->text();
  dlg_unregister->init(ip_addr, peer_name);
  dlg_unregister->setUnregistrationMode();
  dlg_unregister->setWindowTitle(tr("Unregister peer: %1").arg(peer_name));
  dlg_unregister->show();
  registration_dialog = dlg_unregister;
  connect(dlg_unregister, &QDialog::finished, this, &DlgPeer::regDlgClosed);
  connect(dlg_unregister, &DlgRegisterPeer::register_finished,
          [this]() { this->close(); });
}

void DlgPeer::regDlgClosed() {
  ui->btn_register_unregister->setEnabled(true);
  ui->btn_register_unregister->setEnabled(true);
  if (registration_dialog == nullptr) return;
  registration_dialog->deleteLater();
}

void DlgPeer::hideSSH() {
  ui->gr_ssh->hide();
  ui->label->hide();
  ui->label_3->hide();
  ui->label_2->hide();
  ui->label_4->hide();
  ui->lbl_ip->hide();
  ui->lbl_port->hide();
  ui->le_user->hide();
  ui->le_pass->hide();
  ui->btn_ssh_peer->hide();
  this->adjustSize();
}

void DlgPeer::hidePeer() {
  ui->lbl_name->hide();
  ui->le_name->hide();
  // ui->lbl_status->hide(); ui->le_status->hide();
  ui->lbl_bridge->hide();
  ui->cmb_bridge->hide();
  ui->lbl_cpu->hide();
  ui->le_cpu->hide();
  ui->lbl_disk->hide();
  ui->le_disk->hide();
  ui->lbl_ram->hide();
  ui->le_ram->hide();

  // buttons
  ui->btn_start_stop->hide();
  ui->btn_start_stop->hide();
  ui->change_confugre->hide();
  ui->btn_reload->hide();
  ui->btn_destroy->hide();
  ui->btn_register_unregister->hide();
  ui->btn_register_unregister->hide();
  ui->btn_update_peer->hide();

  ui->lbl_update_peeros->setText(
      tr("This peer is not in your machine. "
         "Specific functions are available only for peers in your machine."));
  this->adjustSize();
}

void DlgPeer::hideEnvs() {
  ui->lbl_env_info->setText(tr("No \"environments\" on this peer."));

  ui->lbl_env->hide();
  ui->lbl_env_owner->hide();
  ui->lbl_env_status->hide();
  ui->line_1->hide();
  ui->line_2->hide();
  ui->line_3->hide();
  this->adjustSize();
}

void DlgPeer::rh_stop_or_start() {
  if (ui->btn_start_stop->text() == tr("Start peer")) {
    rh_start();
  } else {
    rh_stop();
  }
}

void DlgPeer::rh_stop() {
  enabled_peer_buttons(false);
  static QString stop_command = "halt";
  CommandPeerTerminal *thread_init = new CommandPeerTerminal(this);
  ui->btn_start_stop->setText(tr("Trying to stop this peer, please wait."));
  thread_init->init(rh_dir, stop_command, rh_name);
  emit peer_modified(rh_name);
  thread_init->startWork();
  connect(thread_init, &CommandPeerTerminal::outputReceived,
          [this](system_call_wrapper_error_t res) {
            if (res == SCWE_SUCCESS) {
              CNotificationObserver::Instance()->Info(
                  tr("The process to stop peer %1 has started. "
                     "Do not close this terminal until processing "
                     "has completed.")
                      .arg(this->ui->le_name->text()),
                  DlgNotification::N_NO_ACTION);
              this->close();
            } else {
              CNotificationObserver::Instance()->Error(
                  tr("Failed to stop this peer \"%1\". "
                     "Check the stability of your Internet connection first, "
                     "before trying again.")
                      .arg(this->ui->le_name->text()),
                  DlgNotification::N_NO_ACTION);
              enabled_peer_buttons(true);
              ui->btn_start_stop->setText(tr("Stop peer"));
            }
          });
}

void DlgPeer::rh_start() {
  static QString up_command = "up";
  enabled_peer_buttons(false);
  if (ui->change_confugre->isChecked()) {
    if (!change_configs()) {
      enabled_peer_buttons(true);
      return;
    }
  }
  CommandPeerTerminal *thread_init = new CommandPeerTerminal(this);
  ui->btn_start_stop->setText(tr("Trying to launch this peer, please wait."));
  thread_init->init(rh_dir, up_command, rh_name);
  thread_init->startWork();
  connect(thread_init, &CommandPeerTerminal::outputReceived,
          [this](system_call_wrapper_error_t res) {
            if (res == SCWE_SUCCESS) {
              emit peer_modified(this->rh_name);
              CNotificationObserver::Instance()->Info(
                  tr("The process to launch peer %1 has started. "
                     "Do not close this terminal until processing "
                     "has completed.")
                      .arg(this->ui->le_name->text()),
                  DlgNotification::N_NO_ACTION);
              this->close();
            } else {
              CNotificationObserver::Instance()->Error(
                  tr("Failed to launch this peer \"%1\". "
                     "Check the stability of your Internet connection first, "
                     "before trying again.")
                      .arg(this->ui->le_name->text()),
                  DlgNotification::N_NO_ACTION);
              enabled_peer_buttons(true);
              ui->btn_start_stop->setText(tr("Start peer"));
            }
          });
}

void DlgPeer::rh_ssh() {
  static QString up_command = "ssh";
  enabled_peer_buttons(false);
  CommandPeerTerminal *thread_init = new CommandPeerTerminal(this);
  ui->btn_ssh_peer->setText(tr("Processing..."));
  thread_init->init(rh_dir, up_command, rh_name);
  thread_init->startWork();
  connect(thread_init, &CommandPeerTerminal::outputReceived,
          [this](system_call_wrapper_error_t res) {
            if (res == SCWE_SUCCESS) {
              qDebug() << "sshed to peer" << rh_name;
            } else {
              CNotificationObserver::Instance()->Error(
                  tr("Failed to SSH to the peer \"%1\". Check the stability of "
                     "your Internet connection first, "
                     "before trying again.")
                      .arg(this->ui->le_name->text()),
                  DlgNotification::N_NO_ACTION);
            }
            enabled_peer_buttons(true);
            ui->btn_ssh_peer->setText(tr("SSH into Peer"));
          });
}

void DlgPeer::rh_destroy() {
  if (hub_available) {
    CNotificationObserver::Error(tr("Registered peers cannot be destroyed. "
                                    "Before destroying this peer, you must "
                                    "unregister it first from Subutai Bazaar."),
                                 DlgNotification::N_NO_ACTION);
    return;
  }
  enabled_peer_buttons(false);
  if (rh_status == "not_created") {
    QDir del_me(rh_dir);
    if (del_me.removeRecursively()) {
      CNotificationObserver::Instance()->Info(tr("The peer is now destroyed."),
                                              DlgNotification::N_NO_ACTION);
      this->close();
    } else {
      CNotificationObserver::Instance()->Error(
          tr("Failed to delete the peer directory. Make sure that you have the "
             "required Administrative privileges."),
          DlgNotification::N_NO_ACTION);
      enabled_peer_buttons(true);
    }
    return;
  }
  static QString delete_command = "destroy -f";
  CommandPeerTerminal *thread_init = new CommandPeerTerminal(this);
  ui->btn_destroy->setText(tr("Trying to destroy this peer, please wait."));
  thread_init->init(rh_dir, delete_command, rh_name);
  thread_init->startWork();
  connect(thread_init, &CommandPeerTerminal::outputReceived,
          [this](system_call_wrapper_error_t res) {
            if (res == SCWE_SUCCESS) {
              CNotificationObserver::Instance()->Info(
                  tr("The process to destroy peer %1 has started. Do not close "
                     "this terminal until "
                     "processing has completed.")
                      .arg(rh_name),
                  DlgNotification::N_NO_ACTION);
              this->close();
            } else {
              CNotificationObserver::Instance()->Error(
                  tr("Failed to destroy peer \"%1\". Make sure that your "
                     "Internet connection is stable and "
                     "that you have the required administrative privileges.")
                      .arg(rh_name),
                  DlgNotification::N_NO_ACTION);
              enabled_peer_buttons(true);
              ui->btn_destroy->setText(tr("Destroy"));
            }
          });
  return;
}

void DlgPeer::rh_reload() {
  static QString reload_command = "reload";
  enabled_peer_buttons(false);
  if (ui->change_confugre->isChecked()) {
    if (!change_configs()) {
      enabled_peer_buttons(true);
      return;
    }
  }
  CommandPeerTerminal *thread_init = new CommandPeerTerminal(this);
  ui->btn_reload->setText(tr("Trying to reload this peer, please wait."));
  thread_init->init(rh_dir, reload_command, rh_name);
  thread_init->startWork();
  emit peer_modified(this->rh_name);
  connect(thread_init, &CommandPeerTerminal::outputReceived,
          [this](system_call_wrapper_error_t res) {
            if (res == SCWE_SUCCESS) {
              CNotificationObserver::Instance()->Info(
                  tr("The process to reload peer %1 has started. Do not close "
                     "this terminal "
                     "until processing has completed.").arg(rh_name),
                  DlgNotification::N_NO_ACTION);
              this->close();
            } else {
              CNotificationObserver::Instance()->Error(
                  tr("Failed to reload peer \"%1\". Check the stability of "
                     "your Internet connection first, "
                     "before trying again.")
                      .arg(this->ui->le_name->text()),
                  DlgNotification::N_NO_ACTION);
              enabled_peer_buttons(true);
              ui->btn_start_stop->setText(tr("Reload"));
            }
          });
}

void DlgPeer::rh_update() {
  ui->btn_update_peer->setEnabled(false);
  ui->btn_update_peer->setText(tr("Updating"));
  ui->btn_update_peer->setToolTip(
      tr("This peer is currently updating. Please wait"));
  timer_refresh_machine_peer->start();
  emit this->peer_update_peeros(peer_fingerprint);
}

void DlgPeer::update_environments(
    const std::vector<CMyPeerInfo::env_info> &envs) {
  if (!envs.empty()) {
    for (CMyPeerInfo::env_info env : envs) {
      // env name
      QLabel *env_name = new QLabel(env.envName);
      env_name->setAlignment(Qt::AlignHCenter);
      env_name->setTextInteractionFlags(Qt::TextSelectableByMouse);
      // env owner
      QLabel *env_owner = new QLabel(
          QString("<a href=\"%1\">%2</a>")
              .arg(hub_user_profile_url() + QString::number(env.ownerId))
              .arg(env.ownerName));
      env_owner->setAlignment(Qt::AlignHCenter);
      env_owner->setTextFormat(Qt::RichText);
      env_owner->setTextInteractionFlags(Qt::TextBrowserInteraction);
      env_owner->setOpenExternalLinks(true);
      // env status
      QLabel *env_status = new QLabel(env.status);
      env_status->setAlignment(Qt::AlignHCenter);
      env_status->setTextInteractionFlags(Qt::TextSelectableByMouse);

      ui->env_name->addWidget(env_name);
      ui->env_owner->addWidget(env_owner);
      ui->env_status->addWidget(env_status);
    }
    envs_available = true;
    this->adjustSize();
  } else {
    envs_available = false;
    this->adjustSize();
    hideEnvs();
  }
}

DlgPeer::~DlgPeer() {
  qDebug() << "Deleting DlgPeer";
  delete ui;
}
