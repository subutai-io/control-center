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
#include "updater/UpdaterComponentVagrantVMwareUtility.h".h"

CUpdaterComponentVagrantVMwareUtility::CUpdaterComponentVagrantVMwareUtility() {
  this->m_component_id = VMWARE_UTILITY;
}

CUpdaterComponentVagrantVMwareUtility::~CUpdaterComponentVagrantVMwareUtility() {}

bool CUpdaterComponentVagrantVMwareUtility::update_available_internal() {
  QString version = "1.0.2";
  //CSystemCallWrapper::vagrant_version(version); TODO implement version
  return version == "undefined";
}

chue_t CUpdaterComponentVagrantVMwareUtility::install_internal() {
  qDebug() << "Starting installation vagrant-vmware-utility";

  QMessageBox *msg_box = new QMessageBox(
      QMessageBox::Information, QObject::tr("Attention!"),
      QObject::tr(
          "The Vagrant VMware Utility provides the Vagrant VMware provider plugin"
          " access to various VMware functionalities."
          " The Vagrant VMware Utility is required by "
          " the Vagrant VMware Desktop provider plugin.\n"
          "Do you want to proceed?"),
      QMessageBox::Yes | QMessageBox::No);

  QObject::connect(msg_box, &QMessageBox::finished, msg_box,
                   &QMessageBox::deleteLater);
  if (msg_box->exec() != QMessageBox::Yes) {
    install_finished_sl(false);
    return CHUE_SUCCESS;
  }

  update_progress_sl(50, 100);  // imitation of progress bar :D, todo implement
  static QString empty_string = "";
  SilentInstaller *silent_installer = new SilentInstaller(this);
  silent_installer->init(empty_string, empty_string, CC_VMWARE_UTILITY);
  connect(silent_installer, &SilentInstaller::outputReceived, this,
          &CUpdaterComponentVagrantVMwareUtility::install_finished_sl);
  silent_installer->startWork();
  return CHUE_SUCCESS;
}

chue_t CUpdaterComponentVagrantVMwareUtility::update_internal() {
  update_progress_sl(100, 100);
  update_finished_sl(true);
  return CHUE_SUCCESS;
}

chue_t CUpdaterComponentVagrantVMwareUtility::uninstall_internal() {
  update_progress_sl(50, 100);
  static QString empty_string = "";

  SilentUninstaller *silent_uninstaller = new SilentUninstaller(this);
  silent_uninstaller->init(empty_string, empty_string, CC_VMWARE_UTILITY);

  connect(silent_uninstaller, &SilentUninstaller::outputReceived, this,
          &CUpdaterComponentVagrantVMwareUtility::uninstall_finished_sl);

  silent_uninstaller->startWork();

  return CHUE_SUCCESS;
}

void CUpdaterComponentVagrantVMwareUtility::update_post_action(bool success) {
  UNUSED_ARG(success);
}
void CUpdaterComponentVagrantVMwareUtility::install_post_interntal(bool success) {
  if (!success) {
    CNotificationObserver::Instance()->Info(
        tr("Failed to install the Vagrant VMware Utility. You may try manually "
           "installing it "
           "or try again by restarting the Control Center first."),
        DlgNotification::N_NO_ACTION);
  } else {
    CNotificationObserver::Instance()->Info(
        tr("Vagrant VMware Utility has been installed successfully."),
        DlgNotification::N_NO_ACTION);
  }
}

void CUpdaterComponentVagrantVMwareUtility::uninstall_post_internal(bool success) {
  if (!success) {
    CNotificationObserver::Instance()->Info(
        tr("Failed to uninstall the Vagrant VMware Utility. You may try "
           "manually uninstalling it "
           "or try again by restarting the Control Center first."),
        DlgNotification::N_NO_ACTION);
  } else {
    CNotificationObserver::Instance()->Info(
        tr("Vagrant VMware Utility has been uninstalled successfully."),
        DlgNotification::N_NO_ACTION);
  }
}
