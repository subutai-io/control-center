#include "DlgCreatePeer.h"
#include "DlgNotification.h"
#include "NotificationObserver.h"
#include "QDir"
#include "QStandardPaths"
#include "SettingsManager.h"
#include "SystemCallWrapper.h"
#include "TrayControlWindow.h"
#include "ui_DlgCreatePeer.h"
#include "VagrantProvider.h"
#include "Environment.h"
#include "updater/HubComponentsUpdater.h"
#include <QMessageBox>

bool non_static_vmware_utility_update_available() {
  QString version;
  CSystemCallWrapper::vmware_utility_version(version);
  return version == "undefined";
}

bool is_update_available_vmware_utility_th() {
  QFutureWatcher<bool> *watcher = new QFutureWatcher<bool>();
  QFuture<bool> res =
      QtConcurrent::run(non_static_vmware_utility_update_available);
  watcher->setFuture(res);
  watcher->waitForFinished();
  return res.result();
}

DlgCreatePeer::DlgCreatePeer(QWidget *parent)
    : QDialog(parent),
      m_password_state(0),
      m_password_confirm_state(0),
      ui(new Ui::DlgCreatePeer) {
  // Check existing peers
  CPeerController::Instance()->refresh();
  // Bridge interfaces
  QStringList bridges = CPeerController::Instance()->get_bridgedifs();
  // ui
  ui->setupUi(this);
  ui->le_disk->setText("100");
  ui->cmb_bridge->addItems(bridges);
  ui->lbl_provider->setText(VagrantProvider::Instance()->CurrentStr());
  hide_err_labels();
  this->adjustSize();
  // slots
  connect(ui->btn_cancel, &QPushButton::clicked, [this]() { this->close(); });
  connect(ui->btn_create, &QPushButton::clicked, this,
          &DlgCreatePeer::create_button_pressed);
  // requirements
  requirement virtualbox(
      tr("VirtualBox is not ready"), tr("Checking VirtualBox..."),
      tr("VirtualBox is not ready. You should install or update it from "
         "Components"),
      DlgNotification::N_ABOUT, []() {
        return !CHubComponentsUpdater::Instance()->is_update_available(
            IUpdaterComponent::ORACLE_VIRTUALBOX);
      });

  // check license_vmware
  requirement vagrant_vmware_license(
        tr("Vagrant VMWare Desktop provider license is not installed"),
        tr("Checking Vagrant VMware License..."),
        tr("You do not have a <b>license</b> to use <b>Vagrant VMWare Desktop provider.</b> "
           "Please <a href=\"https://www.vagrantup.com/vmware/index.html\">visit</a> to purchase "
           "a license. Once you purchase a license, you can install it "
           "using <b>vagrant plugin license</b> "),
      DlgNotification::N_NO_ACTION, []() {
        return CCommons::IsVagrantVMwareLicenseInstalled();
      });

  // VMware Hypervisor
  requirement vmware(
        tr("VMware is not ready"), tr("Checking VMware..."),
        tr("VMware is not ready. You should install it from "
           "Components"),
        DlgNotification::N_ABOUT, []() {
          return !CHubComponentsUpdater::Instance()->is_update_available(
                IUpdaterComponent::VMWARE);
        });

  // Hyper-V Hypervisor
  requirement hyperv(
        tr("Hyper-V is not ready"), tr("Checking Hyper-V..."),
        tr("Hyper-V is not ready. You should install it from "
           "Components"),
        DlgNotification::N_ABOUT, []() {
          return !CHubComponentsUpdater::Instance()->is_update_available(
                IUpdaterComponent::HYPERV);
        });

  // KVM Hypervisor
  requirement kvm(
        tr("KVM is not ready"), tr("Checking KVM..."),
        tr("KVM is not ready. You should install it from "
           "Components"),
        DlgNotification::N_ABOUT, []() {
          return !CHubComponentsUpdater::Instance()->is_update_available(
                IUpdaterComponent::KVM);
        });

  // Vagrant VMware Utility
  requirement vagrant_vmware_utility(
        tr("Vagrant VMware Utility is not ready"), tr("Checking Vagrant VMware Utility..."),
        tr("Vagrant VMware Utility is not ready. You should install it from "
           "Components"),
        DlgNotification::N_ABOUT, []() {
          return !is_update_available_vmware_utility_th();
        });

  requirement vagrant(
      tr("Vagrant is not ready"), tr("Checking Vagrant..."),
      tr("Vagrant is not ready. You should install or update it from "
         "Components"),
      DlgNotification::N_ABOUT, []() {
        return !CHubComponentsUpdater::Instance()->is_update_available(
            IUpdaterComponent::VAGRANT);
      });
  requirement subutai_plugin(
      tr("Subutai plugin is not ready"), tr("Checking Subutai plugin..."),
      tr("Unable to run the Vagrant Subutai plugin. Make sure that you have it "
         "installed or updated successfully by going to the menu > "
         "Components."),
      DlgNotification::N_ABOUT, []() {
        return !CHubComponentsUpdater::Instance()->is_update_available(
            IUpdaterComponent::VAGRANT_SUBUTAI);
      });
  // For Parallels
  requirement parallels_provider(
      tr("Parallels provider is not ready"), tr("Checking Parallels provider..."),
      tr("Unable to run the Vagrant Parallels provider, Make sure that you have it "
         "installed or updated successfully by going to the menu > "
         "Components."),
      DlgNotification::N_ABOUT, []() {
        return !CHubComponentsUpdater::Instance()->is_update_available(
              IUpdaterComponent::VAGRANT_PARALLELS);
       });
  // For Libvirt
  requirement libvirt_provider(
        tr("Vagrant Libvirt provider is not ready"), tr("Checking Libvirt provider..."),
        tr("Unable to run the Vagrant Libvirt provider, Make sure that you have it "
           "installed or updated successfully by going to the menu > "
           "Components."),
        DlgNotification::N_ABOUT, []() {
          return !CHubComponentsUpdater::Instance()->is_update_available(
                IUpdaterComponent::VAGRANT_LIBVIRT);
         });
  // For VMware. We use vagrant-vmware-desktop provider.
  // Which works both VMware Fusion and Workstation.
  requirement vmware_provider(
        tr("Vagrant VMware provider is not ready"), tr("Checking VMware provider..."),
        tr("Unable to run the Vagrant VMware provider, Make sure that you have it "
           "installed or updated successfully by going to the menu > "
           "Components."),
        DlgNotification::N_ABOUT, []() {
          return !CHubComponentsUpdater::Instance()->is_update_available(
                IUpdaterComponent::VAGRANT_VMWARE_DESKTOP);
         });

  requirement vbguest_plugin(
      tr("VirtualBox plugin is not ready"), tr("Checking VirtualBox plugin..."),
      tr("Vagrant VBGuest plugin is not ready. You should install or update "
         "it from Components"),
      DlgNotification::N_ABOUT, []() {
        return !CHubComponentsUpdater::Instance()->is_update_available(
            IUpdaterComponent::VAGRANT_VBGUEST);
      });
  requirement subutai_box(
      tr("Subutai box is not ready"), tr("Checking Subutai box..."),
      tr("Vagrant Subutai box is not ready. You should install or update it "
         "from Components"),
      DlgNotification::N_ABOUT, []() {
        return !CHubComponentsUpdater::Instance()->is_update_available(
            IUpdaterComponent::SUBUTAI_BOX);
      });

  requirement vms_path(
      tr("VMs storage path is not correct"), tr("Checking VMs storage path..."),
      tr("Unable to find virtual machines' storage location. "
         "Make sure that VMs storage path is set correctly in Settings."),
      DlgNotification::N_SETTINGS, []() {
        QDir vms_dir(CSettingsManager::Instance().vm_storage());
        return vms_dir.exists();
      });

  // Default requrements
  m_requirements_ls = std::vector<requirement>{
      vagrant, subutai_plugin, vms_path};

  // add provider requirements by hypervisor
  switch(VagrantProvider::Instance()->CurrentProvider()) {
  case VagrantProvider::VIRTUALBOX:
    m_requirements_ls.push_back(virtualbox);
    m_requirements_ls.push_back(vbguest_plugin);
    break;
  case VagrantProvider::PARALLELS:
    m_requirements_ls.push_back(parallels_provider);
    break;
  case VagrantProvider::VMWARE_DESKTOP:
    m_requirements_ls.push_back(vmware_provider);
    m_requirements_ls.push_back(vagrant_vmware_license);
    m_requirements_ls.push_back(vmware);
    m_requirements_ls.push_back(vagrant_vmware_utility);


    ui->lbl_bridge->hide();
    ui->cmb_bridge->hide();

    break;
  case VagrantProvider::LIBVIRT:
    m_requirements_ls.push_back(libvirt_provider);
    m_requirements_ls.push_back(kvm);

    if (bridges.size() == 0) {
      ui->lbl_bridge->hide();
      ui->cmb_bridge->hide();
    }

    break;
  case VagrantProvider::HYPERV:
    m_requirements_ls.push_back(hyperv);

    if (bridges.size() <= 1) {
      ui->lbl_bridge->hide();
      ui->cmb_bridge->hide();
    }
    break;
  default:
    break;
  }

  // At the and check Vagrant Subutai Box
  m_requirements_ls.push_back(subutai_box);

  // format
  ui->le_ram->setValidator(new QIntValidator(1, 100000, this));
  ui->le_disk->setValidator(new QIntValidator(1, 100000, this));
  ui->le_pass->setEchoMode(QLineEdit::Password);
  ui->le_pass_confirm->setEchoMode(QLineEdit::Password);
  ui->le_name->setMaxLength(20);
  ui->pb_peer->setMaximum((int)m_requirements_ls.size() + 1);
  m_invalid_chars.setPattern("\\W");
  static QIcon show_password_icon(":/hub/show-password.png");
  static QIcon hide_password_icon(":/hub/hide-password.png");

  // Password
  this->m_show_password_action =
      ui->le_pass->addAction(show_password_icon, QLineEdit::TrailingPosition);
  // Password Confirm
  this->m_show_confirm_password_action = ui->le_pass_confirm->addAction(
      show_password_icon, QLineEdit::TrailingPosition);

  // QLineEdit password show
  connect(this->m_show_password_action, &QAction::triggered, [this]() {
    this->m_password_state ^= 1;
    this->m_show_password_action->setIcon(
          this->m_password_state ? hide_password_icon : show_password_icon);
    ui->le_pass->setEchoMode(this->m_password_state ? QLineEdit::Normal
                                                    : QLineEdit::Password);
  });

  // QLineEdit confirm password
  connect(this->m_show_confirm_password_action, &QAction::triggered, [this]() {
    this->m_password_confirm_state ^= 1;
    this->m_show_confirm_password_action->setIcon(
          this->m_password_confirm_state ? hide_password_icon : show_password_icon);
    ui->le_pass_confirm->setEchoMode(this->m_password_confirm_state
                                         ? QLineEdit::Normal
                                         : QLineEdit::Password);
  });

  // add num cpu to combobox item
  for(unsigned int i = 0; i < Environment::Instance()->numCpu(); i++) {
    ui->cmb_cpu->addItem(QString::number(i+1));
  }
}

DlgCreatePeer::~DlgCreatePeer() { delete ui; }

DlgCreatePeer::pass_err DlgCreatePeer::check_pass(QString pass) {
  if (pass.isEmpty()) return PASS_EMPTY;  // empty error;
  if (pass.size() < 7)                    // too short
    return PASS_SMALL;
  if (pass.contains(m_invalid_chars))  // should contain chars and digits
    return PASS_INVALID;
  return PASS_FINE;
}

bool DlgCreatePeer::check_configurations() {
  qDebug() << "Check conf RAM: "
           << Environment::Instance()->ramSize();
  QString ram = ui->le_ram->text();
  QString disk = ui->le_disk->text();
  QString password1 = ui->le_pass->text();
  QString password2 = ui->le_pass_confirm->text();
  bool errors_exist = false;
  // password
  pass_err pass_error = check_pass(password1);
  if (pass_error != 3) {
    QString error_message = "";
    switch (pass_error) {
      case PASS_EMPTY:
        error_message = tr("Password cannot be empty.");
        break;
      case PASS_SMALL:
        error_message = tr("Password size should be more than 7.");
        break;
      case PASS_INVALID:
        error_message = tr("Password has invalid symbols.");
        break;
      default:
        error_message =
            tr("An unexpected error has occurred. Please try again later.");
        break;
    }
    ui->lbl_err_pass->setText(error_message);
    ui->lbl_err_pass->setStyleSheet("QLabel {color : red}");
    ui->lbl_err_pass->show();
    errors_exist = true;
  } else if (password1 != password2) {
    ui->lbl_err_pass->setText(
        tr("Passwords do not match. Enter both passwords again."));
    ui->lbl_err_pass->setStyleSheet("QLabel {color : red}");
    ui->lbl_err_pass->show();
    errors_exist = true;
  } else
    ui->lbl_err_pass->hide();
  // name
  if (ui->le_name->text().isEmpty()) {
    ui->lbl_err_name->setText(tr("Name of the Peer cannot be empty"));
    ui->lbl_err_name->setStyleSheet("QLabel {color : red}");
    ui->lbl_err_name->show();
    errors_exist = true;
  } else if (ui->le_name->text().contains(m_invalid_chars) ||
             ui->le_name->text().contains("_")) {
    ui->lbl_err_name->setText(tr("Use only letters and numbers."));
    ui->lbl_err_name->setStyleSheet("QLabel {color : red}");
    ui->lbl_err_name->show();
    errors_exist = true;
  } else
    ui->lbl_err_name->hide();
  // ram
  if (ram.toInt() < 2048) {
    ui->lbl_err_ram->setText(tr("Ram cannot be less than 2048 MB."));
    ui->lbl_err_ram->setStyleSheet("QLabel {color : red}");
    ui->lbl_err_ram->show();
    errors_exist = true;
  } else if (ram.toInt() > (int) Environment::Instance()->ramSize()) {   // check max ram
    ui->lbl_err_ram->setText(tr("Ram cannot be more than %1 MB.").arg(Environment::Instance()->ramSize()));
    ui->lbl_err_ram->setStyleSheet("QLabel {color : red}");
    ui->lbl_err_ram->show();
    errors_exist = true;
  } else
    ui->lbl_err_ram->hide();

  // disk
  if (disk.toInt() < 40) {
    ui->lbl_err_disk->setText(tr("Disk cannot be less than 40 GB."));
    ui->lbl_err_disk->setStyleSheet("QLabel {color : red}");
    ui->lbl_err_disk->setToolTip(tr("40 GB is the minimum"
                                    "required disk size for the peer"));
    ui->lbl_err_disk->show();
    errors_exist = true;
  } else if (disk.toInt() > 2048) { // disk max size 2048 GB = 2 TB
    ui->lbl_err_disk->setText(tr("Disk cannot be more than 2048 GB."));
    ui->lbl_err_disk->setStyleSheet("QLabel {color : red}");
    ui->lbl_err_disk->setToolTip(tr("2048 GB is a "
                                    "maximum disk size in VirtualBox"));
    ui->lbl_err_disk->show();
    errors_exist = true;
  } else if (disk.toInt() > int(Environment::Instance()->diskSize())) {
    ui->lbl_err_disk->setText(tr("Disk cannot be more than %1 GB.").arg(Environment::Instance()->diskSize()));
    ui->lbl_err_disk->setStyleSheet("QLabel {color : red}");
    ui->lbl_err_disk->setToolTip(tr("Disk size of peer can't be more than\n"
                                    "your available disk size on your machine."));
    ui->lbl_err_disk->show();
    errors_exist = true;
  } else {
    ui->lbl_err_disk->hide();
  }


  return errors_exist;
}

int DlgCreatePeer::reserve_new_port() {
  // Use minimum not-used port
  std::vector<int> used_ports;
  used_ports.emplace_back(9998);
  int port = 9999;

  system_call_res_t rs = CPeerController::Instance()->get_global_status();
  if (rs.res == SCWE_SUCCESS && rs.exit_code == 0 && !rs.out.isEmpty()) {
    QStringList cr = CPeerController::Instance()->get_global_status().out;
    for (QString dir: cr) {
      QString cur_port_str = CSystemCallWrapper::vagrant_port(dir);
      cur_port_str.remove(QRegularExpression("[^0-9]"));
      int cur_port = cur_port_str.toInt();
      if (cur_port < 9999) {
        continue;
      }
      used_ports.emplace_back(cur_port);
    }
  }
  // We will store in TrayControlWindow::Instance()->reserved_ports
  // ports that were used to create peer, but not reserved by vagrant yet.
  for (int i: TrayControlWindow::Instance()->reserved_ports) {
    used_ports.emplace_back(i);
  }
  sort(used_ports.begin(), used_ports.end());
   for (std::vector<int>::size_type i = 0; i < used_ports.size(); i++) {
    // if this is last port number in our used_port or
    // the next one is greater than current + 1,
    // current + 1 will be the minimum excluded number, hence it will be free.
    if (i + 1 == used_ports.size() || used_ports[i] + 1 < used_ports[i + 1]) {
      port = used_ports[i] + 1;
      break;
    }
  }
   return port;
}

void DlgCreatePeer::create_button_pressed() {
  // Check Vagrant command available:
  if (VagrantProvider::Instance()->CurrentProvider() != VagrantProvider::VMWARE_DESKTOP) {
    if (!CCommons::IsVagrantVMwareLicenseInstalled()) {
      QMessageBox* msg_box =
         new QMessageBox(QMessageBox::Question, tr("Info"),
                         tr("Vagrant VMware Desktop provider <b>license not installed.</b> "
                            "In order to create peer, uninstall Vagrant VMWare Desktop Provider.<br/>"
                            "Do you want to proceed?"),
                         QMessageBox::Yes | QMessageBox::No);
      connect(msg_box, &QMessageBox::finished, msg_box,
              &QMessageBox::deleteLater);

      if (msg_box->exec() == QMessageBox::Yes) {
          update_system::CHubComponentsUpdater::Instance()->uninstall(IUpdaterComponent::VAGRANT_VMWARE_DESKTOP);
      }
    }
  }

  QFuture<bool> future_check_configurations = QtConcurrent::run([this]() {
    return check_configurations();
  });
  future_check_configurations.waitForFinished();

  if (future_check_configurations.result()) return;

  QString dir = create_dir("subutai-peer_" + ui->le_name->text());
  if (dir.isEmpty()) {
    ui->lbl_err_name->setText(tr("Name already exists"));
    ui->lbl_err_name->setStyleSheet("QLabel {color : red}");
    ui->lbl_err_name->show();
    return;
  }
  hide_err_labels();
  set_enabled_buttons(false);

  QFuture<bool> future_check_machine = QtConcurrent::run([this]() {
    return check_machine();
  });
  future_check_machine.waitForFinished();

  if (!future_check_machine.result()) {
    ui->btn_create->setEnabled(true);
    ui->lbl_err_os->setStyleSheet("QLabel {color : red}");
    ui->pb_peer->setValue(0);
    ui->pb_peer->setEnabled(false);
    set_enabled_buttons(true);
    QDir directory_delete(dir);
    directory_delete.removeRecursively();
    return;
  }

  QFuture<int> future_reserve_new_port = QtConcurrent::run([this]() {
    return reserve_new_port();
  });
  future_reserve_new_port.waitForFinished();

  int port = future_reserve_new_port.result();

  qDebug() << "port: " << port;
  //reserve this port until vagrant gets it. Timeout is 2 minutes.
  TrayControlWindow::Instance()->reserved_ports.insert(port);
  QTimer::singleShot(120 * 1000, [port]() {
    TrayControlWindow::Instance()->reserved_ports.erase(port);
  });

  ui->lbl_err_os->setStyleSheet("QLabel {color : green}");
  ui->lbl_err_os->setText(tr("Initalializing environment..."));
  InitPeer *thread_init = new InitPeer(this);
  thread_init->init(dir, ui->cmb_os->currentText());
  connect(thread_init, &InitPeer::outputReceived,
          [dir, this, port](system_call_wrapper_error_t res) {
            this->init_completed(res, dir, this->ui->le_ram->text(),
                                 this->ui->cmb_cpu->currentText(),
                                 this->ui->le_disk->text(), port);
          });
  thread_init->startWork();
}

bool DlgCreatePeer::check_machine() {
  ui->lbl_err_os->show();
  ui->pb_peer->setEnabled(true);
  ui->lbl_err_os->setStyleSheet("QLabel {color : green}");
  static size_t i;
  for (i = 0; i < m_requirements_ls.size(); i++) {
    ui->pb_peer->setValue(i + 1);
    ui->lbl_err_os->setText(m_requirements_ls[i].status_label);
    if (!m_requirements_ls[i].checker_function()) {
      ui->lbl_err_os->setText(m_requirements_ls[i].error_label);
      CNotificationObserver::Error(m_requirements_ls[i].error_notification,
                                   m_requirements_ls[i].notification_type);
      break;
    }
  }

  return i == m_requirements_ls.size();
}

void DlgCreatePeer::set_enabled_buttons(bool state) {
  ui->le_disk->setEnabled(state);
  ui->le_name->setEnabled(state);
  ui->le_pass->setEnabled(state);
  ui->le_pass_confirm->setEnabled(state);
  ui->le_ram->setEnabled(state);
  ui->btn_create->setEnabled(state);
  ui->btn_cancel->setEnabled(state);
  ui->cmb_bridge->setEnabled(state);
  ui->cmb_cpu->setEnabled(state);
  ui->cmb_os->setEnabled(state);
}

void DlgCreatePeer::hide_err_labels() {
  ui->lbl_err_cpu->hide();
  ui->lbl_err_disk->hide();
  ui->lbl_err_ifs->hide();
  ui->lbl_err_name->hide();
  ui->lbl_err_pass->hide();
  ui->lbl_err_ram->hide();
  ui->lbl_err_os->hide();
}

// Create peer directory in home path on Virtualbox
QString DlgCreatePeer::virtualbox_dir(const QString &name) {
  QString new_dir = "";
  QDir current_local_dir = VagrantProvider::Instance()->BasePeerDir();

  if (!current_local_dir.mkdir(name)) return new_dir;
  current_local_dir.cd(name);

  return current_local_dir.absolutePath();
}

// Create peer directory by VM storage path on VMware, Hyper-V
QString DlgCreatePeer::vmware_dir(const QString &peer_folder) {

  // 1. create "peer" folder.
  QString empty;
  QDir peer_dir = VagrantProvider::Instance()->BasePeerDir();

  if (!peer_dir.mkdir(peer_folder)) {
    return empty;
  }
  peer_dir.cd(peer_folder);

  return peer_dir.absolutePath();
}

// for peers, empty if that peer dir exists
QString DlgCreatePeer::create_dir(const QString &peer_folder) {
  switch (VagrantProvider::Instance()->CurrentProvider()) {
  case VagrantProvider::VIRTUALBOX:
    return virtualbox_dir(peer_folder);
  case VagrantProvider::VMWARE_DESKTOP:
    return vmware_dir(peer_folder);
  case VagrantProvider::HYPERV:
    return vmware_dir(peer_folder);
  default:
    return virtualbox_dir(peer_folder);
  }
}

void DlgCreatePeer::init_completed(system_call_wrapper_error_t res, QString dir_peer,
                                   QString ram, QString cpu, QString disk, int port) {
  if (res != SCWE_SUCCESS) { 
    CNotificationObserver::Instance()->Error(
        tr("Coudn't create peer, sorry. Check if all software is installed "
           "correctly"),
        DlgNotification::N_NO_ACTION);

    ui->btn_create->setEnabled(true);
    set_enabled_buttons(true);
    ui->pb_peer->setValue(0);
    ui->pb_peer->setEnabled(false);
    ui->lbl_err_os->setText(tr("Failed to initialize environment"));
    ui->lbl_err_name->setStyleSheet("QLabel {color : red}");
    QDir dir(dir_peer);
    if (dir.exists()) {
      dir.removeRecursively();
    }

    return;
  }
  CNotificationObserver::Instance()->Info(
      tr("Initialization is completed. Installing peer... Don't close terminal "
         "until installation is finished."),
      DlgNotification::N_NO_ACTION);
  QString filename = QString("%1/vagrant-subutai.yml").arg(dir_peer);
  QFile file(filename);
  // write config file
  if (file.open(QIODevice::ReadWrite)) {
    QTextStream stream(&file);
    stream << "DESIRED_CONSOLE_PORT : " << port << endl;
    stream << "SUBUTAI_RAM : " << ram << endl;
    stream << "SUBUTAI_CPU : " << cpu << endl;
    QString branch = current_branch_name_with_changes();
    if (branch == "production")
      stream << "SUBUTAI_ENV : "
             << "prod" << endl;
    else if (branch == "development")
      stream << "SUBUTAI_ENV : "
             << "dev" << endl;
    else
      stream << "SUBUTAI_ENV : "
             << "master" << endl;

    stream << "DISK_SIZE : " << disk << endl;

    if (!this->ui->cmb_bridge->currentText().isEmpty()) {
      stream << "BRIDGE : "
             << QString("\"%1\"").arg(this->ui->cmb_bridge->currentText())
             << endl;
    }

    switch (VagrantProvider::Instance()->CurrentProvider()) {
    case VagrantProvider::VIRTUALBOX:
      stream << "SUBUTAI_DISK_PATH : "
             << QString("\"%1\"")
                .arg(QDir::fromNativeSeparators(CSystemCallWrapper::get_virtualbox_vm_storage()));
      break;
    default:
      break;
    }
  }
  file.close();
  // write provision step file
  QDir pr_dir;
  pr_dir.mkdir(dir_peer + QDir::separator() + ".vagrant");
  QString p_name = dir_peer + QDir::separator() + ".vagrant" + QDir::separator() +
                   "provision_step";
  QFile p_file(p_name);
  if (p_file.open(QIODevice::ReadWrite)) {
    QTextStream stream(&p_file);
    stream << "0" << endl;
  }
  p_file.close();
  QString vagrant_up_string = QString("up --provider %1").arg(
        VagrantProvider::Instance()->CurrentVal());
  QString peer_name = ui->le_name->text(), peer_pass = ui->le_pass->text();
  CSettingsManager::Instance().set_peer_pass(peer_name, peer_pass);

  QFuture<system_call_wrapper_error_t> future_vagrant_command_terminal =
        QtConcurrent::run(CSystemCallWrapper::vagrant_command_terminal, dir_peer,
                          vagrant_up_string, ui->le_name->text());
  future_vagrant_command_terminal.waitForFinished();
  res = future_vagrant_command_terminal.result();

  if (res != SCWE_SUCCESS) {
    CNotificationObserver::Instance()->Error("Coudn't start  peer, sorry",
                                             DlgNotification::N_NO_ACTION);
    set_enabled_buttons(true);
  } else
    this->close();
}
