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
#include "updater/UpdaterComponentVirtualbox.h"

CUpdaterComponentVIRTUALBOX::CUpdaterComponentVIRTUALBOX() {
  m_component_id = ORACLE_VIRTUALBOX;
}

CUpdaterComponentVIRTUALBOX::~CUpdaterComponentVIRTUALBOX() {}

QString CUpdaterComponentVIRTUALBOX::download_virtualbox_path() {
  QStringList lst_temp =
      QStandardPaths::standardLocations(QStandardPaths::TempLocation);
  return (lst_temp.isEmpty() ? QApplication::applicationDirPath()
                             : lst_temp[0]);
}

bool CUpdaterComponentVIRTUALBOX::update_available_internal() {
  QString version;
  CSystemCallWrapper::oracle_virtualbox_version(version);
  return version == "undefined";
}

chue_t CUpdaterComponentVIRTUALBOX::install_internal() {
  QString version = "undefined";
  qDebug() << "Starting install oracle virtualbox";

  QMessageBox *msg_box = new QMessageBox(
      QMessageBox::Information, QObject::tr("Attention!"),
      QObject::tr(
          "<a href='https://www.virtualbox.org/wiki/VirtualBox'>VirtualBox</a>"
          " is used as the default hypervisor.<br>"
          "VirtualBox will be installed on your machine.<br>"
          "Do you want to proceed?"),
      QMessageBox::Yes | QMessageBox::No);
  msg_box->setTextFormat(Qt::RichText);

  QObject::connect(msg_box, &QMessageBox::finished, msg_box,
                   &QMessageBox::deleteLater);
  if (msg_box->exec() != QMessageBox::Yes) {
    install_finished_sl(false, version);
    return CHUE_SUCCESS;
  }
  QString file_name = oracle_virtualbox_kurjun_package_name();
  QString file_dir = download_virtualbox_path();
  QString str_oracle_virtualbox_downloaded_path = file_dir + "/" + file_name;

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
      item->name(), str_oracle_virtualbox_downloaded_path, item->size());

  SilentInstaller *silent_installer = new SilentInstaller(this);
  silent_installer->init(file_dir, file_name, CC_VB);
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
          &CUpdaterComponentVIRTUALBOX::install_finished_sl);
  connect(silent_installer, &SilentInstaller::outputReceived, dm,
          &CDownloadFileManager::deleteLater);
  dm->start_download();
  return CHUE_SUCCESS;
}

chue_t CUpdaterComponentVIRTUALBOX::update_internal() {
  update_progress_sl(100, 100);
  update_finished_sl(true);
  return CHUE_SUCCESS;
}

chue_t CUpdaterComponentVIRTUALBOX::uninstall_internal() {
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
  if (CURRENT_OS == OS_MAC) {
    QString file_name = "VirtualBox_Uninstall.tool";
    QString file_dir = download_virtualbox_path();
    QString str_oracle_virtualbox_downloaded_path = file_dir + QDir::separator() + file_name;

    std::vector<CGorjunFileInfo> fi =
        CRestWorker::Instance()->get_gorjun_file_info(
          file_name, "https://bazaar.subutai.io/rest/v1/cdn/raw");
    if (fi.empty()) {
      qCritical("File %s isn't presented on kurjun",
                m_component_id.toStdString().c_str());
      install_finished_sl(false, "undefined");
      return CHUE_NOT_ON_KURJUN;
    }
    std::vector<CGorjunFileInfo>::iterator item = fi.begin();

    CDownloadFileManager *dm = new CDownloadFileManager(
        item->name(), str_oracle_virtualbox_downloaded_path, item->size());
    dm->set_link("https://bazaar.subutai.io/rest/v1/cdn/raw");

    SilentUninstaller *silent_uninstaller = new SilentUninstaller(this);
    silent_uninstaller->init(file_dir, file_name, CC_VB);
    connect(dm, &CDownloadFileManager::download_progress_sig,
            [this](qint64 rec, qint64 total) {
              update_progress_sl(rec, total);
            });
    connect(dm, &CDownloadFileManager::finished,
            [this, silent_uninstaller](bool success) {
              if (!success) {
                silent_uninstaller->outputReceived(success, tr("undefined"));
              } else {
                this->update_progress_sl(0,0);
                silent_uninstaller->startWork();
              }
            });
    connect(silent_uninstaller, &SilentUninstaller::outputReceived, this,
            &CUpdaterComponentVIRTUALBOX::uninstall_finished_sl);
    connect(silent_uninstaller, &SilentUninstaller::outputReceived, dm,
            &CDownloadFileManager::deleteLater);
    dm->start_download();
    return CHUE_SUCCESS;
  } else {
    SilentUninstaller *silent_uninstaller = new SilentUninstaller(this);
    silent_uninstaller->init(empty_string, empty_string, CC_VB);

    connect(silent_uninstaller, &SilentUninstaller::outputReceived, this,
            &CUpdaterComponentVIRTUALBOX::uninstall_finished_sl);

    silent_uninstaller->startWork();
  }

  return CHUE_SUCCESS;
}

void CUpdaterComponentVIRTUALBOX::update_post_action(bool success) {
  UNUSED_ARG(success);
}

void CUpdaterComponentVIRTUALBOX::install_post_internal(bool success) {
  if (!success)
    CNotificationObserver::Instance()->Error(
        tr("Failed to complete VirtualBox installation. Try again later, "
           "or install it manually."),
        DlgNotification::N_NO_ACTION);
  else
    CNotificationObserver::Instance()->Info(
        tr("Virtualbox has been installed."), DlgNotification::N_NO_ACTION);
}

void CUpdaterComponentVIRTUALBOX::uninstall_post_internal(bool success) {
  if (!success)
    CNotificationObserver::Instance()->Error(
        tr("Failed to complete VirtualBox uninstallation. Try again later, "
           "or uninstall it manually."),
        DlgNotification::N_NO_ACTION);
  else
    CNotificationObserver::Instance()->Info(
        tr("Virtualbox has been uninstalled."), DlgNotification::N_NO_ACTION);
}
