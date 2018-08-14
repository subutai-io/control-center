#include <QPixmap>
#include <QThread>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrent>

#include "Commons.h"
#include "DlgAbout.h"
#include "NotificationObserver.h"
#include "OsBranchConsts.h"
#include "SettingsManager.h"
#include "SystemCallWrapper.h"
#include "ui_DlgAbout.h"
#include "updater/HubComponentsUpdater.h"

using namespace update_system;

QString get_p2p_version() {
  QString p2p_version = "";
  CSystemCallWrapper::p2p_version(p2p_version);
  return p2p_version;
}

QString get_x2go_version() {
  QString x2go_version = "";
  CSystemCallWrapper::x2go_version(x2go_version);
  return x2go_version;
}

QString get_vagrant_version() {
  QString vagrant_version = "";
  CSystemCallWrapper::vagrant_version(vagrant_version);
  return vagrant_version;
}

QString get_oracle_virtualbox_version() {
  QString version = "";
  CSystemCallWrapper::oracle_virtualbox_version(version);
  return version;
}

QString get_chrome_version() {
  QString version = "";
  CSystemCallWrapper::chrome_version(version);
  return version;
}

QString get_firefox_version() {
  QString version = "";
  CSystemCallWrapper::firefox_version(version);
  return version;
}

QString get_safari_version() {
  QString version = "";
  CSystemCallWrapper::safari_version(version);
  return version;
}

QString get_e2e_version() {
  QString version = "";
  CSystemCallWrapper::subutai_e2e_version(version);
  return version;
}

QString get_vagrant_provider_version() {
  QString version = "";

  CSystemCallWrapper::vagrant_plugin_version(version, VagrantProvider::Instance()->CurrentName());
  return version;
}

QString get_vagrant_subutai_version() {
  QString version = "";
  static const QString subutai_plugin = "vagrant-subutai";

  CSystemCallWrapper::vagrant_plugin_version(version, subutai_plugin);
  return version;
}

QString get_vagrant_vbguest_version() {
  QString version = "";
  static const QString vbguest_plugin = "vagrant-vbguest";

  CSystemCallWrapper::vagrant_plugin_version(version, vbguest_plugin);
  return version;
}

QString get_subutai_box_version() {
  QString version = "";
  QString provider = VagrantProvider::Instance()->CurrentVal(), box = subutai_box_name();
  CSystemCallWrapper::vagrant_latest_box_version(box, provider, version);
  return version;
}

QString get_hypervisor_vmware_version() {
  QString version = "";
  CSystemCallWrapper::vmware_version(version);

  return version;
}

QString get_vagrant_vmware_utility_version() {
  QString version = "";
  CSystemCallWrapper::vmware_utility_version(version);

  return version;
}

QString get_edge_version() {
  QString version = "";
  CSystemCallWrapper::edge_version(version);

  return version;
}

QString get_xquartz_version() {
  QString version = "";
  CSystemCallWrapper::xquartz_version(version);

  return version;
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::set_visible_libvirt(bool value) {
  // Vagrant libvirt provider vagrant-libvirt
  ui->lbl_provider_libvirt->setVisible(value);
  ui->lbl_provider_libvirt_icon->setVisible(value);
  ui->lbl_provider_libvirt_version->setVisible(value);
  ui->lbl_spacer_vagrant_libvirt->setVisible(value);
  ui->btn_provider_libvirt_update->setVisible(value);
  ui->cb_provider_libvirt->setVisible(value);
  ui->pb_provider_libvirt->setVisible(value);
  ui->hl_vagrant_libvirt->setEnabled(value);
  this->adjustSize();
}

void DlgAbout::set_visible_parallels(bool value) {
  // Vagrant parallels provider vagrant-parallels
  ui->lbl_provider_parallels->setVisible(value);
  ui->lbl_provider_parallels_icon->setVisible(value);
  ui->lbl_provider_parallels_version->setVisible(value);
  ui->lbl_spacer_provider_parallels->setVisible(value);
  ui->btn_provider_parallels_update->setVisible(value);
  ui->cb_provider_parallels->setVisible(value);
  ui->pb_provider_parallels->setVisible(value);
  ui->hl_vagrant_parallels->setEnabled(value);
  this->adjustSize();
}

void DlgAbout::set_visible_virtualbox(bool value) {
  // virtualbox hypervisor
  ui->lbl_oracle_virtualbox_version_val->setVisible(value);
  ui->lbl_spacer_vbox->setVisible(value);
  ui->lbl_vbox_info_icon->setVisible(value);
  ui->lbl_vbox_version->setVisible(value);
  ui->btn_oracle_virtualbox_update->setVisible(value);
  ui->pb_oracle_virtualbox->setVisible(value);
  ui->cb_oracle_virtualbox->setVisible(value);

  // virtualbox plugin vagrant-vbguest
  ui->lbl_vbguest_plugin_version->setVisible(value);
  ui->lbl_spacer_vbguest_plugin->setVisible(value);
  ui->lbl_vbguest_plugin_info_icon->setVisible(value);
  ui->lbl_vbguest_plugin_version_val->setVisible(value);
  ui->pb_vbguest_plugin->setVisible(value);
  ui->btn_vbguest_plugin_update->setVisible(value);
  ui->cb_vagrant_vbguest_plugin->setVisible(value);
  this->adjustSize();
}

void DlgAbout::set_visible_vmware(bool value) {
  // Vagrant provider vagrant-vmware-desktop
  ui->lbl_provider_vmware->setVisible(value);
  ui->lbl_provider_vmware_icon->setVisible(value);
  ui->lbl_provider_vmware_version->setVisible(value);
  ui->lbl_spacer_provider_vmware->setVisible(value);
  ui->btn_provider_vmware_update->setVisible(value);
  ui->cb_provider_vmware->setVisible(value);
  ui->pb_provider_vmware->setVisible(value);
  ui->hl_vagrant_vmware->setEnabled(value);

  // Hypervisor VMware
  ui->lbl_hypervisor_vmware->setVisible(value);
  ui->lbl_hypervisor_vmware_icon->setVisible(value);
  ui->lbl_hypervisor_vmware_version->setVisible(value);
  ui->lbl_spacer_hypervisor_vmware->setVisible(value);
  ui->btn_hypervisor_vmware_update->setVisible(value);
  ui->cb_hypervisor_vmware->setVisible(value);
  ui->pb_hypervisor_vmware->setVisible(value);
  ui->hl_hypervisor_vmware->setEnabled(value);

  // Vagrant VMware Utility
  ui->lbl_provider_vmware_utility->setVisible(value);
  ui->lbl_provider_vmware_utility_icon->setVisible(value);
  ui->lbl_provider_vmware_utility_version->setVisible(value);
  ui->lbl_spacer_vagrant_vmware_utility->setVisible(value);
  ui->btn_provider_vmware_utility_update->setVisible(value);
  ui->cb_provider_vmware_utility->setVisible(value);
  ui->pb_provider_vmare_utility->setVisible(value);
  ui->hl_vagrant_vmware_utility->setEnabled(value);
  this->adjustSize();
}

void DlgAbout::set_hidden_providers() {
  // Parallels
  set_visible_parallels(false);
  // Libvirt
  set_visible_libvirt(false);
  // VMware
  set_visible_vmware(false);
  // Virtulbox
  set_visible_virtualbox(false);

  switch (VagrantProvider::Instance()->CurrentProvider()) {
  case VagrantProvider::VIRTUALBOX:
    set_visible_virtualbox(true);

    this->m_dct_fpb[IUpdaterComponent::ORACLE_VIRTUALBOX] = {
      ui->lbl_oracle_virtualbox_version_val, ui->pb_oracle_virtualbox,
      ui->cb_oracle_virtualbox, ui->btn_oracle_virtualbox_update,
      get_oracle_virtualbox_version};

    this->m_dct_fpb[IUpdaterComponent::VAGRANT_VBGUEST] = {
      ui->lbl_vbguest_plugin_version_val, ui->pb_vbguest_plugin,
      ui->cb_vagrant_vbguest_plugin, ui->btn_vbguest_plugin_update,
      get_vagrant_vbguest_version};

    break;
  //case VagrantProvider::PARALLELS:
  //  set_visible_parallels(true);

  //  this->m_dct_fpb[IUpdaterComponent::VAGRANT_PARALLELS] = {
  //    ui->lbl_provider_parallels_version, ui->pb_provider_parallels,
  //    ui->cb_provider_parallels, ui->btn_provider_parallels_update,
  //    get_vagrant_provider_version};

  //  break;
  case VagrantProvider::VMWARE_DESKTOP:
    set_visible_vmware(true);

    this->m_dct_fpb[IUpdaterComponent::VAGRANT_VMWARE_DESKTOP] = {
      ui->lbl_provider_vmware_version, ui->pb_provider_vmware,
      ui->cb_provider_vmware, ui->btn_provider_vmware_update,
      get_vagrant_provider_version
    };

    this->m_dct_fpb[IUpdaterComponent::VMWARE] = {
      ui->lbl_hypervisor_vmware_version, ui->pb_hypervisor_vmware,
      ui->cb_hypervisor_vmware, ui->btn_hypervisor_vmware_update,
      get_hypervisor_vmware_version
    };

    this->m_dct_fpb[IUpdaterComponent::VMWARE_UTILITY] = {
      ui->lbl_provider_vmware_utility_version, ui->pb_provider_vmare_utility,
      ui->cb_provider_vmware_utility, ui->btn_provider_vmware_utility_update,
      get_vagrant_vmware_utility_version
    };

    break;
  //case VagrantProvider::LIBVIRT:
  //  set_visible_libvirt(true);

 //   this->m_dct_fpb[IUpdaterComponent::VAGRANT_LIBVIRT] = {
 //     ui->lbl_provider_libvirt_version, ui->pb_provider_libvirt,
 //     ui->cb_provider_libvirt, ui->btn_provider_libvirt_update,
 //     get_vagrant_provider_version};

    break;
  //case VagrantProvider::HYPERV:
    // do nothing
 //   break;
  default:
    set_visible_virtualbox(true);
    break;
  }

  this->adjustSize();
}

DlgAbout::DlgAbout(QWidget* parent) : QDialog(parent), ui(new Ui::DlgAbout) {
  ui->setupUi(this);
  ui->lbl_tray_version_val->setText(TRAY_VERSION + branch_name_str());
  current_browser = CSettingsManager::Instance().default_browser();

  set_visible_chrome("Chrome" == current_browser);
  set_visible_firefox("Firefox" == current_browser);
  set_visible_edge("Edge" == current_browser);
  set_visible_safari("Safari" == current_browser);
  set_visible_xquartz(OS_MAC == CURRENT_OS);

  QLabel* ilbls[] = {this->ui->lbl_p2p_info_icon,
                     this->ui->lbl_tray_info_icon,
                     this->ui->lbl_x2go_info_icon,
                     this->ui->lbl_vagrant_info_icon,
                     this->ui->lbl_chrome_info_icon,
                     this->ui->lbl_e2e_info_icon,
                     this->ui->lbl_vbox_info_icon,
                     this->ui->lbl_subutai_plugin_info_icon,
                     this->ui->lbl_vbguest_plugin_info_icon,
                     this->ui->lbl_subutai_box_info_icon,
                     this->ui->lbl_firefox_info_icon,
                     this->ui->lbl_edge_info_icon,
                     this->ui->lbl_safari_info_icon,
                     this->ui->lbl_xquartz_info_icon,
                     this->ui->lbl_provider_vmware_icon,
                     this->ui->lbl_provider_vmware_utility_icon,
                     this->ui->lbl_hypervisor_vmware_icon,
                     nullptr};

  static QPixmap info_icon = QPixmap(":/hub/info_icon.png");

  for (QLabel** i = ilbls; *i; ++i) {
    (*i)->setPixmap(info_icon);
    (*i)->setToolTipDuration(1000 * 1000);
    (*i)->setTextFormat(Qt::RichText);
  }

  this->ui->lbl_tray_info_icon->setToolTip(tr(
      "<nobr>Subutai Control Center is a tray application<br>"
      "that is meant to ease bazaar usage."));

  this->ui->lbl_p2p_info_icon->setToolTip(tr(
      "<nobr>Subutai P2P is powerful tool that establishes<br>"
      "connections to peers and environments."));

  this->ui->lbl_vagrant_info_icon->setToolTip(tr(
      "<nobr>Vagrant is a tool for building and<br>"
      "managing virtual machine environments."));

  this->ui->lbl_e2e_info_icon->setToolTip(tr(
      "<nobr>Subutai E2E is an extension for browser which<br>"
      "helps to store and manage PGP-keys."));

  this->ui->lbl_vbox_info_icon->setToolTip(tr(
      "<nobr>Oracle VirtualBox is hypervisor for<br>"
      "managing virtual machine environments"));

  this->ui->lbl_chrome_info_icon->setToolTip(tr(
      "Google Chrome is a web browser used by default."));

  this->ui->lbl_firefox_info_icon->setToolTip(tr(
      "Mozilla Firefox is a web browser used by default."));

  this->ui->lbl_edge_info_icon->setToolTip(tr(
      "Microsoft Edge is web browser used by default."));

  this->ui->lbl_safari_info_icon->setToolTip(tr(
      "Safari is web browser used by default."));

  this->ui->lbl_subutai_box_info_icon->setToolTip(tr(
      "Subutai Box is the resource box for peer creation."));

  this->ui->lbl_subutai_plugin_info_icon->setToolTip(tr(
      "<nobr>The Vagrant Subutai plugin sets up<br>"
      "peer parameters, like disk size and RAM."));

  this->ui->lbl_vbguest_plugin_info_icon->setToolTip(tr(
      "<nobr>The Vagrant VirtualBox plugin sets<br>"
      "VirtualBox as your hypervisor for Vagrant."));

  this->ui->lbl_x2go_info_icon->setToolTip(tr(
      "X2Go client enables remote desktop access."));

  this->ui->lbl_xquartz_info_icon->setToolTip(tr(
      "XQuartz is a tool for X2Go OS X client to use the OS X X11 server"));

  this->ui->lbl_provider_vmware_icon->setToolTip(tr(
      "The Vagrant VMware Desktop provider manage VMware machines."));

  this->ui->lbl_provider_vmware_utility_icon->setToolTip(tr(
      "The Vagrant VMware Utility provides the Vagrant VMware provider plugin"
      " access to various VMware functionalities."));

  this->ui->lbl_hypervisor_vmware_icon->setToolTip(tr(
      "<nobr>VMware is hypervisor for<br>"
      "managing virtual machine environments"));

  QLabel* lbls[] = {this->ui->lbl_chrome_version_val,
                    this->ui->lbl_p2p_version_val,
                    this->ui->lbl_tray_version_val,
                    this->ui->lbl_x2go_version_val,
                    this->ui->lbl_firefox_version_val,
                    this->ui->lbl_chrome_version_val,
                    this->ui->lbl_edge_version_val,
                    this->ui->lbl_safari_version_val,
                    this->ui->lbl_subutai_e2e_val,
                    this->ui->lbl_subutai_plugin_version_val,
                    this->ui->lbl_vbguest_plugin_version_val,
                    this->ui->lbl_subutai_box_version,
                    this->ui->lbl_provider_libvirt_version,
                    this->ui->lbl_provider_parallels_version,
                    this->ui->lbl_provider_vmware_version,
                    this->ui->lbl_provider_vmware_utility_version,
                    this->ui->lbl_xquartz_version,
                    nullptr};

  for (QLabel** i = lbls; *i; ++i) {
    (*i)->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    (*i)->setWordWrap(true);
  }

  // connect
  connect(ui->btn_p2p_update, &QPushButton::released, this,
          &DlgAbout::btn_p2p_update_released);
  connect(ui->btn_tray_update, &QPushButton::released, this,
          &DlgAbout::btn_tray_update_released);
  connect(ui->btn_recheck, &QPushButton::released, this,
          &DlgAbout::btn_recheck_released);
  connect(ui->btn_x2go_update, &QPushButton::released, this,
          &DlgAbout::btn_x2go_update_released);
  connect(ui->btn_vagrant_update, &QPushButton::released, this,
          &DlgAbout::btn_vagrant_update_released);
  connect(ui->btn_oracle_virtualbox_update, &QPushButton::released, this,
          &DlgAbout::btn_oracle_virtualbox_update_released);
  connect(ui->btn_chrome, &QPushButton::released, this,
          &DlgAbout::btn_chrome_update_release);
  connect(ui->btn_firefox, &QPushButton::released, this,
          &DlgAbout::btn_firefox_update_released);
  connect(ui->btn_subutai_e2e, &QPushButton::released, this,
          &DlgAbout::btn_e2e_update_released);
  connect(ui->btn_subutai_plugin_update, &QPushButton::released, this,
          &DlgAbout::btn_subutai_plugin_update_released);
  connect(ui->btn_vbguest_plugin_update, &QPushButton::released, this,
          &DlgAbout::btn_vbguest_plugin_update_released);
  connect(ui->btn_subutai_box, &QPushButton::released, this,
          &DlgAbout::btn_subutai_box_update_released);
  connect(ui->btn_xquartz_update, &QPushButton::released, this,
          &DlgAbout::btn_xquartz_update_released);
  connect(ui->btn_uninstall_components, &QPushButton::released, this,
          &DlgAbout::btn_uninstall_components);
  connect(ui->btn_close, &QPushButton::released, this,
          &DlgAbout::btn_close_released);
  connect(ui->btn_provider_libvirt_update, &QPushButton::released, this,
          &DlgAbout::btn_provider_libvirt_updates_released);
  connect(ui->btn_provider_parallels_update, &QPushButton::released, this,
          &DlgAbout::btn_provider_parallels_update_released);
  connect(ui->btn_provider_vmware_update, &QPushButton::released, this,
          &DlgAbout::btn_provider_vmware_update_released);
  connect(ui->btn_hypervisor_vmware_update, &QPushButton::released, this,
          &DlgAbout::btn_hypervisor_vmware_update_released);
  connect(ui->btn_provider_vmware_utility_update, &QPushButton::released, this,
          &DlgAbout::btn_vagrant_vmware_utility_update_released);

  connect(CHubComponentsUpdater::Instance(),
          &CHubComponentsUpdater::download_file_progress, this,
          &DlgAbout::download_progress);
  connect(CHubComponentsUpdater::Instance(),
          &CHubComponentsUpdater::update_available, this,
          &DlgAbout::update_available);
  connect(CHubComponentsUpdater::Instance(),
          &CHubComponentsUpdater::updating_finished, this,
          &DlgAbout::update_finished);
  connect(CHubComponentsUpdater::Instance(),
          &CHubComponentsUpdater::installing_finished, this,
          &DlgAbout::install_finished);
  connect(CHubComponentsUpdater::Instance(),
          &CHubComponentsUpdater::uninstalling_finished, this,
          &DlgAbout::uninstall_finished);

  m_dct_fpb[IUpdaterComponent::P2P] = {
    ui->lbl_p2p_version_val, ui->pb_p2p,
    ui->cb_p2p, ui->btn_p2p_update,
    get_p2p_version
  };

  m_dct_fpb[IUpdaterComponent::TRAY] = {
    ui->lbl_tray_version_val, ui->pb_tray,
    NULL, ui->btn_tray_update, NULL
  };

  m_dct_fpb[IUpdaterComponent::X2GO] = {
    ui->lbl_x2go_version_val, ui->pb_x2go,
    ui->cb_x2goclient, ui->btn_x2go_update,
    get_x2go_version
  };

  m_dct_fpb[IUpdaterComponent::VAGRANT] = {
    ui->lbl_vagrant_version_val, ui->pb_vagrant,
    ui->cb_vagrant, ui->btn_vagrant_update,
    get_vagrant_version
  };

  m_dct_fpb[IUpdaterComponent::ORACLE_VIRTUALBOX] = {
    ui->lbl_oracle_virtualbox_version_val, ui->pb_oracle_virtualbox,
    ui->cb_oracle_virtualbox, ui->btn_oracle_virtualbox_update,
    get_oracle_virtualbox_version
  };

  m_dct_fpb[IUpdaterComponent::CHROME] = {
    ui->lbl_chrome_version_val, ui->pb_chrome,
    ui->cb_chrome, ui->btn_chrome,
    get_chrome_version
  };

  m_dct_fpb[IUpdaterComponent::FIREFOX] = {
    ui->lbl_firefox_version_val, ui->pb_firefox,
    ui->cb_firefox, ui->btn_firefox,
    get_firefox_version
  };

  m_dct_fpb[IUpdaterComponent::E2E] = {
    ui->lbl_subutai_e2e_val, ui->pb_e2e,
    ui->cb_subutai_e2e, ui->btn_subutai_e2e,
    get_e2e_version
  };

  m_dct_fpb[IUpdaterComponent::VAGRANT_SUBUTAI] = {
    ui->lbl_subutai_plugin_version_val, ui->pb_subutai_plugin,
    ui->cb_vagrant_subtuai_plugin, ui->btn_subutai_plugin_update,
    get_vagrant_subutai_version
  };

  m_dct_fpb[IUpdaterComponent::VAGRANT_VBGUEST] = {
    ui->lbl_vbguest_plugin_version_val, ui->pb_vbguest_plugin,
    ui->cb_vagrant_vbguest_plugin, ui->btn_vbguest_plugin_update,
    get_vagrant_vbguest_version
  };

  m_dct_fpb[IUpdaterComponent::SUBUTAI_BOX] = {
    ui->lbl_subutai_box_version, ui->pb_subutai_box,
    ui->cb_vagrant_box, ui->btn_subutai_box,
    get_subutai_box_version
  };

  m_dct_fpb[IUpdaterComponent::VAGRANT_LIBVIRT] = {
    ui->lbl_provider_libvirt_version, ui->pb_provider_libvirt, ui->cb_provider_libvirt,
    ui->btn_provider_libvirt_update, get_vagrant_provider_version
  };

  m_dct_fpb[IUpdaterComponent::VAGRANT_PARALLELS] = {
    ui->lbl_provider_parallels_version, ui->pb_provider_parallels, ui->cb_provider_parallels,
    ui->btn_provider_parallels_update, get_vagrant_provider_version
  };

  m_dct_fpb[IUpdaterComponent::VAGRANT_VMWARE_DESKTOP] = {
    ui->lbl_provider_vmware_version, ui->pb_provider_vmware, ui->cb_provider_vmware,
    ui->btn_provider_vmware_update, get_vagrant_provider_version
  };

  m_dct_fpb[IUpdaterComponent::VMWARE] = {
    ui->lbl_hypervisor_vmware_version, ui->pb_hypervisor_vmware, ui->cb_hypervisor_vmware,
    ui->btn_hypervisor_vmware_update, get_hypervisor_vmware_version
  };

  m_dct_fpb[IUpdaterComponent::VMWARE_UTILITY] = {
    ui->lbl_provider_vmware_utility_version, ui->pb_provider_vmare_utility, ui->cb_provider_vmware_utility,
    ui->btn_provider_vmware_utility_update, get_vagrant_vmware_utility_version
  };

  m_dct_fpb[IUpdaterComponent::XQUARTZ] = {
    ui->lbl_xquartz_version_val, ui->pb_xquartz, ui->cb_xquartz,
    ui->btn_xquartz_update, get_xquartz_version
  };

  // hide providers and add provider to components dictionary
  set_hidden_providers();

  for (auto it = m_dct_fpb.begin(); it != m_dct_fpb.end(); it++) {
    std::pair<quint64, quint64> progress =
        CHubComponentsUpdater::Instance()->get_last_pb_value(it->first);
    if (progress.second == 0) {
      it->second.pb->setValue(0);
      it->second.pb->setMaximum(0);
      it->second.pb->setMinimum(0);
    } else {
      uint value = (progress.first * 100) / progress.second;
      it->second.pb->setValue(value);
    }
  }

  ui->pb_initialization_progress->setMaximum(
      DlgAboutInitializer::COMPONENTS_COUNT);

  // hide checkboxes and pb
  for (const auto& component : m_dct_fpb) {
    if (component.second.cb != nullptr) {
      component.second.cb->setVisible(false);
    }
    set_hidden_pb(component.first);
  }

  ui->gridLayout->setSizeConstraint(QLayout::SetFixedSize);
  ui->gl_components->setSizeConstraint(QLayout::SetFixedSize);
  ui->gridLayout_3->setSizeConstraint(QLayout::SetFixedSize);
  this->setMinimumWidth(600);
  this->setMaximumHeight(450);
  this->adjustSize();

  check_for_versions_and_updates();
}

void DlgAbout::set_visible_chrome(bool value) {
  ui->btn_chrome->setVisible(value);
  ui->lbl_chrome_version->setVisible(value);
  ui->lbl_chrome_info_icon->setVisible(value);
  ui->lbl_chrome_version_val->setVisible(value);
  ui->lbl_spacer_chrome->setVisible(value);
  ui->pb_chrome->setVisible(value);
  ui->cb_chrome->setVisible(value);
  this->adjustSize();
}

void DlgAbout::set_visible_firefox(bool value) {
  ui->btn_firefox->setVisible(value);
  ui->lbl_firefox_version->setVisible(value);
  ui->lbl_firefox_info_icon->setVisible(value);
  ui->lbl_firefox_version_val->setVisible(value);
  ui->lbl_spacer_firefox->setVisible(value);
  ui->pb_firefox->setVisible(value);
  ui->cb_firefox->setVisible(value);
  ui->hl_firefox->setEnabled(value);
  this->adjustSize();
}

void DlgAbout::set_visible_edge(bool value) {
  ui->lbl_edge->setVisible(value);
  ui->lbl_edge_info_icon->setVisible(value);
  ui->lbl_edge_version_val->setVisible(value);
  ui->lbl_spacer_edge->setVisible(value);
  if (value) {
    set_visible_e2e(false);
  }
  this->adjustSize();
}

void DlgAbout::set_visible_safari(bool value) {
  ui->lbl_safari->setVisible(value);
  ui->lbl_safari_info_icon->setVisible(value);
  ui->lbl_safari_version_val->setVisible(value);
  ui->lbl_spacer_safari->setVisible(value);
  this->adjustSize();
}

void DlgAbout::set_visible_e2e(bool value) {
  ui->btn_subutai_e2e->setVisible(value);
  ui->cb_subutai_e2e->setVisible(value);
  ui->lbl_e2e_info_icon->setVisible(value);
  ui->lbl_e2e_version->setVisible(value);
  ui->lbl_spacer_e2e->setVisible(value);
  ui->lbl_subutai_e2e_val->setVisible(value);
  ui->pb_e2e->setVisible(value);
}

void DlgAbout::set_visible_xquartz(bool value) {
  ui->btn_xquartz_update->setVisible(value);
  ui->lbl_xquartz_info_icon->setVisible(value);
  ui->lbl_xquartz_version->setVisible(value);
  ui->lbl_xquartz_version_val->setVisible(value);
  ui->lbl_spacer_xquartz->setVisible(value);;
  ui->pb_xquartz->setVisible(value);
  ui->hl_xquartz->setEnabled(value);
}
DlgAbout::~DlgAbout() { delete ui; }
////////////////////////////////////////////////////////////////////////////

void DlgAbout::check_for_versions_and_updates() {
  ui->btn_recheck->setEnabled(false);
  ui->pb_initialization_progress->setEnabled(true);
  DlgAboutInitializer* di = new DlgAboutInitializer();

  connect(di, &DlgAboutInitializer::finished, this,
          &DlgAbout::initialization_finished);
  connect(di, &DlgAboutInitializer::got_chrome_version, this,
          &DlgAbout::got_chrome_version_sl);
  connect(di, &DlgAboutInitializer::got_firefox_version, this,
          &DlgAbout::got_firefox_version_sl);
  connect(di, &DlgAboutInitializer::got_edge_version, this,
          &DlgAbout::got_edge_version_sl);
  connect(di, &DlgAboutInitializer::got_safari_version, this,
          &DlgAbout::got_safari_version_sl);
  connect(di, &DlgAboutInitializer::got_p2p_version, this,
          &DlgAbout::got_p2p_version_sl);
  connect(di, &DlgAboutInitializer::got_x2go_version, this,
          &DlgAbout::got_x2go_version_sl);
  connect(di, &DlgAboutInitializer::got_vagrant_version, this,
          &DlgAbout::got_vagrant_version_sl);
  connect(di, &DlgAboutInitializer::got_oracle_virtualbox_version, this,
          &DlgAbout::got_oracle_virtualbox_version_sl);
  connect(di, &DlgAboutInitializer::got_e2e_version, this,
          &DlgAbout::got_e2e_version_sl);
  connect(di, &DlgAboutInitializer::got_subutai_plugin_version, this,
          &DlgAbout::got_subutai_plugin_version_sl);
  connect(di, &DlgAboutInitializer::got_vbguest_plugin_version, this,
          &DlgAbout::got_vbguest_plugin_version_sl);
  connect(di, &DlgAboutInitializer::got_subutai_box_version, this,
          &DlgAbout::got_subutai_box_version_sl);
  connect(di, &DlgAboutInitializer::got_provider_version, this,
          &DlgAbout::got_provider_version_sl);
  connect(di, &DlgAboutInitializer::got_hypervisor_vmware_version, this,
          &DlgAbout::got_hypervisor_vmware_version_sl);
  connect(di, &DlgAboutInitializer::got_vagrant_vmware_utility_version, this,
          &DlgAbout::got_vagrant_vmware_utility_version_sl);
  connect(di, &DlgAboutInitializer::got_xquartz_version, this,
          &DlgAbout::got_xquartz_version_sl);
  connect(di, &DlgAboutInitializer::update_available, this,
          &DlgAbout::update_available_sl);
  connect(di, &DlgAboutInitializer::init_progress, this,
          &DlgAbout::init_progress_sl);
  di->startWork();
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::btn_tray_update_released() {
  ui->pb_tray->setHidden(false);
  ui->btn_tray_update->setEnabled(false);
  CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::TRAY);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::btn_p2p_update_released() {
  ui->pb_p2p->setHidden(false);
  ui->btn_p2p_update->setEnabled(false);
  if (ui->lbl_p2p_version_val->text() == "undefined")
    CHubComponentsUpdater::Instance()->install(IUpdaterComponent::P2P);
  else
    CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::P2P);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::btn_x2go_update_released() {
  ui->pb_x2go->setHidden(false);
  ui->btn_x2go_update->setEnabled(false);
  if (ui->lbl_x2go_version_val->text() == "undefined")
    CHubComponentsUpdater::Instance()->install(IUpdaterComponent::X2GO);
  else
    CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::X2GO);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::btn_vagrant_update_released() {
  ui->pb_vagrant->setHidden(false);
  ui->btn_vagrant_update->setEnabled(false);
  if (ui->lbl_vagrant_version_val->text() == "undefined")
    CHubComponentsUpdater::Instance()->install(IUpdaterComponent::VAGRANT);
  else
    CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::VAGRANT);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::btn_oracle_virtualbox_update_released() {
  ui->pb_oracle_virtualbox->setHidden(false);
  ui->btn_oracle_virtualbox_update->setHidden(false);
  ui->btn_oracle_virtualbox_update->setEnabled(false);
  if (ui->lbl_oracle_virtualbox_version_val->text() == "undefined")
    CHubComponentsUpdater::Instance()->install(
        IUpdaterComponent::ORACLE_VIRTUALBOX);
  else
    CHubComponentsUpdater::Instance()->force_update(
        IUpdaterComponent::ORACLE_VIRTUALBOX);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::btn_chrome_update_release() {
  ui->pb_chrome->setHidden(false);
  ui->btn_chrome->setEnabled(false);
  if (ui->lbl_chrome_version_val->text() == "undefined")
    CHubComponentsUpdater::Instance()->install(IUpdaterComponent::CHROME);
  else
    CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::CHROME);
}
////////////////////////////////////////////////////////////////////////////
void DlgAbout::btn_firefox_update_released() {
  ui->pb_firefox->setHidden(false);
  ui->btn_firefox->setEnabled(false);
  if (ui->lbl_firefox_version_val->text() == "undefined")
    CHubComponentsUpdater::Instance()->install(IUpdaterComponent::FIREFOX);
  else
    CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::FIREFOX);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::btn_e2e_update_released() {
  ui->pb_e2e->setHidden(false);
  ui->btn_subutai_e2e->setEnabled(false);
  if (ui->lbl_subutai_e2e_val->text() == "undefined") {
    CHubComponentsUpdater::Instance()->install(IUpdaterComponent::E2E);
  } else
    CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::E2E);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::btn_subutai_plugin_update_released() {
  ui->pb_subutai_plugin->setHidden(false);
  ui->btn_subutai_plugin_update->setEnabled(false);

  if (ui->lbl_subutai_plugin_version_val->text() == "undefined") {
    CHubComponentsUpdater::Instance()->install(
        IUpdaterComponent::VAGRANT_SUBUTAI);
  } else
    CHubComponentsUpdater::Instance()->force_update(
        IUpdaterComponent::VAGRANT_SUBUTAI);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::btn_vbguest_plugin_update_released() {
  ui->pb_vbguest_plugin->setHidden(false);
  ui->btn_vbguest_plugin_update->setEnabled(false);
  if (ui->lbl_vbguest_plugin_version_val->text() == "undefined") {
    CHubComponentsUpdater::Instance()->install(
        IUpdaterComponent::VAGRANT_VBGUEST);
  } else
    CHubComponentsUpdater::Instance()->force_update(
        IUpdaterComponent::VAGRANT_VBGUEST);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::btn_subutai_box_update_released() {
  ui->pb_subutai_box->setHidden(false);
  ui->btn_subutai_box->setEnabled(false);
  if (ui->lbl_subutai_box_version->text() == "undefined") {
    CHubComponentsUpdater::Instance()->install(IUpdaterComponent::SUBUTAI_BOX);
  } else
    CHubComponentsUpdater::Instance()->force_update(
        IUpdaterComponent::SUBUTAI_BOX);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::btn_xquartz_update_released() {
  ui->pb_xquartz->setHidden(false);
  ui->btn_xquartz_update->setEnabled(false);
  if (ui->lbl_xquartz_version_val->text() == "undefined") {
    CHubComponentsUpdater::Instance()->install(IUpdaterComponent::XQUARTZ);
  } else {
    CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::XQUARTZ);
  }
}
////////////////////////////////////////////////////////////////////////////
void DlgAbout::btn_recheck_released() {
  for (auto it = m_dct_fpb.begin(); it != m_dct_fpb.end(); it++) {
    it->second.btn->setEnabled(false);
  }
  check_for_versions_and_updates();
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::btn_close_released() { this->close(); }
////////////////////////////////////////////////////////////////////////////

void DlgAbout::btn_provider_libvirt_updates_released() {
  ui->pb_provider_libvirt->setHidden(false);
  ui->btn_provider_libvirt_update->setEnabled(false);
  if (ui->lbl_provider_libvirt_version->text() == "undefined") {
    CHubComponentsUpdater::Instance()->install(
        IUpdaterComponent::VAGRANT_LIBVIRT);
  } else
    CHubComponentsUpdater::Instance()->force_update(
        IUpdaterComponent::VAGRANT_LIBVIRT);
}

void DlgAbout::btn_provider_parallels_update_released() {
  ui->pb_provider_parallels->setHidden(false);
  ui->btn_provider_parallels_update->setEnabled(false);
  if (ui->lbl_provider_parallels_version->text() == "undefined") {
    CHubComponentsUpdater::Instance()->install(
        IUpdaterComponent::VAGRANT_PARALLELS);
  } else
    CHubComponentsUpdater::Instance()->force_update(
        IUpdaterComponent::VAGRANT_PARALLELS);
}

void DlgAbout::btn_provider_vmware_update_released() {
  ui->pb_provider_vmware->setHidden(false);
  ui->btn_provider_vmware_update->setEnabled(false);
  if (ui->lbl_provider_vmware_version->text() == "undefined") {
    CHubComponentsUpdater::Instance()->install(
        IUpdaterComponent::VAGRANT_VMWARE_DESKTOP);
  } else
    CHubComponentsUpdater::Instance()->force_update(
        IUpdaterComponent::VAGRANT_VMWARE_DESKTOP);
}

void DlgAbout::btn_hypervisor_vmware_update_released() {
  ui->pb_hypervisor_vmware->setHidden(false);
  ui->btn_hypervisor_vmware_update->setHidden(false);
  ui->btn_hypervisor_vmware_update->setEnabled(false);
  if (ui->lbl_hypervisor_vmware_version->text() == "undefined")
    CHubComponentsUpdater::Instance()->install(
        IUpdaterComponent::VMWARE);
  else
    CHubComponentsUpdater::Instance()->force_update(
        IUpdaterComponent::VMWARE);
}

void DlgAbout::btn_vagrant_vmware_utility_update_released() {
  // Check is VMware is installed
  if (ui->lbl_hypervisor_vmware_version->text() == "undefined") {
    // install first VMware
    QMessageBox msg;
    msg.setText(QObject::tr(
                     "Vagrant VMware Utility requires a valid installation of VMware."
                     " Please install VMware and then install Vagrant VMware Utility again!"
                  ));
    msg.exec();
  } else {
    ui->pb_provider_vmare_utility->setHidden(false);
    ui->btn_provider_vmware_utility_update->setHidden(false);
    ui->btn_provider_vmware_utility_update->setEnabled(false);

    if (ui->lbl_provider_vmware_utility_version->text() == "undefined")
      CHubComponentsUpdater::Instance()->install(
            IUpdaterComponent::VMWARE_UTILITY);
    else
      CHubComponentsUpdater::Instance()->force_update(
            IUpdaterComponent::VMWARE_UTILITY);
  }
}
////////////////////////////////////////////////////////////////////////////
/// \brief DlgAbout::btn_uninstall_components
///
void DlgAbout::btn_uninstall_components() {
  std::vector <std::pair<int, QString> > uninstall_vector; // pair <priority, name> of uninstalled component
  static QStringList high_priority_component = {IUpdaterComponent::SUBUTAI_BOX,
                                                IUpdaterComponent::VAGRANT_VBGUEST,
                                                IUpdaterComponent::VAGRANT_SUBUTAI,
                                                IUpdaterComponent::E2E}; // components with 1 priority, other will be 0

  QString uninstalling_components_str;
  for (const auto& component : m_dct_fpb) {
    if (component.second.cb == nullptr) {continue;}
    if (component.second.cb->isChecked() && component.second.cb->isVisible()) {
      qDebug() << "Checkbox enabled: " << component.first;
      if (m_dct_fpb[component.first].lbl->text() == "Install Vagrant first" ||
          m_dct_fpb[component.first].lbl->text() == "No supported browser is available") {
        continue;
      }
      uninstalling_components_str += "<i>" + IUpdaterComponent::component_id_to_user_view(component.first) + "</i><br>";
      uninstall_vector.push_back(std::make_pair(!high_priority_component.contains(component.first), component.first));
    }
  }
  if (uninstall_vector.empty()) {return;}

  QMessageBox *msg_box = new QMessageBox(
      QMessageBox::Information, QObject::tr("Attention!"),
      QObject::tr("<b>You are going to uninstall following components:</b><br>%1"
          "Do you want to proceed?").arg(uninstalling_components_str),
      QMessageBox::Yes | QMessageBox::No);
  msg_box->setTextFormat(Qt::RichText);
  QObject::connect(msg_box, &QMessageBox::finished, msg_box,
                   &QMessageBox::deleteLater);
  if (msg_box->exec() != QMessageBox::Yes) {
    return;
  }

  sort(uninstall_vector.begin(), uninstall_vector.end());
  for (size_t i = 0; i < uninstall_vector.size(); i++) {
    QString componen_id = uninstall_vector[i].second;
    m_dct_fpb[componen_id].pb->setEnabled(true);
    m_dct_fpb[componen_id].pb->setVisible(true);
    CHubComponentsUpdater::Instance()->uninstall(componen_id);
  }
}

////////////////////////////////////////////////////////////////////////////
void DlgAbout::download_progress(const QString& component_id, qint64 rec,
                                 qint64 total) {
  if (m_dct_fpb.find(component_id) == m_dct_fpb.end()) return;
  if (total == 0) {
    m_dct_fpb[component_id].pb->setValue(0);
    m_dct_fpb[component_id].pb->setMinimum(0);
    m_dct_fpb[component_id].pb->setMaximum(0);
  } else {
    m_dct_fpb[component_id].pb->setValue((rec * 100) / total);
  }
}

////////////////////////////////////////////////////////////////////////////
void DlgAbout::update_available(const QString& file_id) {
  if (m_dct_fpb.find(file_id) == m_dct_fpb.end()) return;
  m_dct_fpb[file_id].pb->setEnabled(
      !(CHubComponentsUpdater::Instance()->is_in_progress(file_id)));
  m_dct_fpb[file_id].btn->setEnabled(
      !(CHubComponentsUpdater::Instance()->is_in_progress(file_id)));
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::update_finished(const QString& component_id, bool success) {
  if (m_dct_fpb.find(component_id) == m_dct_fpb.end()) return;

  m_dct_fpb[component_id].btn->setEnabled(false);
  m_dct_fpb[component_id].pb->setValue(0);
  m_dct_fpb[component_id].pb->setRange(0, 100);

  if (m_dct_fpb[component_id].pf_version) {
    m_dct_fpb[component_id].pb->setHidden(true);
    m_dct_fpb[component_id].lbl->setText(m_dct_fpb[component_id].pf_version());
  }

  if (success) {
    if (m_dct_fpb[component_id].cb != nullptr) {
      m_dct_fpb[component_id].btn->setVisible(false);
      m_dct_fpb[component_id].cb->setChecked(false);
      m_dct_fpb[component_id].cb->setVisible(true);
    }
  } else {
    if (m_dct_fpb[component_id].cb != nullptr) {
      m_dct_fpb[component_id].btn->setEnabled(true);
      m_dct_fpb[component_id].cb->setChecked(false);
      m_dct_fpb[component_id].cb->setVisible(false);
    }
  }
  m_dct_fpb[component_id].pb->setVisible(false);

  qDebug() << "update finished: " << component_id << " status: " << success;
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::initialization_finished() {
  ui->lbl_about_init->setEnabled(false);
  ui->pb_initialization_progress->setEnabled(false);
  ui->btn_recheck->setEnabled(true);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::init_progress_sl(int part, int total) {
  UNUSED_ARG(total);
  ui->pb_initialization_progress->setValue(part);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::got_p2p_version_sl(QString version) {
  // check component key is exist
  if (this->m_dct_fpb.find(IUpdaterComponent::P2P) != this->m_dct_fpb.end()) {
    ui->lbl_p2p_version_val->setText(version);
    if (version == "undefined") {
      set_hidden_pb(IUpdaterComponent::P2P);
      ui->btn_p2p_update->setHidden(false);
      ui->cb_p2p->setVisible(false);
      ui->btn_p2p_update->setText(tr("Install"));
      ui->btn_p2p_update->activateWindow();
    } else
      ui->btn_p2p_update->setText(tr("Update"));
  }
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::got_chrome_version_sl(QString version) {
  if (this->m_dct_fpb.find(IUpdaterComponent::CHROME) != this->m_dct_fpb.end()) {
    if (CSettingsManager::Instance().default_browser() == "Chrome") {
      ui->lbl_chrome_version_val->setText(version);
      if (version == "undefined") {
        set_hidden_pb(IUpdaterComponent::CHROME);
        ui->btn_chrome->setHidden(false);
        ui->cb_chrome->setVisible(false);
        ui->btn_chrome->setText(tr("Install"));
        ui->btn_chrome->activateWindow();
      } else {
        ui->btn_chrome->setText(tr("Update"));
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////

void DlgAbout::got_firefox_version_sl(QString version) {
  if (CSettingsManager::Instance().default_browser() == "Firefox") {
    ui->lbl_firefox_version_val->setText(version);
    if (version == "undefined") {
      set_hidden_pb(IUpdaterComponent::FIREFOX);
      ui->btn_firefox->setHidden(false);
      ui->cb_firefox->setVisible(false);
      ui->btn_firefox->setText(tr("Install"));
      ui->btn_firefox->activateWindow();
    } else {
      ui->btn_firefox->setText(tr("Update"));
    }
  }
}

////////////////////////////////////////////////////////////////////////////

void DlgAbout::got_edge_version_sl(QString version) {
  ui->lbl_edge_version_val->setText(version);
}

////////////////////////////////////////////////////////////////////////////

void DlgAbout::got_safari_version_sl(QString version) {
  ui->lbl_safari_version_val->setText(version);
}

////////////////////////////////////////////////////////////////////////////

void DlgAbout::got_e2e_version_sl(QString version) {
  if (this->m_dct_fpb.find(IUpdaterComponent::E2E) != this->m_dct_fpb.end()) {
    ui->lbl_subutai_e2e_val->setText(version);
    if (current_browser != "Edge") {
      if (version == "undefined") {
        set_hidden_pb(IUpdaterComponent::E2E);
        ui->btn_subutai_e2e->setHidden(false);
        ui->cb_subutai_e2e->setVisible(false);
        ui->btn_subutai_e2e->setText(tr("Install"));
        ui->cb_subutai_e2e->setEnabled(true);
      } else if(version == "No supported browser is available") {
        ui->cb_subutai_e2e->setEnabled(false);
      } else {
        ui->btn_subutai_e2e->setText(tr("Update"));
        ui->cb_subutai_e2e->setEnabled(true);
      }
    } else {
      ui->btn_subutai_e2e->setVisible(false);
    }
  }
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::got_x2go_version_sl(QString version) {
  if (this->m_dct_fpb.find(IUpdaterComponent::X2GO) != this->m_dct_fpb.end()) {
    if (version == "undefined") {
      set_hidden_pb(IUpdaterComponent::X2GO);
      ui->btn_x2go_update->setHidden(false);
      ui->cb_x2goclient->setVisible(false);
      ui->btn_x2go_update->setText(tr("Install"));
      ui->btn_x2go_update->activateWindow();
    } else {
      ui->btn_x2go_update->setText(tr("Update"));
    }
    ui->lbl_x2go_version_val->setText(version);
  }
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::got_vagrant_version_sl(QString version) {
  if (this->m_dct_fpb.find(IUpdaterComponent::VAGRANT) != this->m_dct_fpb.end()) {
    if (version == "undefined") {
      set_hidden_pb(IUpdaterComponent::VAGRANT);
      ui->btn_vagrant_update->setHidden(false);
      ui->cb_vagrant->setVisible(false);
      ui->btn_vagrant_update->setText(tr("Install"));
      ui->btn_vagrant_update->activateWindow();
    } else {
      ui->btn_vagrant_update->setText(tr("Update"));
    }
    ui->lbl_vagrant_version_val->setText(version);
  }
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::got_oracle_virtualbox_version_sl(QString version) {
  if (VagrantProvider::Instance()->CurrentProvider() != VagrantProvider::VIRTUALBOX) {
    return;
  }
  if (this->m_dct_fpb.find(IUpdaterComponent::ORACLE_VIRTUALBOX) != this->m_dct_fpb.end()) {
    if (version == "undefined") {
      set_hidden_pb(IUpdaterComponent::ORACLE_VIRTUALBOX);
      ui->btn_oracle_virtualbox_update->setHidden(false);
      ui->cb_oracle_virtualbox->setVisible(false);
      ui->btn_oracle_virtualbox_update->setText(tr("Install"));
      ui->btn_oracle_virtualbox_update->activateWindow();
    } else {
      ui->btn_oracle_virtualbox_update->setText(tr("Update"));
    }
    ui->lbl_oracle_virtualbox_version_val->setText(version);
  }
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::got_subutai_plugin_version_sl(QString version) {
  if (this->m_dct_fpb.find(IUpdaterComponent::VAGRANT_SUBUTAI) != this->m_dct_fpb.end()) {
    if (version == "undefined") {
      set_hidden_pb(IUpdaterComponent::VAGRANT_SUBUTAI);
      ui->btn_subutai_plugin_update->setHidden(false);
      ui->cb_vagrant_subtuai_plugin->setVisible(false);
      ui->btn_subutai_plugin_update->setText(tr("Install"));
      ui->btn_subutai_plugin_update->activateWindow();
      ui->cb_vagrant_subtuai_plugin->setEnabled(true);
    } else if(version == "Install Vagrant first") {
      ui->cb_vagrant_subtuai_plugin->setEnabled(false);
    } else {
      ui->btn_subutai_plugin_update->setText(tr("Update"));
      ui->cb_vagrant_subtuai_plugin->setEnabled(true);
    }
    ui->lbl_subutai_plugin_version_val->setText(version);
  }
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::got_vbguest_plugin_version_sl(QString version) {
  if (VagrantProvider::Instance()->CurrentProvider() != VagrantProvider::VIRTUALBOX) {
    return;
  }
  if (this->m_dct_fpb.find(IUpdaterComponent::VAGRANT_VBGUEST) != this->m_dct_fpb.end()) {
    if (version == "undefined") {
      set_hidden_pb(IUpdaterComponent::VAGRANT_VBGUEST);
      ui->btn_vbguest_plugin_update->setHidden(false);
      ui->cb_vagrant_vbguest_plugin->setVisible(false);
      ui->btn_vbguest_plugin_update->setText(tr("Install"));
      ui->btn_vbguest_plugin_update->activateWindow();
      ui->cb_vagrant_vbguest_plugin->setEnabled(true);
    } else if(version == "Install Vagrant first") {
      ui->cb_vagrant_vbguest_plugin->setEnabled(false);
    } else {
      ui->btn_vbguest_plugin_update->setText(tr("Update"));
      ui->cb_vagrant_vbguest_plugin->setEnabled(true);
    }
  }
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::got_subutai_box_version_sl(QString version) {
  if (this->m_dct_fpb.find(IUpdaterComponent::SUBUTAI_BOX) != this->m_dct_fpb.end()) {
    if (version == "undefined") {
      set_hidden_pb(IUpdaterComponent::SUBUTAI_BOX);
      ui->btn_subutai_box->setHidden(false);
      ui->cb_vagrant_box->setVisible(false);
      ui->btn_subutai_box->setText(tr("Install"));
      ui->btn_subutai_box->activateWindow();
    } else {
      ui->btn_subutai_box->setText(tr("Update"));
    }
    ui->lbl_subutai_box_version->setText(version);
  }
}

void DlgAbout::got_hypervisor_vmware_version_sl(QString version) {
  if (VagrantProvider::Instance()->CurrentProvider() != VagrantProvider::VMWARE_DESKTOP) {
    return;
  }
  if (this->m_dct_fpb.find(IUpdaterComponent::VMWARE) != this->m_dct_fpb.end()) {
    if (version == "undefined") {
      set_hidden_pb(IUpdaterComponent::VMWARE);
      ui->btn_hypervisor_vmware_update->setHidden(false);
      ui->cb_hypervisor_vmware->setVisible(false);
      ui->btn_hypervisor_vmware_update->setText(tr("Install"));
      ui->btn_hypervisor_vmware_update->activateWindow();
    } else {
      ui->btn_hypervisor_vmware_update->setText(tr("Update"));
    }
    ui->lbl_hypervisor_vmware_version->setText(version);
  }
}

void DlgAbout::got_vagrant_vmware_utility_version_sl(QString version) {
  if (VagrantProvider::Instance()->CurrentProvider() != VagrantProvider::VMWARE_DESKTOP) {
    return;
  }
  if (this->m_dct_fpb.find(IUpdaterComponent::VMWARE_UTILITY) != this->m_dct_fpb.end()) {
    if (version == "undefined") {
      set_hidden_pb(IUpdaterComponent::VMWARE_UTILITY);
      ui->btn_provider_vmware_utility_update->setHidden(false);
      ui->cb_provider_vmware_utility->setHidden(false);
      ui->btn_provider_vmware_utility_update->setText(tr("Install"));
      ui->btn_provider_vmware_utility_update->activateWindow();
    } else {
      ui->btn_provider_vmware_utility_update->setText(tr("Update"));
    }
    ui->lbl_provider_vmware_utility_version->setText(version);
  }
}

void DlgAbout::got_provider_version_sl(QString version) {
  QString COMPONENT_KEY = "";

  switch (VagrantProvider::Instance()->CurrentProvider()) {
  //case VagrantProvider::PARALLELS:
  //  COMPONENT_KEY = IUpdaterComponent::VAGRANT_PARALLELS;
  //  break;
  case VagrantProvider::VMWARE_DESKTOP:
    COMPONENT_KEY = IUpdaterComponent::VAGRANT_VMWARE_DESKTOP;
    break;
  //case VagrantProvider::LIBVIRT:
  //  COMPONENT_KEY = IUpdaterComponent::VAGRANT_LIBVIRT;
  //  break;
  default:
    break;
  }

  qDebug() << "Got provider version:"
           << COMPONENT_KEY
           << version;

  // check component key is exist
  if (this->m_dct_fpb.find(COMPONENT_KEY) != this->m_dct_fpb.end()) {
    if (version == "undefined") {
      set_hidden_pb(COMPONENT_KEY);
      this->m_dct_fpb[COMPONENT_KEY].btn->setHidden(false);
      //ui->btn_subutai_box->setHidden(false);
      this->m_dct_fpb[COMPONENT_KEY].cb->setVisible(false);
      //ui->cb_vagrant_box->setVisible(false);
      this->m_dct_fpb[COMPONENT_KEY].btn->setText(tr("Install"));
      //ui->btn_subutai_box->setText(tr("Install"));
      this->m_dct_fpb[COMPONENT_KEY].btn->activateWindow();
      //ui->btn_subutai_box->activateWindow();
    } else {
      this->m_dct_fpb[COMPONENT_KEY].btn->setText(tr("Update"));
      //ui->btn_subutai_box->setText(tr("Update"));
    }
    this->m_dct_fpb[COMPONENT_KEY].lbl->setText(version);
    //ui->lbl_subutai_box_version->setText(version);
  }
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::got_xquartz_version_sl(QString version) {
  if (OS_MAC != CURRENT_OS) return;
  ui->lbl_xquartz_version_val->setText(version);
  if (version == "undefined") {
    set_hidden_pb(IUpdaterComponent::XQUARTZ);
    ui->btn_xquartz_update->setHidden(false);
    ui->cb_xquartz->setVisible(false);
    ui->btn_xquartz_update->setText(tr("Install"));
    ui->btn_xquartz_update->activateWindow();
  } else
    ui->btn_xquartz_update->setText(tr("Update"));
}
////////////////////////////////////////////////////////////////////////////
void DlgAbout::set_hidden_pb(const QString& component_id) {
  if (CHubComponentsUpdater::Instance()->is_in_progress(component_id)) {
    m_dct_fpb[component_id].pb->setVisible(true);
  } else {
    m_dct_fpb[component_id].pb->setHidden(true);
  }
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::update_available_sl(const QString& component_id,
                                   bool available) {
  bool update_available = available;
  if (component_id == IUpdaterComponent::XQUARTZ
      && OS_MAC != CURRENT_OS) {
    return;
  }
  if (m_dct_fpb.find(component_id) == m_dct_fpb.end()) {
    return;
  }
  // update available component
  if (update_available) {
    qInfo() << "update available: " << component_id;
    if (m_dct_fpb[component_id].cb != nullptr) {
      m_dct_fpb[component_id].btn->setVisible(true);
      m_dct_fpb[component_id].cb->setHidden(true);
      m_dct_fpb[component_id].cb->setChecked(false);
    }
  } else if (m_dct_fpb[component_id].cb != nullptr) {
    // not available component
    m_dct_fpb[component_id].btn->setHidden(true);
    m_dct_fpb[component_id].cb->setVisible(true);
  }

  update_available =
      (!(CHubComponentsUpdater::Instance()->is_in_progress(component_id)) &&
       available);
  m_dct_fpb[component_id].btn->setEnabled(update_available);

  if (component_id == IUpdaterComponent::E2E && current_browser == "Edge") {
    m_dct_fpb[component_id].cb->setVisible(false);
    m_dct_fpb[component_id].btn->setVisible(false);
  }

  if (component_id == IUpdaterComponent::FIREFOX && current_browser != "Firefox") {
    m_dct_fpb[component_id].cb->setVisible(false);
    m_dct_fpb[component_id].btn->setVisible(false);
  }

  if (component_id == IUpdaterComponent::CHROME && current_browser != "Chrome") {
    m_dct_fpb[component_id].cb->setVisible(false);
    m_dct_fpb[component_id].btn->setVisible(false);
  }

  VagrantProvider::PROVIDERS current_provider =
      VagrantProvider::Instance()->CurrentProvider();

  if (component_id == IUpdaterComponent::ORACLE_VIRTUALBOX &&
      current_provider != VagrantProvider::VIRTUALBOX) {
    m_dct_fpb[component_id].cb->setVisible(false);
    m_dct_fpb[component_id].btn->setVisible(false);
  }

  if (component_id == IUpdaterComponent::VAGRANT_VBGUEST &&
      current_provider != VagrantProvider::VIRTUALBOX) {
    m_dct_fpb[component_id].cb->setVisible(false);
    m_dct_fpb[component_id].btn->setVisible(false);
  }

  if (component_id == IUpdaterComponent::VMWARE &&
      current_provider != VagrantProvider::VMWARE_DESKTOP) {
    m_dct_fpb[component_id].cb->setVisible(false);
    m_dct_fpb[component_id].btn->setVisible(false);
  }

  if (component_id == IUpdaterComponent::VMWARE_UTILITY &&
      current_provider != VagrantProvider::VMWARE_DESKTOP) {
    m_dct_fpb[component_id].cb->setVisible(false);
    m_dct_fpb[component_id].btn->setVisible(false);
  }

  if (component_id == IUpdaterComponent::VAGRANT_VMWARE_DESKTOP &&
      current_provider != VagrantProvider::VMWARE_DESKTOP) {
    m_dct_fpb[component_id].cb->setVisible(false);
    m_dct_fpb[component_id].btn->setVisible(false);
  }

  if (component_id == IUpdaterComponent::VAGRANT_LIBVIRT &&
      current_provider != VagrantProvider::LIBVIRT) {
    m_dct_fpb[component_id].cb->setVisible(false);
    m_dct_fpb[component_id].btn->setVisible(false);
  }

  if (component_id == IUpdaterComponent::VAGRANT_PARALLELS &&
      current_provider != VagrantProvider::PARALLELS) {
    m_dct_fpb[component_id].cb->setVisible(false);
    m_dct_fpb[component_id].btn->setVisible(false);
  }
}
////////////////////////////////////////////////////////////////////////////

void DlgAboutInitializer::do_initialization() {
  try {
    int initialized_component_count = 0;

    QString p2p_version = get_p2p_version();
    emit got_p2p_version(p2p_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString chrome_version;
    CSystemCallWrapper::chrome_version(chrome_version);
    emit got_chrome_version(chrome_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString firefox_version;
    CSystemCallWrapper::firefox_version(firefox_version);
    emit got_firefox_version(firefox_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString edge_version;
    CSystemCallWrapper::edge_version(edge_version);
    emit got_edge_version(edge_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString safari_version;
    CSystemCallWrapper::safari_version(safari_version);
    emit got_safari_version(safari_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString x2go_version = get_x2go_version();
    emit got_x2go_version(x2go_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString vagrant_version = get_vagrant_version();
    emit got_vagrant_version(vagrant_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString oracle_virtualbox_version = get_oracle_virtualbox_version();
    emit got_oracle_virtualbox_version(oracle_virtualbox_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString subutai_e2e_version = get_e2e_version();
    emit got_e2e_version(subutai_e2e_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString subutai_plugin_versin = get_vagrant_subutai_version();
    emit got_subutai_plugin_version(subutai_plugin_versin);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString vbguest_plugin_version = get_vagrant_vbguest_version();
    emit got_vbguest_plugin_version(vbguest_plugin_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString subutai_box_version = get_subutai_box_version();
    emit got_subutai_box_version(subutai_box_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString vagrant_provider_version = get_vagrant_provider_version();
    emit got_provider_version(vagrant_provider_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString hypervisor_vmware_version = get_hypervisor_vmware_version();
    emit got_hypervisor_vmware_version(hypervisor_vmware_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString vagrant_vmware_utility_version = get_vagrant_vmware_utility_version();
    emit got_vagrant_vmware_utility_version(vagrant_vmware_utility_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    if (OS_MAC == CURRENT_OS) {
      QString xquartz_version = get_xquartz_version();
      emit got_xquartz_version(xquartz_version);
    }
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);


    std::vector<QString> uas = {IUpdaterComponent::P2P,
                     IUpdaterComponent::TRAY,
                     IUpdaterComponent::X2GO,
                     IUpdaterComponent::E2E,
                     IUpdaterComponent::VAGRANT,
                     IUpdaterComponent::CHROME,
                     IUpdaterComponent::FIREFOX,
                     IUpdaterComponent::VAGRANT_SUBUTAI,
                     IUpdaterComponent::SUBUTAI_BOX,
                     IUpdaterComponent::XQUARTZ
                     };

    switch(VagrantProvider::Instance()->CurrentProvider()) {
    case VagrantProvider::VIRTUALBOX:
      uas.push_back(IUpdaterComponent::ORACLE_VIRTUALBOX);
      uas.push_back(IUpdaterComponent::VAGRANT_VBGUEST);
      break;
    //case VagrantProvider::PARALLELS:
    //  uas.push_back(IUpdaterComponent::VAGRANT_PARALLELS);
    //  break;
    case VagrantProvider::VMWARE_DESKTOP:
      uas.push_back(IUpdaterComponent::VMWARE);
      uas.push_back(IUpdaterComponent::VAGRANT_VMWARE_DESKTOP);
      uas.push_back(IUpdaterComponent::VMWARE_UTILITY);
      break;
    //case VagrantProvider::LIBVIRT:
    //  uas.push_back(IUpdaterComponent::VAGRANT_LIBVIRT);
    //  break;
    //case VagrantProvider::HYPERV:
      // do nothing
    //  break;
    default:
      uas.push_back(IUpdaterComponent::ORACLE_VIRTUALBOX);
      uas.push_back(IUpdaterComponent::VAGRANT_VBGUEST);
      break;
    }

    std::vector<bool> ua;
    for (int i = 0; i < (int) uas.size(); ++i) {
      ua.push_back(
          CHubComponentsUpdater::Instance()->is_update_available(uas.at(i)));
      emit init_progress(++initialized_component_count, COMPONENTS_COUNT);
    }

    for (int i = 0; i < (int) uas.size(); i++) {
      if (uas[i] != IUpdaterComponent::E2E || CSettingsManager::Instance().default_browser() != "Edge") {
        emit update_available(uas[i], ua[i]);
      }
    }
  } catch (std::exception& ex) {
    qCritical("Err in DlgAboutInitializer::do_initialization() . %s",
              ex.what());
  }

  emit finished();
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::install_finished(const QString& component_id, bool success) {
  qDebug() << "Install finished for: " << component_id << "with result"
           << success;

  if (m_dct_fpb.find(component_id) == m_dct_fpb.end()) return;
  m_dct_fpb[component_id].btn->setEnabled(false);
  m_dct_fpb[component_id].pb->setValue(0);
  m_dct_fpb[component_id].pb->setRange(0, 100);
  m_dct_fpb[component_id].btn->setText(tr("Update"));
  if (m_dct_fpb[component_id].pf_version) {
    m_dct_fpb[component_id].lbl->setText(m_dct_fpb[component_id].pf_version());
  }

  if (success) {
    m_dct_fpb[component_id].pb->setHidden(true);
    if (m_dct_fpb[component_id].cb != nullptr) {
      m_dct_fpb[component_id].btn->setHidden(true);
      m_dct_fpb[component_id].cb->setChecked(false);
      m_dct_fpb[component_id].cb->setVisible(true);
    }
  } else {
    m_dct_fpb[component_id].btn->setEnabled(true);
    m_dct_fpb[component_id].btn->setText(tr("Install"));
    if (m_dct_fpb[component_id].cb != nullptr) {
      m_dct_fpb[component_id].pb->setVisible(false);
      m_dct_fpb[component_id].cb->setVisible(false);
      m_dct_fpb[component_id].cb->setEnabled(false);
    }
  }
}

void DlgAbout::uninstall_finished(const QString& component_id, bool success) {
  qDebug() << "Uninstall finished for: " << component_id << "with result"
           << success;

  if (m_dct_fpb.find(component_id) == m_dct_fpb.end()) return;
  if (success) {
    m_dct_fpb[component_id].lbl->setText(m_dct_fpb[component_id].pf_version());
    m_dct_fpb[component_id].cb->setVisible(false);
    m_dct_fpb[component_id].cb->setChecked(false);
    m_dct_fpb[component_id].btn->setVisible(true);
    m_dct_fpb[component_id].btn->setEnabled(true);
    m_dct_fpb[component_id].btn->setText(tr("Install"));
  }
  m_dct_fpb[component_id].cb->setChecked(false);
  m_dct_fpb[component_id].pb->setValue(0);
  m_dct_fpb[component_id].pb->setRange(0, 100);
  m_dct_fpb[component_id].pb->setVisible(false);
}

////////////////////////////////////////////////////////////////////////////
void DlgAboutInitializer::abort() { emit finished(); }
////////////////////////////////////////////////////////////////////////////
