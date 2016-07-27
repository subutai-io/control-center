#include <QApplication>
#include <QDir>

#include "HubComponentsUpdater.h"
#include "ExecutableUpdater.h"
#include "DownloadFileManager.h"
#include "SystemCallWrapper.h"
#include "RestWorker.h"
#include "NotifiactionObserver.h"
#include "DownloadFileManager.h"
#include "ExecutableUpdater.h"
#include "libssh2/LibsshErrors.h"

CHubComponentsUpdater::CHubComponentsUpdater() :
  m_tray_update_checks(0),
  m_tray_updating(false),
  m_subutai_rh_updating(false) {

}

CHubComponentsUpdater::~CHubComponentsUpdater() {

}
////////////////////////////////////////////////////////////////////////////

std::string
CHubComponentsUpdater::p2p_path() const {
  std::string p2p_path;

  if (CSettingsManager::Instance().p2p_path() != "p2p") {
    p2p_path = CSettingsManager::Instance().p2p_path().toStdString();
  } else {
    system_call_wrapper_error_t cr;
    if ((cr = CSystemCallWrapper::which("p2p", p2p_path)) != SCWE_SUCCESS) {
      CNotifiactionObserver::Instance()->NotifyAboutError(QString("Can't find p2p in PATH. Err : %1").arg(
                                                            CSystemCallWrapper::scwe_error_to_str(cr)));
    }
  }
  return p2p_path;
}
////////////////////////////////////////////////////////////////////////////

static const char* tray_update_file() {
  static const char* fn =
    #if defined(RT_OS_LINUX)
      "tray_9683ecfe-1034-11e6-b626-f816544befe7";
    #elif defined(RT_OS_DARWIN)
      "tray_9683ecfe-1034-11e6-b626-f816544befe7_mac";
    #elif defined(RT_OS_WINDOWS)
      "tray_9683ecfe-1034-11e6-b626-f816544befe7.exe"
    #else
      "";
    #error "TRAY_UPDATE_FILE macros undefined"
    #endif
  return fn;
}

bool CHubComponentsUpdater::tray_check_for_update() {
  if (m_tray_update_checks != 0) return true;
  std::vector<CGorjunFileInfo> fi = CRestWorker::Instance()->get_gorjun_file_info(tray_update_file());
  if (fi.empty()) return false;

  ++m_tray_update_checks;
  //todo add work with MD5 checking after installers changes.
  return true;
}
////////////////////////////////////////////////////////////////////////////


chue_t CHubComponentsUpdater::tray_update() {
  if (m_tray_updating) return CHUE_IN_PROGRESS;
  atomic_locker al(&m_tray_updating);
  std::vector<CGorjunFileInfo> fi = CRestWorker::Instance()->get_gorjun_file_info(tray_update_file());
  if (fi.empty()) return CHUE_NOT_ON_KURJUN;
  auto item = fi.begin();
  QString new_file_path = QApplication::applicationDirPath() +
                          QDir::separator() +
                          QString(tray_update_file());

  CDownloadFileManager *dm = new CDownloadFileManager(item->id(),
                                                      new_file_path,
                                                      item->size());
  CExecutableUpdater *eu = new CExecutableUpdater(new_file_path,
                                                  QApplication::applicationFilePath());

  connect(dm, SIGNAL(download_progress_sig(qint64,qint64)), this, SLOT(download_file_progress_sl(qint64,qint64)));
  connect(dm, SIGNAL(finished()), eu, SLOT(replace_executables()));
  connect(eu, SIGNAL(finished()), dm, SLOT(deleteLater()));
  connect(eu, SIGNAL(finished()), eu, SLOT(deleteLater()));
  dm->start_download();
  //todo check file replace result
  return CHUE_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////

chue_t CHubComponentsUpdater::subutai_rh_update() {
  if (m_subutai_rh_updating) return CHUE_IN_PROGRESS;
  atomic_locker al(&m_subutai_rh_updating);

  int exit_code = 0;
  CSystemCallWrapper::run_ss_updater(CSettingsManager::Instance().rh_host().toStdString().c_str(),
                                     CSettingsManager::Instance().rh_port().toStdString().c_str(),
                                     CSettingsManager::Instance().rh_user().toStdString().c_str(),
                                     CSettingsManager::Instance().rh_pass().toStdString().c_str(),
                                     exit_code);

  if (exit_code == RLE_SUCCESS) {
    CNotifiactionObserver::NotifyAboutInfo("Update command succesfull finished");
    CApplicationLog::Instance()->LogInfo("Update command succesfull finished");
    return CHUE_SUCCESS;
  }

  QString err_msg = QString("Update command failed with exit code : %1").arg(exit_code);
  CNotifiactionObserver::NotifyAboutError(err_msg);
  CApplicationLog::Instance()->LogError(err_msg.toStdString().c_str());
  return CHUE_FAILED;
}
////////////////////////////////////////////////////////////////////////////

static const char* p2p_update_file_name() {
  static const char* fn =
    #if defined(RT_OS_LINUX)
      "p2p";
    #elif defined(RT_OS_DARWIN)
      "p2p_osx";
    #elif defined(RT_OS_WINDOWS)
      "p2p.exe"
    #else
      "";
    #error "p2p update file name undefined"
    #endif
  return fn;
}

bool
CHubComponentsUpdater::p2p_check_for_update() {
  if (m_p2p_update_checks != 0)
    return true;
  std::vector<CGorjunFileInfo> fi = CRestWorker::Instance()->get_gorjun_file_info(p2p_update_file_name());
  if (fi.empty()) return false;
  std::string str_p2p_path = p2p_path();
  if (str_p2p_path.empty()) return false;

  QString md5_current = CCommons::FileMd5(QString::fromStdString(str_p2p_path));
  QString md5_kurjun = fi[0].md5_sum();

  if (md5_current == md5_kurjun)
    return false;

  ++m_tray_update_checks;
  return true;
}
////////////////////////////////////////////////////////////////////////////

chue_t
CHubComponentsUpdater::p2p_update() {
  if (m_p2p_updating) return CHUE_IN_PROGRESS;
  atomic_locker al(&m_p2p_updating);
  std::string str_p2p_path = p2p_path();
  if (str_p2p_path.empty())

  return CHUE_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////

void
CHubComponentsUpdater::download_file_progress_sl(qint64 cur,
                                                 qint64 full) {
  emit download_file_progress(cur, full);
}
////////////////////////////////////////////////////////////////////////////

void
CHubComponentsUpdater::tray_downloading_finished() {
  --m_tray_update_checks;
}
////////////////////////////////////////////////////////////////////////////

void
CHubComponentsUpdater::p2p_downloading_finished() {
  --m_p2p_update_checks;
}
////////////////////////////////////////////////////////////////////////////
