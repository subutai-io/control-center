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
#include "updater/HubComponentsUpdater.h"
#include "updater/UpdaterComponentXQuartz.h"

CUpdaterComponentXQuartz::CUpdaterComponentXQuartz() { m_component_id = XQUARTZ; }

CUpdaterComponentXQuartz::~CUpdaterComponentXQuartz() {}

QString CUpdaterComponentXQuartz::download_xquartz_path() {
  QStringList lst_temp =
      QStandardPaths::standardLocations(QStandardPaths::TempLocation);
  return (lst_temp.isEmpty() ? QApplication::applicationDirPath()
                             : lst_temp[0]);
}

bool CUpdaterComponentXQuartz::update_available_internal() {
  QString version;
  CSystemCallWrapper::xquartz_version(version);
  return version == "undefined";
}

chue_t CUpdaterComponentXQuartz::install_internal() {
  qDebug() << "Starting install xquart";
  // show messagebox with info about component
  QMessageBox *msg_box = new QMessageBox(
      QMessageBox::Information, QObject::tr("Attention!"),
      QObject::tr(
          "<a href='https://www.xquartz.org'>XQuartz</a>"
          " is used by X2Go for remote desktop connection.<br>"
          "XQuartz will be installed on your machine.<br>"
          "Do you want to proceed?"),
      QMessageBox::Yes | QMessageBox::No);
  msg_box->setTextFormat(Qt::RichText);

  QObject::connect(msg_box, &QMessageBox::finished, msg_box,
                   &QMessageBox::deleteLater);
  if (msg_box->exec() != QMessageBox::Yes) {
    install_finished_sl(false, "undefined");
    return CHUE_SUCCESS;
  }
  // start installation
  QString file_name = xquartz_kurjun_package_name();
  QString file_dir = download_xquartz_path();
  QString str_xquartz_downloaded_path = file_dir + "/" + file_name;

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
      item->name(), str_xquartz_downloaded_path, item->size());
  dm->set_link(ipfs_download_url().arg(item->id(), item->name()));

  SilentInstaller *silent_installer = new SilentInstaller(this);
  silent_installer->init(file_dir, file_name, CC_XQUARTZ);

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
          &CUpdaterComponentXQuartz::install_finished_sl);

  connect(silent_installer, &SilentInstaller::outputReceived, dm,
          &CDownloadFileManager::deleteLater);
  dm->start_download();
  return CHUE_SUCCESS;
}

chue_t CUpdaterComponentXQuartz::update_internal() {
  update_progress_sl(100, 100);
  update_finished_sl(true);
  return CHUE_SUCCESS;
}

chue_t CUpdaterComponentXQuartz::uninstall_internal() {
  qDebug() << "uninstall start xquartz";
  static QString empty_string = "";

  SilentUninstaller *silent_uninstaller = new SilentUninstaller(this);
  silent_uninstaller->init(empty_string, empty_string, CC_XQUARTZ);

  connect(silent_uninstaller, &SilentUninstaller::outputReceived, this,
          &CUpdaterComponentXQuartz::uninstall_finished_sl);

  silent_uninstaller->startWork();

  return CHUE_SUCCESS;
}

void CUpdaterComponentXQuartz::update_post_action(bool success) {
  UNUSED_ARG(success);
}

void CUpdaterComponentXQuartz::install_post_internal(bool success) {
  if (!success)
    CNotificationObserver::Instance()->Error(
        tr("Failed to complete XQuartz installation. Try again later, "
           "or install it manually."),
        DlgNotification::N_NO_ACTION);
  else
    CNotificationObserver::Instance()->Info(
        tr("The XQuartz has been installed."),
        DlgNotification::N_NO_ACTION);
}

void CUpdaterComponentXQuartz::uninstall_post_internal(bool success) {
  if (!success)
    CNotificationObserver::Instance()->Error(
        tr("Failed to complete XQuartz uninstallation. Try again later, "
           "or uninstall it manually."),
        DlgNotification::N_NO_ACTION);
  else
    CNotificationObserver::Instance()->Info(tr("XQuartz has been uninstalled"),
                                            DlgNotification::N_NO_ACTION);
}
