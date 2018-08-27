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
#include "updater/UpdaterComponentX2go.h"

CUpdaterComponentX2GO::CUpdaterComponentX2GO() { m_component_id = X2GO; }

CUpdaterComponentX2GO::~CUpdaterComponentX2GO() {}

QString CUpdaterComponentX2GO::download_x2go_path() {
  QStringList lst_temp =
      QStandardPaths::standardLocations(QStandardPaths::TempLocation);
  return (lst_temp.isEmpty() ? QApplication::applicationDirPath()
                             : lst_temp[0]);
}

bool CUpdaterComponentX2GO::update_available_internal() {
  QString version;
  CSystemCallWrapper::x2go_version(version);
  return version == "undefined";
}

chue_t CUpdaterComponentX2GO::install_internal() {
  QString version = "undefined";
  qDebug() << "Starting install x2go";

  QMessageBox *msg_box = new QMessageBox(
      QMessageBox::Information, QObject::tr("Attention!"),
      QObject::tr(
          "<a href='https://wiki.x2go.org/doku.php/doc:usage:x2goclient'>X2Go "
          "client</a>"
          " enables remote desktop access.<br>"
          "The X2Go client will be installed on your machine.<br>"
          "Do you want to proceed?"),
      QMessageBox::Yes | QMessageBox::No);
  msg_box->setTextFormat(Qt::RichText);

  QObject::connect(msg_box, &QMessageBox::finished, msg_box,
                   &QMessageBox::deleteLater);
  if (msg_box->exec() != QMessageBox::Yes) {
    install_finished_sl(false, version);
    return CHUE_SUCCESS;
  }

  QStringList lst_temp =
      QStandardPaths::standardLocations(QStandardPaths::TempLocation);
  QString file_name = x2go_kurjun_package_name();
  QString file_dir = download_x2go_path();
  QString str_x2go_downloaded_path = file_dir + "/" + file_name;

  std::vector<CGorjunFileInfo> fi =
      CRestWorker::Instance()->get_gorjun_file_info(file_name);

  if (fi.empty()) {
    qCritical("File %s isn't presented on kurjun",
              m_component_id.toStdString().c_str());
    install_finished_sl(false, version);
    return CHUE_NOT_ON_KURJUN;
  }

  std::vector<CGorjunFileInfo>::iterator item = fi.begin();

  CDownloadFileManager *dm = new CDownloadFileManager(
      item->id(), str_x2go_downloaded_path, item->size());

  SilentInstaller *silent_installer = new SilentInstaller(this);
  silent_installer->init(file_dir, file_name, CC_X2GO);

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
                  tr("Running installation scripts."),
                  DlgNotification::N_NO_ACTION);
              silent_installer->startWork();
            }
          });

  connect(silent_installer, &SilentInstaller::outputReceived, this,
          &CUpdaterComponentX2GO::install_finished_sl);

  connect(silent_installer, &SilentInstaller::outputReceived, dm,
          &CDownloadFileManager::deleteLater);
  dm->start_download();
  return CHUE_SUCCESS;
}

chue_t CUpdaterComponentX2GO::update_internal() {
  update_progress_sl(100, 100);
  update_finished_sl(true);
  return CHUE_SUCCESS;
}

chue_t CUpdaterComponentX2GO::uninstall_internal() {
  static QString empty_string = "";

  SilentUninstaller *silent_uninstaller = new SilentUninstaller(this);
  silent_uninstaller->init(empty_string, empty_string, CC_X2GO);

  connect(silent_uninstaller, &SilentUninstaller::outputReceived, this,
          &CUpdaterComponentX2GO::uninstall_finished_sl);

  silent_uninstaller->startWork();

  return CHUE_SUCCESS;
}

void CUpdaterComponentX2GO::update_post_action(bool success) {
  UNUSED_ARG(success);
}

void CUpdaterComponentX2GO::install_post_internal(bool success) {
  if (!success)
    CNotificationObserver::Instance()->Error(
        tr("Failed to complete X2Go-Client installation. Try again later, "
           "or install it manually."),
        DlgNotification::N_NO_ACTION);
  else
    CNotificationObserver::Instance()->Info(
        tr("The X2Go-Client has been installed."),
        DlgNotification::N_NO_ACTION);
}

void CUpdaterComponentX2GO::uninstall_post_internal(bool success) {

  if (!success)
    CNotificationObserver::Instance()->Error(
        tr("Failed to complete X2Go uninstallation. Try again later, "
           "or uninstall it manually."),
        DlgNotification::N_NO_ACTION);
  else
    CNotificationObserver::Instance()->Info(tr("X2Go has been uninstalled"),
                                            DlgNotification::N_NO_ACTION);
}
