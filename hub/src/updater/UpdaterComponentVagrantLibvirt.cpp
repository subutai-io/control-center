#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>

#include "Commons.h"
#include "DownloadFileManager.h"
#include "NotificationObserver.h"
#include "OsBranchConsts.h"
#include "P2PController.h"
#include "RestWorker.h"
#include "SystemCallWrapper.h"
#include "updater/ExecutableUpdater.h"
#include "updater/HubComponentsUpdater.h"
#include "updater/UpdaterComponentVagrantLibvirt.h"

CUpdaterComponentVAGRANT_LIBVIRT::CUpdaterComponentVAGRANT_LIBVIRT() {
  this->m_component_id = VAGRANT_LIBVIRT;
}

CUpdaterComponentVAGRANT_LIBVIRT::~CUpdaterComponentVAGRANT_LIBVIRT() {}

bool CUpdaterComponentVAGRANT_LIBVIRT::update_available_internal() {
  QString version;
  QString vagrant_plugin = "vagrant-libvirt";
  system_call_wrapper_error_t res =
      CSystemCallWrapper::vagrant_plugin_version(version, vagrant_plugin);
  QString cloud_version =
      CRestWorker::Instance()->get_vagrant_plugin_cloud_version(vagrant_plugin);
  if (version == "undefined") return true;
  if (res != SCWE_SUCCESS) return false;
  if (cloud_version == "undefined" || cloud_version.isEmpty()) return false;
  return cloud_version > version;
}

chue_t CUpdaterComponentVAGRANT_LIBVIRT::install_internal() {
  qDebug() << "Starting install vagrant libvirt";

  QMessageBox *msg_box = new QMessageBox(
      QMessageBox::Information, QObject::tr("Attention!"),
      QObject::tr(
          "This is a Vagrant plugin that adds an Libvirt provider to Vagrant, "
          "allowing Vagrant to control and provision machines via Libvirt toolkit.\n"
          "The Vagrant Libvirt provider will be installed on your machine.\n"
          "Do you want to proceed?"),
      QMessageBox::Yes | QMessageBox::No);

  QObject::connect(msg_box, &QMessageBox::finished, msg_box,
                   &QMessageBox::deleteLater);
  if (msg_box->exec() != QMessageBox::Yes) {
    install_finished_sl(false, "undefined");
    return CHUE_SUCCESS;
  }

  update_progress_sl(0, 0);
  static QString empty_string = "";
  SilentInstaller *silent_installer = new SilentInstaller(this);
  silent_installer->init(empty_string, empty_string, CC_VAGRANT_LIBVIRT);
  connect(silent_installer, &SilentInstaller::outputReceived, this,
          &CUpdaterComponentVAGRANT_LIBVIRT::install_finished_sl);
  silent_installer->startWork();
  return CHUE_SUCCESS;
}

chue_t CUpdaterComponentVAGRANT_LIBVIRT::update_internal() {
  update_progress_sl(100, 100);
  static QString empty_string = "";

  SilentUpdater *silent_updater = new SilentUpdater(this);
  silent_updater->init(empty_string, empty_string, CC_VAGRANT_LIBVIRT);

  connect(silent_updater, &SilentUpdater::outputReceived, this,
          &CUpdaterComponentVAGRANT_LIBVIRT::update_finished_sl);

  silent_updater->startWork();

  return CHUE_SUCCESS;
}

chue_t CUpdaterComponentVAGRANT_LIBVIRT::uninstall_internal() {
  update_progress_sl(50, 100);
  static QString empty_string = "";

  SilentUninstaller *silent_uninstaller = new SilentUninstaller(this);
  silent_uninstaller->init(empty_string, empty_string, CC_VAGRANT_LIBVIRT);

  connect(silent_uninstaller, &SilentUninstaller::outputReceived, this,
          &CUpdaterComponentVAGRANT_LIBVIRT::uninstall_finished_sl);

  silent_uninstaller->startWork();

  return CHUE_SUCCESS;
}

void CUpdaterComponentVAGRANT_LIBVIRT::update_post_action(bool success) {
  if (!success) {
    CNotificationObserver::Instance()->Info(
        tr("Failed to update the Vagrant Libvirt provider. You may try manually "
           "installing it "
           "or try again by restarting the Control Center first."),
        DlgNotification::N_NO_ACTION);
  } else {
    CNotificationObserver::Instance()->Info(
        tr("Vagrant Libvirt provider has been updated successfully."),
        DlgNotification::N_NO_ACTION);
  }
}
void CUpdaterComponentVAGRANT_LIBVIRT::install_post_internal(bool success) {
  if (!success) {
    CNotificationObserver::Instance()->Info(
        tr("Failed to install the Vagrant Libvirt provider. You may try manually "
           "installing it "
           "or try again by restarting the Control Center first."),
        DlgNotification::N_NO_ACTION);
  } else {
    CNotificationObserver::Instance()->Info(
        tr("Vagrant Libvirt provider has been installed successfully."),
        DlgNotification::N_NO_ACTION);
  }
}

void CUpdaterComponentVAGRANT_LIBVIRT::uninstall_post_internal(bool success) {
  if (!success) {
    CNotificationObserver::Instance()->Info(
        tr("Failed to uninstall the Vagrant Libvirt provider. You may try "
           "manually uninstalling it "
           "or try again by restarting the Control Center first."),
        DlgNotification::N_NO_ACTION);
  } else {
    CNotificationObserver::Instance()->Info(
        tr("Vagrant Libvirt provider has been uninstalled successfully."),
        DlgNotification::N_NO_ACTION);
  }
}
