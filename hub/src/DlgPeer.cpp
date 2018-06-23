#include "DlgPeer.h"
#include "DlgRegisterPeer.h"
#include "PeerController.h"
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

DlgPeer::DlgPeer(QWidget *parent, QString peer_id)
    : QDialog(parent), ui(new Ui::DlgPeer) {
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
  // btn rh
  connect(ui->btn_destroy, &QPushButton::clicked, this,
          &DlgPeer::rh_destroy_sl);
  connect(ui->btn_start_stop, &QPushButton::clicked, this,
          &DlgPeer::rh_stop_or_start_sl);
  connect(ui->btn_reload, &QPushButton::clicked, this, &DlgPeer::rh_reload_sl);
  connect(ui->btn_register_unregister, &QPushButton::clicked, this,
          &DlgPeer::rh_register_or_unregister_sl);
  connect(ui->btn_update_peer, &QPushButton::clicked, this,
          &DlgPeer::rh_update_sl);
  connect(ui->btn_ssh_peer, &QPushButton::clicked, this, &DlgPeer::rh_ssh_sl);
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
  });
  // btn other
  connect(ui->btn_launch_console, &QPushButton::clicked, this,
          &DlgPeer::launch_console_sl);
  connect(ui->btn_peer_on_hub, &QPushButton::clicked, this,
          &DlgPeer::launch_bazaar_sl);

  this->ui->gr_ssh->setVisible(true);
  this->ui->gr_peer_control->setVisible(true);
  // vars
  registration_dialog = nullptr;
  ssh_available = false;
  advanced = false;
  hub_available = false;
  updatePeer();
  refresh_timer = new QTimer(this);
  refresh_timer->setInterval(7 * 1000); //7 seconds
  connect(refresh_timer, &QTimer::timeout, this, &DlgPeer::updatePeer);
  refresh_timer->start();
}

void DlgPeer::updatePeer() {
  if (TrayControlWindow::Instance()->my_peers_button_table.find(
          peer_fingerprint) ==
      TrayControlWindow::Instance()->my_peers_button_table.end()) {
    qCritical() << "Opened dialog for removed peer: " << peer_fingerprint;
    this->close();
  } else {
    TrayControlWindow::my_peer_button *peer_info =
        TrayControlWindow::Instance()->my_peers_button_table[peer_fingerprint];
    if (peer_info == nullptr) {
      qCritical() << "Opened dialog for removed peer: " << peer_fingerprint;
      this->close();
      return;
    }
    std::vector<CLocalPeer> local_peer_info;
    if (peer_info->m_local_peer != NULL) {
      local_peer_info.push_back(*(peer_info->m_local_peer));
    }
    addPeer(peer_info->m_hub_peer,             // bazaar peer info
            peer_info->m_network_peer == NULL  // lan peer info
                ? std::make_pair("", "")
                : *(peer_info->m_network_peer),
            local_peer_info);  // local peer info
  }
  this->adjustSize();
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
  peer_id = peer.id();
  peer_name = peer.name();
  peer_fingerprint = peer.fingerprint();
  ui->le_status->setText(peer.status());
  ui->btn_peer_on_hub->setEnabled(true);
  const std::vector<CMyPeerInfo::env_info> envs = peer.peer_environments();
  if (!hub_available)  // update envs info is not implemented yet
    update_environments(envs);
  hub_available = true;
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
  parse_yml();
}
// takes information about rh from configuration file
// format of configuration file -> KEY : VALUE
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
  if (hub_peer != nullptr) {
    addHubPeer(*hub_peer);
  } else {  // peer is not registered to the bazaar
    hub_available = false;
    envs_available = false;
    ui->btn_peer_on_hub->setEnabled(false);
    ui->btn_register_unregister->setText(tr("Register peer to Bazaar"));
    ui->btn_register_unregister->setToolTip(
        tr("Register peer to your Bazaar account"));
    hideEnvs();
  }
  if (!local_peer.first.isEmpty()) {
    addLocalPeer(local_peer);
  } else {  // peer was not found in lan network
    ssh_available = false;
  }
  if (!lp.empty()) {
    addMachinePeer(lp[0]);
  } else {  // cc didn't see this peer in your machine
    advanced = false;
    ssh_available = false;
  }
  updateUI();
}

void DlgPeer::updateUI() {
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
  } else {  // this means you don't have any access to the peer
    ui->btn_launch_console->setEnabled(false);
    hideSSH();
  }
  if (advanced) {
    ui->gr_peer_control->setTitle(tr("Peer info"));
    ui->le_name->setText(rh_name);
    if (hub_available) {
      this->setWindowTitle(peer_name);
      ui->btn_register_unregister->setText(tr("Unregister from Bazaar"));
      ui->btn_register_unregister->setToolTip(
          tr("Unregister peer from your Bazaar account"));
    } else {
      this->setWindowTitle(rh_name);
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
        ui->btn_start_stop->setText(tr("Start peer"));
        ui->btn_start_stop->setToolTip(tr("Start your peer"));
      } else {
        ui->btn_start_stop->setText(tr("Start peer"));
        ui->btn_start_stop->setToolTip(tr("Start your peer"));
      }
    }
    ui->le_status->setText(rh_status.toUpper());
    ui->le_status->setToolTip(
        CPeerController::Instance()->status_description(rh_status));
    configs();
    // slots
    enabled_peer_buttons(true);
  } else
    hidePeer();
}

void DlgPeer::launch_console_sl() {
  QString console_address =
      advanced ? "https://localhost:%1" : "https://%1:8443";
  CHubController::Instance().launch_browser(
      QString(console_address).arg(this->ssh_ip));
}

void DlgPeer::launch_bazaar_sl() {
  if (!hub_available) {
    qCritical() << "Pressed launch bazaar for nonregistered peer"
                << peer_fingerprint;
    return;
  }
  CHubController::Instance().launch_peer_page(peer_id.toInt());
}

void DlgPeer::rh_ssh_sl() {
  if (!advanced) {
    this->ui->btn_ssh_peer->setEnabled(false);
    this->ui->btn_ssh_peer->setText(tr("PROCESSING.."));
    emit this->ssh_to_rh_sig(this->peer_fingerprint);
  } else
    rh_ssh();
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
    if (DlgRegisterPeer::dialog_used[ssh_ip.toInt() - 9999] == 1 ||
        DlgRegisterPeer::dialog_running[ssh_ip.toInt() - 9999] == 1) {
      ui->btn_register_unregister->setEnabled(false);
    } else {
      ui->btn_register_unregister->setEnabled(state);
    }
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
void DlgPeer::rh_register_or_unregister_sl() {
  hub_available ? rh_unregister() : rh_register();
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
  connect(dlg_unregister, &DlgRegisterPeer::register_finished,
          [this]() { this->close(); });
}

void DlgPeer::regDlgClosed() {
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
}

void DlgPeer::hidePeer() {
  ui->lbl_name->hide();
  ui->le_name->hide();
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
}

void DlgPeer::hideEnvs() {
  ui->lbl_env_info->setText(tr("No \"environments\" on this peer."));
  ui->lbl_env->hide();
  ui->lbl_env_owner->hide();
  ui->lbl_env_status->hide();
  ui->line_1->hide();
  ui->line_2->hide();
  ui->line_3->hide();
}

void DlgPeer::rh_stop_or_start_sl() {
  ui->btn_start_stop->text() == tr("Start peer") ? rh_start() : rh_stop();
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

void DlgPeer::rh_destroy_sl() {
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

void DlgPeer::rh_reload_sl() {
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
                     "until processing has completed.")
                      .arg(rh_name),
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

void DlgPeer::rh_update_sl() {
  ui->btn_update_peer->setEnabled(false);
  ui->btn_update_peer->setText(tr("Updating"));
  ui->btn_update_peer->setToolTip(
      tr("This peer is currently updating. Please wait"));
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
  } else {
    envs_available = false;
    hideEnvs();
  }
}

DlgPeer::~DlgPeer() {
  qDebug() << "Deleting DlgPeer";
  delete ui;
}
