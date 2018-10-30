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
#include "updater/UpdaterComponentParallels.h"

CUpdaterComponentParallels::CUpdaterComponentParallels() {
  m_component_id = IUpdaterComponent::PARALLELS;
}

CUpdaterComponentParallels::~CUpdaterComponentParallels() {}

QString CUpdaterComponentParallels::download_parallels_path() {
  QStringList lst_temp =
      QStandardPaths::standardLocations(QStandardPaths::TempLocation);
  return (lst_temp.isEmpty() ? QApplication::applicationDirPath()
                             : lst_temp[0]);
}

bool CUpdaterComponentParallels::update_available_internal() {
  QString version;
  CSystemCallWrapper::parallels_version(version);
  return version == "undefined";
}

chue_t CUpdaterComponentParallels::install_internal() {
  qDebug() << "Starting install parallels";

  QMessageBox *msg_box = new QMessageBox(
      QMessageBox::Information, QObject::tr("Attention!"),
      QObject::tr(
          "<a href='https://www.parallels.com/'>Parallels</a>"
          " is used as the hypervisor.<br>"
          "Parallels Desktop will be installed on your machine.<br>"
          "Do you want to proceed?"),
      QMessageBox::Yes | QMessageBox::No);
  msg_box->setTextFormat(Qt::RichText);

  QObject::connect(msg_box, &QMessageBox::finished, msg_box,
                   &QMessageBox::deleteLater);
  if (msg_box->exec() != QMessageBox::Yes) {
    install_finished_sl(false, "undefined");
    return CHUE_SUCCESS;
  }
  QString file_name = parallels_kurjun_package_name();
  QString file_dir = download_parallels_path();
  QString file_downloaded_path = file_dir + QDir::separator() + file_name;

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
      item->id(), file_downloaded_path, item->size());
  dm->set_link(ipfs_download_url().arg(item->id(), item->name()));

  SilentInstaller *silent_installer = new SilentInstaller(this);
  silent_installer->init(file_dir, file_name, CC_PARALLELS);

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
          &CUpdaterComponentParallels::install_finished_sl);
  connect(silent_installer, &SilentInstaller::outputReceived, dm,
          &CDownloadFileManager::deleteLater);
  dm->start_download();
  return CHUE_SUCCESS;
}

chue_t CUpdaterComponentParallels::update_internal() {
  update_progress_sl(100, 100);
  update_finished_sl(true);
  return CHUE_SUCCESS;
}

chue_t CUpdaterComponentParallels::uninstall_internal() {
  QMessageBox *msg_box = new QMessageBox(
      QMessageBox::Information, QObject::tr("Attention!"),
      QObject::tr(
          "Please stop all your running virtual machines and press \'Yes\" to continue."),
      QMessageBox::Yes | QMessageBox::No);
  if(msg_box->exec() != QMessageBox::Yes) {
    uninstall_finished_sl(false, "undefined");
    return CHUE_FAILED;
  }

  static QString empty_string = "";
  SilentUninstaller *silent_uninstaller = new SilentUninstaller(this);
  silent_uninstaller->init(empty_string, empty_string, CC_PARALLELS);

  connect(silent_uninstaller, &SilentUninstaller::outputReceived, this,
          &CUpdaterComponentParallels::uninstall_finished_sl);

  silent_uninstaller->startWork();


  return CHUE_SUCCESS;
}

void CUpdaterComponentParallels::update_post_action(bool success) {
  UNUSED_ARG(success);
}

void CUpdaterComponentParallels::install_post_internal(bool success) {
  if (!success)
    CNotificationObserver::Instance()->Error(
        tr("Failed to complete Parallels Desktop installation. Try again later, "
           "or install it manually."),
        DlgNotification::N_NO_ACTION);
  else
    CNotificationObserver::Instance()->Info(
        tr("Parallels Desktop has been installed."), DlgNotification::N_NO_ACTION);
}

void CUpdaterComponentParallels::uninstall_post_internal(bool success) {
  if (!success)
    CNotificationObserver::Instance()->Error(
        tr("Failed to complete Parallels Desktop uninstallation. Try again later, "
           "or uninstall it manually."),
        DlgNotification::N_NO_ACTION);
  else
    CNotificationObserver::Instance()->Info(
        tr("Parallels Desktop has been uninstalled."), DlgNotification::N_NO_ACTION);
}
