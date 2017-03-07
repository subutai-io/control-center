#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include "updater/UpdaterComponentTray.h"
#include "updater/ExecutableUpdater.h"
#include "RestWorker.h"
#include "SystemCallWrapper.h"
#include "NotifiactionObserver.h"
#include "DownloadFileManager.h"
#include "OsBranchConsts.h"

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

  QString str_tray_download_path = QApplication::applicationDirPath() +
                                   QDir::separator() +
                                   QString(tray_kurjun_file_name() +
                                   QString("_download"));

  std::vector<CGorjunFileInfo> fi = CRestWorker::Instance()->get_gorjun_file_info(
                                      tray_kurjun_file_name());

  if (fi.empty()) {
    CApplicationLog::Instance()->LogError("File %s isn't presented on kurjun", m_component_id.toStdString().c_str());
    return CHUE_NOT_ON_KURJUN;
  }

  std::vector<CGorjunFileInfo>::iterator item = fi.begin();
  CDownloadFileManager *dm = new CDownloadFileManager(item->id(),
                                                      str_tray_download_path,
                                                      item->size());

  CExecutableUpdater *eu = new CExecutableUpdater(str_tray_download_path,
                                                  str_tray_path);

  connect(dm, SIGNAL(download_progress_sig(qint64,qint64)),
          this, SLOT(update_progress_sl(qint64,qint64)));

  connect(dm, SIGNAL(finished(bool)), eu, SLOT(replace_executables(bool)));
  connect(eu, SIGNAL(finished(bool)), this, SLOT(update_finished_sl(bool)));
  connect(eu, SIGNAL(finished(bool)), dm, SLOT(deleteLater()));
  connect(eu, SIGNAL(finished(bool)), eu, SLOT(deleteLater()));

  dm->start_download();
  return CHUE_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////

void
CUpdaterComponentTray::update_post_action(bool success) {
  if (!success) {
    CNotificationObserver::NotifyAboutError("Tray application has not been updated");
    return;
  }

  QMessageBox* msg_box = new QMessageBox(QMessageBox::Question, "Attention! Tray update finished",
                      "Tray application has been updated. Do you want to restart it now?",
                      QMessageBox::Yes | QMessageBox::No);
  connect(msg_box, SIGNAL(finished(int)), msg_box, SLOT(deleteLater()));
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
