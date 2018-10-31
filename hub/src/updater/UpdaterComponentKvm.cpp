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
#include "updater/UpdaterComponentKvm.h"
#include "Environment.h"

CUpdaterComponentKvm::CUpdaterComponentKvm() {
  m_component_id = IUpdaterComponent::KVM;
}

CUpdaterComponentKvm::~CUpdaterComponentKvm() {}

bool CUpdaterComponentKvm::update_available_internal() {
  QString version;
  CSystemCallWrapper::kvm_version(version);
  return version == "undefined";
}

chue_t CUpdaterComponentKvm::install_internal() {
  qDebug() << "Starting install KVM";

  QMessageBox *msg_box = new QMessageBox(
      QMessageBox::Information, QObject::tr("Attention!"),
      QObject::tr(
          "KVM is used as the hypervisor.<br>"
          "KVM (for Kernel-based Virtual Machine)\n"
          "is a full virtualization solution for\n"
          "Linux on x86 hardware containing virtualization\n"
          "extensions (Intel VT or AMD-V).\n"
          "Do you want to proceed?"),
      QMessageBox::Yes | QMessageBox::No);

  QObject::connect(msg_box, &QMessageBox::finished, msg_box,
                   &QMessageBox::deleteLater);
  if (msg_box->exec() != QMessageBox::Yes) {
    install_finished_sl(false, "undefined");
    return CHUE_SUCCESS;
  }

  // Pre-installation checks
  if (!Environment::Instance()->isCpuSupport()) {
    QMessageBox *msg_box = new QMessageBox(
        QMessageBox::Information, QObject::tr("Attention!"),
        QObject::tr("Your CPU doesn't support hardware virtualization."),
        QMessageBox::Ok);

    QObject::connect(msg_box, &QMessageBox::finished, msg_box,
                     &QMessageBox::deleteLater);
    if (msg_box->exec() == QMessageBox::Ok) {
      install_finished_sl(false, "undefined");
      return CHUE_SUCCESS;
    }
  }

  update_progress_sl(100, 100);
  static QString empty_string = "";
  SilentInstaller *silent_installer = new SilentInstaller(this);
  silent_installer->init(empty_string, empty_string, CC_KVM);
  connect(silent_installer, &SilentInstaller::outputReceived, this,
          &CUpdaterComponentKvm::install_finished_sl);
  silent_installer->startWork();
  return CHUE_SUCCESS;
}

chue_t CUpdaterComponentKvm::update_internal() {
  update_progress_sl(100, 100);
  update_finished_sl(true);
  return CHUE_SUCCESS;
}

chue_t CUpdaterComponentKvm::uninstall_internal() {
  QMessageBox *msg_box = new QMessageBox(
      QMessageBox::Information, QObject::tr("Attention!"),
      QObject::tr(
          "Please stop all your running virtual machines and press \'Yes\" to continue."),
      QMessageBox::Yes | QMessageBox::No);
  if(msg_box->exec() != QMessageBox::Yes) {
    uninstall_finished_sl(false, tr("undefined"));
    return CHUE_FAILED;
  }

  static QString empty_string = "";

  SilentUninstaller *silent_uninstaller = new SilentUninstaller(this);
  silent_uninstaller->init(empty_string, empty_string, CC_KVM);

  connect(silent_uninstaller, &SilentUninstaller::outputReceived, this,
          &CUpdaterComponentKvm::uninstall_finished_sl);

  silent_uninstaller->startWork();

  return CHUE_SUCCESS;
}

void CUpdaterComponentKvm::update_post_action(bool success) {
  UNUSED_ARG(success);
}

void CUpdaterComponentKvm::install_post_internal(bool success) {
  if (!success)
    CNotificationObserver::Instance()->Error(
        tr("Failed to complete KVM installation. Try again later, "
           "or install it manually."),
        DlgNotification::N_NO_ACTION);
  else {
    CNotificationObserver::Instance()->Info(
        tr("KVM has been installed."), DlgNotification::N_NO_ACTION);
    QMessageBox msg_box;
    msg_box.setText("<b>You need to relogin</b> so that your user becomes "
                    "an effective member of the libvirtd group. "
                    "The members of this group can run virtual machines.");
    msg_box.exec();
  }
}

void CUpdaterComponentKvm::uninstall_post_internal(bool success) {
  if (!success)
    CNotificationObserver::Instance()->Error(
        tr("Failed to complete KVM uninstallation. Try again later, "
           "or uninstall it manually."),
        DlgNotification::N_NO_ACTION);
  else
    CNotificationObserver::Instance()->Info(
        tr("KVM has been uninstalled."), DlgNotification::N_NO_ACTION);
}
