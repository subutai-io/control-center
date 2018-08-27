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
#include "TrayControlWindow.h"
#include "updater/ExecutableUpdater.h"
#include "updater/HubComponentsUpdater.h"
#include "updater/UpdaterComponentSubutaiBox.h"

CUpdaterComponentSUBUTAI_BOX::CUpdaterComponentSUBUTAI_BOX() {
  m_component_id = SUBUTAI_BOX;
}

CUpdaterComponentSUBUTAI_BOX::~CUpdaterComponentSUBUTAI_BOX() {}

QString CUpdaterComponentSUBUTAI_BOX::download_subutai_box_path() {
  QStringList lst_temp =
      QStandardPaths::standardLocations(QStandardPaths::TempLocation);
  return (lst_temp.isEmpty() ? QApplication::applicationDirPath()
                             : lst_temp[0]);
}

bool CUpdaterComponentSUBUTAI_BOX::update_available_internal() {
  QString version;
  QString subutai_box = subutai_box_name();
  QString subutai_provider = VagrantProvider::Instance()->CurrentVal();
  system_call_wrapper_error_t res =
      CSystemCallWrapper::vagrant_latest_box_version(subutai_box,
                                                     subutai_provider, version);
  QString cloud_version =
      CRestWorker::Instance()->get_vagrant_box_cloud_version(subutai_box,
                                                             subutai_provider);
  qDebug() << "subutai box version " << version << " vs "
           << "subutai box cloud version " << cloud_version;
  if (version == "undefined") return true;
  if (res != SCWE_SUCCESS) return false;
  if (cloud_version == "undefined" || cloud_version.isEmpty()) return false;
  return cloud_version > version;
}
chue_t CUpdaterComponentSUBUTAI_BOX::install_internal(){
  return CUpdaterComponentSUBUTAI_BOX::install_internal(false);
}

chue_t CUpdaterComponentSUBUTAI_BOX::install_internal(bool update){
  qDebug() << "Starting install new version of subutai box";

  QString version;
  QString subutai_box = subutai_box_name();
  QString subutai_provider = VagrantProvider::Instance()->CurrentVal();

  CSystemCallWrapper::vagrant_latest_box_version(subutai_box, subutai_provider,
                                                 version);
  if (version == "undefined") {
    QMessageBox *msg_box = new QMessageBox(
        QMessageBox::Information, QObject::tr("Attention!"),
        QObject::tr(
            "<a href='https://app.vagrantup.com/subutai/boxes/stretch'>Subutai "
            "Box</a>"
            " is the resource box for peer creation.<br>"
            "Subutai Box will be installed on your machine.<br>"
            "Do you want to proceed?"),
        QMessageBox::Yes | QMessageBox::No);
    msg_box->setTextFormat(Qt::RichText);

    QObject::connect(msg_box, &QMessageBox::finished, msg_box,
                     &QMessageBox::deleteLater);
    if (msg_box->exec() != QMessageBox::Yes) {
      install_finished_sl(false, "undefined");
      return CHUE_SUCCESS;
    }
  }

  QString file_name = subutai_box_kurjun_package_name(subutai_provider);
  QString file_dir = download_subutai_box_path();
  QString str_downloaded_path = file_dir + "/" + file_name;

  std::vector<CGorjunFileInfo> fi =
      CRestWorker::Instance()->get_gorjun_file_info(file_name);
  if (fi.empty()) {
    qCritical("File %s isn't presented on kurjun",
              m_component_id.toStdString().c_str());
    install_finished_sl(false, "undefined");
    return CHUE_NOT_ON_KURJUN;
  }
  std::vector<CGorjunFileInfo>::iterator item = fi.begin();

  CDownloadFileManager *dm =
      new CDownloadFileManager(item->id(), str_downloaded_path, item->size());

  SilentInstaller *silent_installer = new SilentInstaller(this);
  silent_installer->init(file_dir, file_name, CC_SUBUTAI_BOX);

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
  if (update) {
    connect(silent_installer, &SilentInstaller::outputReceived,
            this, &CUpdaterComponentSUBUTAI_BOX::update_finished_sl);
  } else {
    connect(silent_installer, &SilentInstaller::outputReceived,
            this, &CUpdaterComponentSUBUTAI_BOX::install_finished_sl);
  }
  connect(silent_installer, &SilentInstaller::outputReceived, dm,
          &CDownloadFileManager::deleteLater);

  dm->start_download();
  return CHUE_SUCCESS;
}

chue_t CUpdaterComponentSUBUTAI_BOX::uninstall_internal() {
  if (!CCommons::IsVagrantVMwareLicenseInstalled()) {
    CCommons::InfoVagrantVMwareLicense();
    emit uninstall_finished_sl(false, tr("undefined"));
    return CHUE_SUCCESS;
  }

  size_t total = TrayControlWindow::Instance()->machine_peers_table.size();
  qDebug() << "Total machine peers: " << total;

  // while removing vagrant box. we should check existing vagrant machines. it
  // will corrupt existing machines.
  if (total > 0) {
    QMessageBox *msg_box = new QMessageBox(
        QMessageBox::Information, QObject::tr("Attention!"),
        QObject::tr(
            "Removing the Subutai box could corrupt the peers. "
            "We recommend destroying peers first from \"My Peers\" menu.\n"
            "Do you want to proceed?"),
        QMessageBox::Yes | QMessageBox::No);
    msg_box->setTextFormat(Qt::RichText);

    QObject::connect(msg_box, &QMessageBox::finished, msg_box,
                     &QMessageBox::deleteLater);
    if (msg_box->exec() != QMessageBox::Yes) {
      emit uninstall_finished_sl(false, tr("undefined"));
      return CHUE_SUCCESS;
    }
  }
  static QString empty_string = "";

  SilentUninstaller *silent_uninstaller = new SilentUninstaller(this);
  silent_uninstaller->init(empty_string, empty_string, CC_SUBUTAI_BOX);

  connect(silent_uninstaller, &SilentUninstaller::outputReceived, this,
          &CUpdaterComponentSUBUTAI_BOX::uninstall_finished_sl);

  silent_uninstaller->startWork();

  return CHUE_SUCCESS;
}

chue_t CUpdaterComponentSUBUTAI_BOX::update_internal() {
  return install_internal(true);
}

void CUpdaterComponentSUBUTAI_BOX::update_post_action(bool success) {
  if (!success) {
    CNotificationObserver::Instance()->Error(
        tr("Vagrant Subutai box failed to update"), DlgNotification::N_ABOUT);
  } else {
    CNotificationObserver::Instance()->Info(
        tr("Succesfully updated Vagrant Subutai box"),
        DlgNotification::N_ABOUT);
  }
}

void CUpdaterComponentSUBUTAI_BOX::install_post_internal(bool success) {
  if (!success) {
    CNotificationObserver::Instance()->Error(
        tr("Vagrant Subutai box installation has failed. Make sure you don't have peers on your machine and try again."),
        DlgNotification::N_NO_ACTION);
  } else {
    CNotificationObserver::Instance()->Info(
        tr("Succesfully installed the Vagrant Subutai box"),
        DlgNotification::N_NO_ACTION);
  }
}

void CUpdaterComponentSUBUTAI_BOX::uninstall_post_internal(bool success) {
  if (!success) {
    CNotificationObserver::Instance()->Error(
        tr("Vagrant Subutai box deletion has failed."),
        DlgNotification::N_NO_ACTION);
  } else {
    CNotificationObserver::Instance()->Info(
        tr("Succesfully deleted the Vagrant Subutai box"),
        DlgNotification::N_NO_ACTION);
  }
}
