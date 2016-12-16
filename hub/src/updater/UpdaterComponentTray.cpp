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

template<branch_t v> struct Branch2Type {
  enum {val = v};
};

template<os_t v> struct Os2Type {
  enum {val = v};
};

template<class BR, class OS> const char* tray_kurjun_file_name_internal();

#define tray_kurjun_file_name_def(BT_TYPE, OS_TYPE, STRING) \
  template<> \
  const char* tray_kurjun_file_name_internal<Branch2Type<BT_TYPE>, Os2Type<OS_TYPE> >() { \
    return STRING; \
  }

tray_kurjun_file_name_def(BT_MASTER, OS_LINUX, "SubutaiTray")
tray_kurjun_file_name_def(BT_MASTER, OS_MAC, "SubutaiTray_osx")
tray_kurjun_file_name_def(BT_MASTER, OS_WIN, "SubutaiTray.exe")
tray_kurjun_file_name_def(BT_DEV, OS_LINUX, "SubutaiTray_dev")
tray_kurjun_file_name_def(BT_DEV, OS_MAC, "SubutaiTray_osx_dev")
tray_kurjun_file_name_def(BT_DEV, OS_WIN, "SubutaiTray_dev.exe")

////////////////////////////////////////////////////////////////////////////

const char* CUpdaterComponentTray::tray_kurjun_file_name() {
  static const QString branch(GIT_BRANCH);
  static const QString master("master");
  if (branch == master)
    return tray_kurjun_file_name_internal<Branch2Type<BT_MASTER>, Os2Type<CURRENT_OS> >();
  else // if (branch == dev)
    return tray_kurjun_file_name_internal<Branch2Type<BT_DEV>, Os2Type<CURRENT_OS> >();
}
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
                                                      str_tray_download_path,
                                                      item->size());

  CExecutableUpdater *eu = new CExecutableUpdater(str_tray_download_path,
                                                  str_tray_exe_path);

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

std::string
CUpdaterComponentTray::tray_path() {
  return QApplication::applicationFilePath().toStdString();
}
////////////////////////////////////////////////////////////////////////////
