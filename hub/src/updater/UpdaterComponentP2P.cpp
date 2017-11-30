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
#include "OsBranchConsts.h"

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
      CNotificationObserver::Instance()->Error(tr("Can't find p2p in PATH. Err : %1").arg(
                                                            CSystemCallWrapper::scwe_error_to_str(cr)), DlgNotification::N_SETTINGS);
    }
  }
  return p2p_path;
}
////////////////////////////////////////////////////////////////////////////

QString
CUpdaterComponentP2P::download_p2p_path() {
  QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
  return (lst_temp.isEmpty() ? QApplication::applicationDirPath() : lst_temp[0]) +
                              QDir::separator() + P2P;
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
  return md5_current != md5_kurjun;
}
////////////////////////////////////////////////////////////////////////////

chue_t
CUpdaterComponentP2P::update_internal() {
  QString str_p2p_path = p2p_path();
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
  QString str_p2p_downloaded_path = download_p2p_path();

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
    CNotificationObserver::Instance()->Error(tr("P2P has not been updated"), DlgNotification::N_NO_ACTION);
    return;
  }

  CNotificationObserver::Instance()->Info(tr("P2P has been updated"), DlgNotification::N_NO_ACTION);
  int rse_err = 0;

  system_call_wrapper_error_t scwe =
      CSystemCallWrapper::restart_p2p_service(&rse_err);

  if (scwe != SCWE_SUCCESS) {
    CNotificationObserver::Instance()->Error(tr("p2p post update failed. err : %1").
                                                        arg(CSystemCallWrapper::scwe_error_to_str(scwe)), DlgNotification::N_NO_ACTION);
    return;
  }

  if (rse_err == RSE_MANUAL) {
    QMessageBox *msg_box = new QMessageBox(QMessageBox::Question, tr("Attention! P2P update finished"),
                                           tr("P2P has been updated. Restart p2p daemon, please"),
                                           QMessageBox::Ok);
    connect(msg_box, &QMessageBox::finished, msg_box, &QMessageBox::deleteLater);
    msg_box->exec();
  }
}
////////////////////////////////////////////////////////////////////////////
