#include <QApplication>
#include <QDir>
#include <QMessageBox>

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
      CNotificationObserver::Instance()->Error(QString("Can't find p2p in PATH. Err : %1").arg(
                                                            CSystemCallWrapper::scwe_error_to_str(cr)));
    }
  }
  return p2p_path;
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
    CApplicationLog::Instance()->LogError("Update p2p failed. Path = %s",
                                          (str_p2p_path.isNull() || str_p2p_path.isEmpty() ?
                                             "empty" : str_p2p_path.toStdString().c_str()));
    return CHUE_FAILED;
  }

  //original file path
  QString str_p2p_executable_path = p2p_path();

  //this file will replace original file
  QString str_p2p_downloaded_path = QApplication::applicationDirPath() +
                                  QDir::separator() + P2P;

  std::vector<CGorjunFileInfo> fi = CRestWorker::Instance()->get_gorjun_file_info(
                                      p2p_kurjun_file_name());

  if (fi.empty()) {
    CApplicationLog::Instance()->LogError("File %s isn't presented on kurjun", m_component_id.toStdString().c_str());
    return CHUE_NOT_ON_KURJUN;
  }

  std::vector<CGorjunFileInfo>::iterator item = fi.begin();
  CDownloadFileManager *dm = new CDownloadFileManager(item->id(),
                                                      str_p2p_downloaded_path,
                                                      item->size());

  CExecutableUpdater *eu = new CExecutableUpdater(str_p2p_downloaded_path,
                                                  str_p2p_executable_path);

  connect(dm, &CDownloadFileManager::download_progress_sig,
          this, &CUpdaterComponentP2P::update_progress_sl);

  connect(dm, &CDownloadFileManager::finished, eu, &CExecutableUpdater::replace_executables);
  connect(eu, &CExecutableUpdater::finished, this, &CUpdaterComponentP2P::update_finished_sl);
  connect(eu, &CExecutableUpdater::finished, dm, &CDownloadFileManager::deleteLater);
  connect(eu, &CExecutableUpdater::finished, eu, &CExecutableUpdater::deleteLater);

  dm->start_download();
  return CHUE_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////

void
CUpdaterComponentP2P::update_post_action(bool success) {
  if (!success) {
    CNotificationObserver::Instance()->Error("P2P has not been updated");
    return;
  }

  CNotificationObserver::Instance()->Info("P2P has been updated");
  int rse_err = 0;

  QString download_path = QApplication::applicationDirPath() +
                          QDir::separator() +
                          QString(P2P);
  QFile df(download_path);

  if (df.exists()) {
    if (df.remove()) {
      CApplicationLog::Instance()->LogInfo("p2p file from tray directory removed");
    } else {
      CApplicationLog::Instance()->LogInfo("Failed to remove p2p file. %s", df.errorString().toStdString().c_str());
    }
  }
  system_call_wrapper_error_t scwe =
      CSystemCallWrapper::restart_p2p_service(&rse_err);

  if (scwe != SCWE_SUCCESS) {
    CNotificationObserver::Instance()->Error(QString("p2p post update failed. err : ").
                                                        arg(CSystemCallWrapper::scwe_error_to_str(scwe)));
    return;
  }

  if (rse_err == RSE_MANUAL) {
    QMessageBox *msg_box = new QMessageBox(QMessageBox::Question, "Attention! P2P update finished",
                                           "P2P has been updated. Restart p2p daemon, please",
                                           QMessageBox::Ok);
    connect(msg_box, &QMessageBox::finished, msg_box, &QMessageBox::deleteLater);
    msg_box->exec();
  }
}
////////////////////////////////////////////////////////////////////////////
