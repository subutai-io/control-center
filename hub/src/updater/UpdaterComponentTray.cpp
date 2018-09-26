#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QStandardPaths>
#include "updater/UpdaterComponentTray.h"
#include "updater/ExecutableUpdater.h"
#include "RestWorker.h"
#include "SystemCallWrapper.h"
#include "NotificationObserver.h"
#include "DownloadFileManager.h"
#include "OsBranchConsts.h"
#include "Commons.h"

using namespace update_system;


////////////////////////////////////////////////////////////////////////////

CUpdaterComponentTray::CUpdaterComponentTray() {
  m_component_id = TRAY;
}

CUpdaterComponentTray::~CUpdaterComponentTray() {

}
////////////////////////////////////////////////////////////////////////////

bool
CUpdaterComponentTray::update_available_internal() {
  std::vector<CGorjunFileInfo> fi =
      CRestWorker::Instance()->get_gorjun_file_info(tray_kurjun_file_name());
  if (fi.empty()) return false;
  QString str_tray_path = QApplication::applicationFilePath();
  QString md5_current = CCommons::FileMd5(str_tray_path);
  QString md5_kurjun = fi[0].md5_sum();
  return md5_current != md5_kurjun;
}
////////////////////////////////////////////////////////////////////////////

chue_t
CUpdaterComponentTray::update_internal() {
  QString str_tray_path = tray_path();
  if (str_tray_path.isEmpty()) return CHUE_FAILED;

  QString str_tray_download_path = download_tray_path();

  std::vector<CGorjunFileInfo> fi = CRestWorker::Instance()->get_gorjun_file_info(
                                      tray_kurjun_file_name());

  if (fi.empty()) {
    qCritical("File %s isn't presented on kurjun", m_component_id.toStdString().c_str());
    return CHUE_NOT_ON_KURJUN;
  }

  std::vector<CGorjunFileInfo>::iterator item = fi.begin();
  CExecutableUpdater *eu = new CExecutableUpdater(str_tray_download_path,
                                                  str_tray_path);

  if (item->md5_sum() == CCommons::FileMd5(str_tray_download_path)) {
    qInfo("Already have new version of tray in %s",
                  str_tray_download_path.toStdString().c_str());
    this->update_progress_sl(100, 100);
    connect(eu, &CExecutableUpdater::finished,
            this, &CUpdaterComponentTray::update_finished_sl);
    connect(eu, &CExecutableUpdater::finished,
            eu, &CExecutableUpdater::deleteLater);
    eu->replace_executables(true);
    return CHUE_SUCCESS;
  }

  CDownloadFileManager *dm = new CDownloadFileManager(item->name(),
                                                      str_tray_download_path,
                                                      item->size());
  dm->set_link(ipfs_download_url().arg(item->id(), item->name()));

  connect(dm, &CDownloadFileManager::download_progress_sig,
          this, &CUpdaterComponentTray::update_progress_sl);

  connect(dm, &CDownloadFileManager::finished, eu, &CExecutableUpdater::replace_executables);
  connect(eu, &CExecutableUpdater::finished, this, &CUpdaterComponentTray::update_finished_sl);
  connect(eu, &CExecutableUpdater::finished, dm, &CDownloadFileManager::deleteLater);
  connect(eu, &CExecutableUpdater::finished, eu, &CExecutableUpdater::deleteLater);

  dm->start_download();
  return CHUE_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////

void
CUpdaterComponentTray::update_post_action(bool success) {
  if (!success) {
    CNotificationObserver::Error(tr("Failed to update the Control Center. Make sure that you have the required permissions."), DlgNotification::N_SETTINGS);
    return;
  }

  system_call_wrapper_error_t res = CSystemCallWrapper::tray_post_update();
  if (res != SCWE_SUCCESS) {
    qCritical("Failed to finish post install after tray update");
  }
  QMessageBox* msg_box = new QMessageBox(QMessageBox::Question, tr("Successfully updated the Control Center."),
                      tr("The Control Center has been updated. Do you want to restart it now?"),
                      QMessageBox::Yes | QMessageBox::No);
  connect(msg_box, &QMessageBox::finished, msg_box, &QMessageBox::deleteLater);
  if (msg_box->exec() == QMessageBox::No)
    return;

  CCommons::RestartTray();
}
////////////////////////////////////////////////////////////////////////////

QString
CUpdaterComponentTray::tray_path() {
  return QApplication::applicationFilePath();
}
////////////////////////////////////////////////////////////////////////////

QString
CUpdaterComponentTray::download_tray_path() {
    QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
    return (lst_temp.isEmpty() ? QApplication::applicationDirPath() : lst_temp[0]) +
                                QDir::separator() + TRAY;
}

////////////////////////////////////////////////////////////////////////////
//instalation staff just to make compiler happy
chue_t CUpdaterComponentTray::install_internal(){
    return CHUE_SUCCESS;
}

void CUpdaterComponentTray::install_post_internal(bool success) {
  UNUSED_ARG(success);
  return;
}

chue_t CUpdaterComponentTray::uninstall_internal() {
  m_in_progress = false;
  return CHUE_SUCCESS;
}

void CUpdaterComponentTray::uninstall_post_internal(bool success) {
  UNUSED_ARG(success);
}
