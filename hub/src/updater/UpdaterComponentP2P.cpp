#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>

#include "updater/UpdaterComponentP2P.h"
#include "updater/ExecutableUpdater.h"
#include "RestWorker.h"
#include "SystemCallWrapper.h"
#include "NotificationObserver.h"
#include "DownloadFileManager.h"
#include "Commons.h"
#include "updater/HubComponentsUpdater.h"
#include "OsBranchConsts.h"
#include "P2PController.h"

using namespace update_system;

CUpdaterComponentP2P::CUpdaterComponentP2P() {
  m_component_id = P2P;
}

CUpdaterComponentP2P::~CUpdaterComponentP2P() {

}
////////////////////////////////////////////////////////////////////////////

QString
CUpdaterComponentP2P::p2p_path()
{
  QString p2p_path = P2P;

  if (CSettingsManager::Instance().p2p_path() != P2P) {
    p2p_path = CSettingsManager::Instance().p2p_path();
  } else {
    system_call_wrapper_error_t cr;
    if ((cr = CSystemCallWrapper::which(P2P, p2p_path)) != SCWE_SUCCESS) {
      CNotificationObserver::Instance()->Error(tr("Unable to find the P2P Daemon: %1. Make sure that it is installed "
                                                  "and PATH environment variable contains P2P's directory.").arg(
                                                            CSystemCallWrapper::scwe_error_to_str(cr)), DlgNotification::N_SETTINGS);
    }
  }
  QFileInfo checkFile(p2p_path);
  if (checkFile.exists() && checkFile.isSymLink()) {
    p2p_path = QFile::symLinkTarget(p2p_path);
  }

  if(!checkFile.exists())
      p2p_path = "Not found";
  return p2p_path;
}
////////////////////////////////////////////////////////////////////////////

QString
CUpdaterComponentP2P::download_p2p_path() {
  QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
  return (lst_temp.isEmpty() ? QApplication::applicationDirPath() : lst_temp[0]);
}
////////////////////////////////////////////////////////////////////////////

bool
CUpdaterComponentP2P::update_available_internal() {
  std::vector<CGorjunFileInfo> fi =
      CRestWorker::Instance()->get_gorjun_file_info(p2p_kurjun_file_name());
  if (fi.empty()) return false;
  QString str_p2p_path = p2p_path();
  if (str_p2p_path == P2P) return false;
  QString md5_current = CCommons::FileMd5(str_p2p_path);
  QString md5_kurjun = fi[0].md5_sum();
  qDebug()
          <<"Checking for p2p update"
          <<"md5_current: "<<md5_current
          <<"md5_kurjun: "<<md5_kurjun;
  return md5_current != md5_kurjun;
}
////////////////////////////////////////////////////////////////////////////
chue_t CUpdaterComponentP2P::install_internal(){
    qDebug()
            << "Starting install P2P";

    QMessageBox *msg_box = new QMessageBox(
          QMessageBox::Information, QObject::tr("Attention!"), QObject::tr(
            "<a href='https://subutai.io/getting-started.html#P2P'>Subutai P2P</a>"
            " handles the connection between peers and environments.<br>"
            "Subutai P2P will be installed on your machine.<br>"
            "Do you want to proceed?"), QMessageBox::Yes | QMessageBox::No);
    msg_box->setTextFormat(Qt::RichText);

    QObject::connect(msg_box, &QMessageBox::finished, msg_box, &QMessageBox::deleteLater);
    if (msg_box->exec() != QMessageBox::Yes) {
        install_finished_sl(false);
        return CHUE_SUCCESS;
    }

    QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
    QString file_name = p2p_kurjun_package_name();
    QString file_dir = download_p2p_path();
    QString str_p2p_downloaded_path = file_dir + "/" + file_name;

    std::vector<CGorjunFileInfo> fi = CRestWorker::Instance()->get_gorjun_file_info(p2p_kurjun_package_name());
    if (fi.empty()) {
      qCritical("File %s isn't presented on kurjun", m_component_id.toStdString().c_str());
      return CHUE_NOT_ON_KURJUN;
    }
    std::vector<CGorjunFileInfo>::iterator item = fi.begin();

    CDownloadFileManager *dm = new CDownloadFileManager(item->id(),
                                                        str_p2p_downloaded_path,
                                                        item->size());

    SilentInstaller *silent_installer = new SilentInstaller(this);
    silent_installer->init(file_dir, file_name, CC_P2P);
    connect(dm, &CDownloadFileManager::download_progress_sig,
            [this](qint64 rec, qint64 total){update_progress_sl(rec, total+(total/5));});
    connect(dm, &CDownloadFileManager::finished,[silent_installer](){
        silent_installer->startWork();
    });
    connect(silent_installer, &SilentInstaller::outputReceived,
            this, &CUpdaterComponentP2P::install_finished_sl);
    connect(silent_installer, &SilentInstaller::outputReceived,
            dm, &CDownloadFileManager::deleteLater);
    dm->start_download();
    return CHUE_SUCCESS;
}

chue_t
CUpdaterComponentP2P::update_internal() {
  qDebug() << "Starting to update P2P";
  QString str_p2p_path = p2p_path();
  if(str_p2p_path == "Not found"){
      CNotificationObserver::Instance()->Error(tr("To continue, you must install the P2P Daemon first."), DlgNotification::N_INSTALL_P2P);
      return CHUE_FAILED;
  }
  if (str_p2p_path.isNull() ||
      str_p2p_path.isEmpty() ||
      str_p2p_path == P2P) {
    qCritical("Update p2p failed. Path = %s",
                                          (str_p2p_path.isNull() || str_p2p_path.isEmpty() ?
                                             "empty" : str_p2p_path.toStdString().c_str()));
    return CHUE_FAILED;
  }

  //original file path
  QString str_p2p_executable_path = p2p_path();

  //this file will replace original file
  QString file_name = P2P;
  QString file_dir = download_p2p_path();
  QString str_p2p_downloaded_path = file_dir + "/" + file_name;

  std::vector<CGorjunFileInfo> fi = CRestWorker::Instance()->get_gorjun_file_info(
                                      p2p_kurjun_file_name());

  if (fi.empty()) {
    qCritical("File %s isn't presented on kurjun", m_component_id.toStdString().c_str());
    return CHUE_NOT_ON_KURJUN;
  }

  std::vector<CGorjunFileInfo>::iterator item = fi.begin();


  CExecutableUpdater *eu = new CExecutableUpdater(str_p2p_downloaded_path,
                                                str_p2p_executable_path);

  if (item->md5_sum() == CCommons::FileMd5(str_p2p_downloaded_path))
  {
    qInfo("Already have new version of p2p in %s",
                str_p2p_downloaded_path.toStdString().c_str());

    this->update_progress_sl(100, 100);
    connect(eu, &CExecutableUpdater::finished,
            this, &CUpdaterComponentP2P::update_finished_sl);
    connect(eu, &CExecutableUpdater::finished,
            eu, &CExecutableUpdater::deleteLater);
    eu->replace_executables(true);
    return CHUE_SUCCESS;
  }

  CDownloadFileManager *dm = new CDownloadFileManager(item->id(),
                                                      str_p2p_downloaded_path,
                                                      item->size());
  connect(dm, &CDownloadFileManager::download_progress_sig,
          this, &CUpdaterComponentP2P::update_progress_sl);
  connect(dm, &CDownloadFileManager::finished,
          eu, &CExecutableUpdater::replace_executables);
  connect(eu, &CExecutableUpdater::finished,
          this, &CUpdaterComponentP2P::update_finished_sl);
  connect(eu, &CExecutableUpdater::finished,
          dm, &CDownloadFileManager::deleteLater);
  connect(eu, &CExecutableUpdater::finished,
          eu, &CExecutableUpdater::deleteLater);
  dm->start_download();
  return CHUE_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////

void
CUpdaterComponentP2P::update_post_action(bool success) {
  if (!success) {
    CNotificationObserver::Instance()->Error(tr("Failed to update the P2P Daemon. Make sure that you have the required permissions."), DlgNotification::N_SETTINGS);
    return;
  }

  //P2PController::Instance().p2p_restart();

  CNotificationObserver::Instance()->Info(tr("P2P has been updated"), DlgNotification::N_NO_ACTION);
  int rse_err = 0;

  system_call_wrapper_error_t scwe =
      CSystemCallWrapper::restart_p2p_service(&rse_err, restart_p2p_type::UPDATED_P2P);

  if (scwe != SCWE_SUCCESS) {
    CNotificationObserver::Instance()->Error(tr("p2p post update failed. err : %1").
                                                        arg(CSystemCallWrapper::scwe_error_to_str(scwe)), DlgNotification::N_NO_ACTION);
    return;
  }

  if (rse_err == RSE_MANUAL) {
    QMessageBox *msg_box = new QMessageBox(QMessageBox::Question, tr("Successfully updated the P2P Daemon."),
                                           tr("The P2P Daemon has been updated. Restart P2P Daemon, please."),
                                           QMessageBox::Ok);
    connect(msg_box, &QMessageBox::finished, msg_box, &QMessageBox::deleteLater);
    msg_box->exec();
  }
  // additional post update scripts
  static QString cloud_version = "neoMatrix";
  PostUpdater *post_scripts = new PostUpdater(this);
  post_scripts->start_work();
}

void CUpdaterComponentP2P::install_post_internal(bool success){
    if(success){
        CNotificationObserver::Info(tr("The P2P Daemon has been installed and will start running in a few seconds."), DlgNotification::N_NO_ACTION);
    }
    else{
        CNotificationObserver::Error(tr("Failed to install the P2P Daemon. You may try installing directly through the link under "
                                        "<a href='https://subutai.io/getting-started.html#P2P'>Getting Started.</a>"), DlgNotification::N_NO_ACTION);
    }
}
////////////////////////////////////////////////////////////////////////////

chue_t CUpdaterComponentP2P::uninstall_internal() {
  static QString empty_string = "";

  SilentUninstaller *silent_uninstaller = new SilentUninstaller(this);
  silent_uninstaller->init(empty_string, empty_string, CC_P2P);

  connect(silent_uninstaller, &SilentUninstaller::outputReceived,
          this, &CUpdaterComponentP2P::uninstall_finished_sl);

  silent_uninstaller->startWork();

  return CHUE_SUCCESS;
}

void CUpdaterComponentP2P::uninstall_post_internal(bool success) {
  if(success){
      CNotificationObserver::Info(tr("The P2P Daemon has been uninstalled."), DlgNotification::N_NO_ACTION);
  }
  else{
      CNotificationObserver::Error(tr("Failed to uninstall the P2P Daemon."), DlgNotification::N_NO_ACTION);
  }
}

void PostUpdater::start_work(){
    QThread* thread = new QThread();
    connect(thread, &QThread::started,
            this, &PostUpdater::post_update);
    connect(this, &PostUpdater::output_received,
            thread, &QThread::quit);
    connect(thread, &QThread::finished,
            this, &PostUpdater::deleteLater);
    connect(thread, &QThread::finished,
            thread, &QThread::deleteLater);
    this->moveToThread(thread);
    thread->start();
}

void PostUpdater::post_update(){
    QFutureWatcher<system_call_wrapper_error_t> *watcher
        = new QFutureWatcher<system_call_wrapper_error_t>(this);
    QFuture<system_call_wrapper_error_t>  res = QtConcurrent::run(CSystemCallWrapper::p2p_post_update);
    watcher->setFuture(res);
    connect(watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished, [this, res](){
      emit this->output_received(res.result() == SCWE_SUCCESS);
    });
}
