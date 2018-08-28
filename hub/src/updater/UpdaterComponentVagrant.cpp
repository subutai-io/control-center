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
#include "updater/UpdaterComponentVagrant.h"

CUpdaterComponentVAGRANT::CUpdaterComponentVAGRANT() {
  m_component_id = VAGRANT;
}

CUpdaterComponentVAGRANT::~CUpdaterComponentVAGRANT() {}

QString CUpdaterComponentVAGRANT::download_vagrant_path() {
  QStringList lst_temp =
      QStandardPaths::standardLocations(QStandardPaths::TempLocation);
  return (lst_temp.isEmpty() ? QApplication::applicationDirPath()
                             : lst_temp[0]);
}

bool CUpdaterComponentVAGRANT::update_available_internal() {
  QString version;
  CSystemCallWrapper::vagrant_version(version);
  return version == "undefined";
}

chue_t CUpdaterComponentVAGRANT::install_internal() {
  QString version = "undefined";
  qDebug() << "Starting install vagrant";

  QMessageBox *msg_box = new QMessageBox(
      QMessageBox::Information, QObject::tr("Attention!"),
      QObject::tr(
          "<a href='https://www.vagrantup.com/intro/index.html'>Vagrant</a>"
          " is used to build and manage virtual machine environments.<br>"
          "Vagrant will be installed on your machine.<br>"
          "Do you want to proceed?"),
      QMessageBox::Yes | QMessageBox::No);
  msg_box->setTextFormat(Qt::RichText);

  QObject::connect(msg_box, &QMessageBox::finished, msg_box,
                   &QMessageBox::deleteLater);
  if (msg_box->exec() != QMessageBox::Yes) {
    install_finished_sl(false, version);
    return CHUE_SUCCESS;
  }
  QString file_name = vagrant_kurjun_package_name();
  QString file_dir = download_vagrant_path();
  QString str_vagrant_downloaded_path = file_dir + "/" + file_name;

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
      item->name(), str_vagrant_downloaded_path, item->size());

  SilentInstaller *silent_installer = new SilentInstaller(this);
  silent_installer->init(file_dir, file_name, CC_VAGRANT);

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
          &CUpdaterComponentVAGRANT::install_finished_sl);

  connect(silent_installer, &SilentInstaller::outputReceived, dm,
          &CDownloadFileManager::deleteLater);
  dm->start_download();
  return CHUE_SUCCESS;
}

chue_t CUpdaterComponentVAGRANT::update_internal() {
  update_progress_sl(100, 100);
  update_finished_sl(true);
  return CHUE_SUCCESS;
}

chue_t CUpdaterComponentVAGRANT::uninstall_internal() {
  qDebug() << "uninstall start vagrant";
  static QString empty_string = "";

  SilentUninstaller *silent_uninstaller = new SilentUninstaller(this);
  silent_uninstaller->init(empty_string, empty_string, CC_VAGRANT);

  connect(silent_uninstaller, &SilentUninstaller::outputReceived, this,
          &CUpdaterComponentVAGRANT::uninstall_finished_sl);

  silent_uninstaller->startWork();

  return CHUE_SUCCESS;
}

void CUpdaterComponentVAGRANT::update_post_action(bool success) {
  UNUSED_ARG(success);
}

void CUpdaterComponentVAGRANT::install_post_internal(bool success) {
  if (!success)
    CNotificationObserver::Instance()->Error(
        tr("Failed to complete Vagrant installation. Try again later, "
           "or install it manually."),
        DlgNotification::N_NO_ACTION);
  else
    CNotificationObserver::Instance()->Info(
        tr("Vagrant has been installed. You may now install the Vagrant "
           "plugins to complete your setup."),
        DlgNotification::N_NO_ACTION);
}

void CUpdaterComponentVAGRANT::uninstall_post_internal(bool success) {
  if (!success)
    CNotificationObserver::Instance()->Error(
        tr("Failed to complete Vagrant uninstallation. Try again later, "
           "or uninstall it manually."),
        DlgNotification::N_NO_ACTION);
  else
    CNotificationObserver::Instance()->Info(tr("Vagrant has been uninstalled"),
                                            DlgNotification::N_NO_ACTION);
}
