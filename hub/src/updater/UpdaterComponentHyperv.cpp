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
#include "updater/UpdaterComponentHyperv.h"

CUpdaterComponentHyperv::CUpdaterComponentHyperv() {
  m_component_id = HYPERV;
}

CUpdaterComponentHyperv::~CUpdaterComponentHyperv() {}

bool CUpdaterComponentHyperv::update_available_internal() {
  QString version;
  CSystemCallWrapper::hyperv_version(version);
  return version == "undefined";
}

chue_t CUpdaterComponentHyperv::install_internal() {
  qDebug() << "Starting enable hyperv";

  QMessageBox *msg_box = new QMessageBox(
      QMessageBox::Information, QObject::tr("Attention!"),
      QObject::tr(
          "Hyper-V is used as the hypervisor.<br>"
          "The Hyper-V will be enabling on your machine.\n"
          "<b>After enabling the Hyper-V, "
          "it is required to reboot machine.</b>\n"
          "Do you want to proceed?"),
      QMessageBox::Yes | QMessageBox::No);

  QObject::connect(msg_box, &QMessageBox::finished, msg_box,
                   &QMessageBox::deleteLater);
  if (msg_box->exec() != QMessageBox::Yes) {
    install_finished_sl(false, "undefined");
    return CHUE_SUCCESS;
  }

  update_progress_sl(0, 0);  // imitation of progress bar :D, todo implement
  static QString empty_string = "";
  SilentInstaller *silent_installer = new SilentInstaller(this);
  silent_installer->init(empty_string, empty_string, CC_HYPERV);
  connect(silent_installer, &SilentInstaller::outputReceived, this,
          &CUpdaterComponentHyperv::install_finished_sl);
  silent_installer->startWork();
  return CHUE_SUCCESS;
}

chue_t CUpdaterComponentHyperv::update_internal() {
  update_progress_sl(100, 100);
  update_finished_sl(true);
  return CHUE_SUCCESS;
}

chue_t CUpdaterComponentHyperv::uninstall_internal() {
  QMessageBox *msg_box = new QMessageBox(
      QMessageBox::Information, QObject::tr("Attention!"),
      QObject::tr(
          "The <br>Hyper-V</br> will be disabling on your machine.\n"
          "After disabling the Hyper-V, <b>it is required to reboot machine.</b>\n"
          "Do you want to proceed?"),
      QMessageBox::Yes | QMessageBox::No);

  QObject::connect(msg_box, &QMessageBox::finished, msg_box,
                   &QMessageBox::deleteLater);
  if (msg_box->exec() != QMessageBox::Yes) {
    install_finished_sl(false, "undefined");
    return CHUE_SUCCESS;
  }

  static QString empty_string = "";

  SilentUninstaller *silent_uninstaller = new SilentUninstaller(this);
  silent_uninstaller->init(empty_string, empty_string, CC_HYPERV);

  connect(silent_uninstaller, &SilentUninstaller::outputReceived, this,
          &CUpdaterComponentHyperv::uninstall_finished_sl);

  silent_uninstaller->startWork();

  return CHUE_SUCCESS;
}

void CUpdaterComponentHyperv::update_post_action(bool success) {
  UNUSED_ARG(success);
}
void CUpdaterComponentHyperv::install_post_internal(bool success) {
  if (!success) {
    CNotificationObserver::Instance()->Info(
        tr("Failed to enable the Hyper-V. You may try manually "
           "enable it "
           "or try again by restarting the Control Center first."),
        DlgNotification::N_NO_ACTION);
  } else {
    CNotificationObserver::Instance()->Info(
        tr("Hyper-V has been enabled successfully."),
        DlgNotification::N_NO_ACTION);
  }
}

void CUpdaterComponentHyperv::uninstall_post_internal(bool success) {
  if (!success) {
    CNotificationObserver::Instance()->Info(
        tr("Failed to disable the Hyper-V. You may try "
           "manually disable it "
           "or try again by restarting the Control Center first."),
        DlgNotification::N_NO_ACTION);
  } else {
    CNotificationObserver::Instance()->Info(
        tr("Hyper-V has been disabled successfully."),
        DlgNotification::N_NO_ACTION);
  }
}
