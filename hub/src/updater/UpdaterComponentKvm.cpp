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

CUpdaterComponentKvm::CUpdaterComponentKvm() {
  m_component_id = KVM;
}

QString CUpdaterComponentKvm::download_script_path() {
  QStringList lst_temp =
      QStandardPaths::standardLocations(QStandardPaths::TempLocation);
  return (lst_temp.isEmpty() ? QApplication::applicationDirPath()
                             : lst_temp[0]);
}

CUpdaterComponentKvm::~CUpdaterComponentKvm() {}

bool CUpdaterComponentKvm::update_available_internal() {
  QString version;
  CSystemCallWrapper::hyperv_version(version);
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

  QString file_name = kvm_install_script_kurjun_name();
  QString file_dir = download_script_path();
  QString str_script_downloaded_path = file_dir + QDir::separator() + file_name;

  std::vector<CGorjunFileInfo> fi =
      CRestWorker::Instance()->get_gorjun_file_info(file_name);
  if (fi.empty()) {
    qCritical("File %s isn't presented on kurjun",
              m_component_id.toStdString().c_str());
    install_finished_sl(false, "undefined");
    return CHUE_NOT_ON_KURJUN;
  }
  std::vector<CGorjunFileInfo>::iterator item = fi.begin();

  CDownloadFileManager *dm = new CDownloadFileManager(
      item->name(), str_script_downloaded_path, item->size());
  dm->set_link(ipfs_download_url().arg(item->id(), item->name()));

  SilentInstaller *silent_installer = new SilentInstaller(this);
  silent_installer->init(file_dir, file_name, CC_KVM);

  connect(dm, &CDownloadFileManager::download_progress_sig,
          [this](qint64 rec, qint64 total) {
            update_progress_sl(rec, total);
          });
  connect(dm, &CDownloadFileManager::finished,
          [this, silent_installer](bool success) {
            if (!success) {
              silent_installer->outputReceived(success, "undefined");
            } else {
              this->update_progress_sl(0,0);
              CNotificationObserver::Instance()->Info(
                  tr("Running installation scripts might be take too long time please wait."),
                  DlgNotification::N_NO_ACTION);
              silent_installer->startWork();
            }
          });
  connect(silent_installer, &SilentInstaller::outputReceived, this,
          &CUpdaterComponentKvm::install_finished_sl);
  connect(silent_installer, &SilentInstaller::outputReceived, dm,
          &CDownloadFileManager::deleteLater);
  dm->start_download();
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
  else
    CNotificationObserver::Instance()->Info(
        tr("KVM has been installed."), DlgNotification::N_NO_ACTION);
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
