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

using namespace update_system;

const char* CUpdaterComponentTray::tray_kurjun_file_name() {
  static const char* fn =
    #if defined(RT_OS_LINUX)
      "SubutaiTray";
    #elif defined(RT_OS_DARWIN)
      "SubutaiTray_osx";
    #elif defined(RT_OS_WINDOWS)
      "SubutaiTray.exe";
    #else
      "";
    #error "TRAY_UPDATE_FILE macros undefined"
    #endif
  return fn;
}
////////////////////////////////////////////////////////////////////////////

CUpdaterComponentTray::CUpdaterComponentTray() {
  m_component_id = QString(TRAY);
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
  std::string str_tray_path = tray_path();
  if (str_tray_path.empty()) return CHUE_FAILED;

  QString str_tray_exe_path = QString::fromStdString(tray_path());
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
                                                      m_component_id,
                                                      str_tray_download_path,
                                                      item->size());

  CExecutableUpdater *eu = new CExecutableUpdater(m_component_id,
                                                  str_tray_download_path,
                                                  str_tray_exe_path);

  connect(dm, SIGNAL(download_progress_sig(QString,qint64,qint64)),
          this, SLOT(update_progress_sl(QString,qint64,qint64)));

  connect(dm, SIGNAL(finished(QString,bool)), eu, SLOT(replace_executables(QString,bool)));
  connect(eu, SIGNAL(finished(QString,bool)), this, SLOT(update_finished_sl(QString,bool)));
  connect(eu, SIGNAL(finished(QString,bool)), dm, SLOT(deleteLater()));
  connect(eu, SIGNAL(finished(QString,bool)), eu, SLOT(deleteLater()));

  dm->start_download();
  return CHUE_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////

void
CUpdaterComponentTray::update_post_action() {
  QMessageBox* msg_box = new QMessageBox(QMessageBox::Question, "Attention! Tray update finished",
                      "Tray application has been updated. Do you want to restart it now?",
                      QMessageBox::Yes | QMessageBox::No);
  connect(msg_box, SIGNAL(finished(int)), msg_box, SLOT(deleteLater()));
  if (msg_box->exec() == QMessageBox::No)
    return;

  QProcess* proc = new QProcess;
  proc->start(QApplication::applicationFilePath());
  QApplication::exit(0);
}
////////////////////////////////////////////////////////////////////////////

std::string
CUpdaterComponentTray::tray_path() {
  return QApplication::applicationFilePath().toStdString();
}
////////////////////////////////////////////////////////////////////////////
