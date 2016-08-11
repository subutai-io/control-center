#include <QApplication>
#include <QDir>
#include "updater/UpdaterComponentTray.h"
#include "updater/ExecutableUpdater.h"
#include "RestWorker.h"
#include "SystemCallWrapper.h"
#include "NotifiactionObserver.h"
#include "DownloadFileManager.h"

using namespace update_system;

static const char* tray_kurjun_file_name() {
  static const char* fn =
    #if defined(RT_OS_LINUX)
      "tray_9683ecfe-1034-11e6-b626-f816544befe7";
    #elif defined(RT_OS_DARWIN)
      "tray_9683ecfe-1034-11e6-b626-f816544befe7_mac";
    #elif defined(RT_OS_WINDOWS)
      "tray_9683ecfe-1034-11e6-b626-f816544befe7.exe";
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
  return !fi.empty();
}
////////////////////////////////////////////////////////////////////////////

chue_t
CUpdaterComponentTray::update_internal() {
  std::string str_tray_path = tray_path();
  if (str_tray_path.empty()) return CHUE_FAILED;

  QString qstr_tray_path = QString::fromStdString(tray_path());
  QString qstr_tray_replace_path = QApplication::applicationDirPath() +
                                   QDir::separator() +
                                   QString(tray_kurjun_file_name());

  std::vector<CGorjunFileInfo> fi = CRestWorker::Instance()->get_gorjun_file_info(
                                      tray_kurjun_file_name());

  if (fi.empty()) {
    CApplicationLog::Instance()->LogError("File %s isn't presented on kurjun", m_component_id.toStdString().c_str());
    return CHUE_NOT_ON_KURJUN;
  }

  std::vector<CGorjunFileInfo>::iterator item = fi.begin();
  CDownloadFileManager *dm = new CDownloadFileManager(item->id(),
                                                      m_component_id,
                                                      qstr_tray_path,
                                                      item->size());

  CExecutableUpdater *eu = new CExecutableUpdater(m_component_id,
                                                  qstr_tray_path,
                                                  qstr_tray_replace_path);

  connect(dm, SIGNAL(download_progress_sig(QString,qint64,qint64)),
          this, SLOT(update_progress_sl(QString,qint64,qint64)));

  connect(dm, SIGNAL(finished(QString, bool)), eu, SLOT(replace_executables(QString,bool)));
  connect(eu, SIGNAL(finished(QString,bool)), this, SLOT(update_finished_sl(QString,bool)));
  connect(eu, SIGNAL(finished(QString,bool)), dm, SLOT(deleteLater()));
  connect(eu, SIGNAL(finished(QString,bool)), eu, SLOT(deleteLater()));

  dm->start_download();
  return CHUE_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////

std::string
CUpdaterComponentTray::tray_path() {
  return QApplication::applicationFilePath().toStdString();
}
////////////////////////////////////////////////////////////////////////////
