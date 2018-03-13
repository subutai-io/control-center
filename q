[1mdiff --git a/hub/include/SystemCallWrapper.h b/hub/include/SystemCallWrapper.h[m
[1mindex e4a9ae8..6fb5e51 100644[m
[1m--- a/hub/include/SystemCallWrapper.h[m
[1m+++ b/hub/include/SystemCallWrapper.h[m
[36m@@ -96,11 +96,11 @@[m [mclass CSystemCallWrapper {[m
 [m
  public:[m
   static system_call_res_t ssystem_th(const QString &cmd,[m
[31m-                                      const QStringList &args, bool read_output,[m
[32m+[m[32m                                      QStringList &args, bool read_output,[m
                                       bool log,[m
                                       unsigned long timeout_msec = ULONG_MAX);[m
 [m
[31m-  static system_call_res_t ssystem(const QString &cmd, const QStringList &args,[m
[32m+[m[32m  static system_call_res_t ssystem(const QString &cmd, QStringList &args,[m
                                    bool read_out, bool log,[m
                                    unsigned long timeout_msec = 30000);[m
 [m
[36m@@ -149,7 +149,7 @@[m [mclass CSystemCallWrapper {[m
                                                          const QString &port,[m
                                                          const QString &key);[m
 [m
[31m-  static system_call_wrapper_error_t install_package(const QString &file_info);[m
[32m+[m[32m  static system_call_wrapper_error_t install_package(const QString &dir, const QString &file_name);[m
 [m
   static system_call_wrapper_error_t run_sshpass_in_terminal(const QString &user,[m
                                                          const QString &ip,[m
[1mdiff --git a/hub/include/updater/HubComponentsUpdater.h b/hub/include/updater/HubComponentsUpdater.h[m
[1mindex 4627c14..baff4b6 100644[m
[1m--- a/hub/include/updater/HubComponentsUpdater.h[m
[1m+++ b/hub/include/updater/HubComponentsUpdater.h[m
[36m@@ -4,8 +4,11 @@[m
 #include <QObject>[m
 #include <QTimer>[m
 #include <map>[m
[32m+[m[32m#include <QFuture>[m
[32m+[m[32m#include <QtConcurrent/QtConcurrent>[m
 [m
 #include "updater/IUpdaterComponent.h"[m
[32m+[m[32m#include "SystemCallWrapper.h"[m
 #include "SettingsManager.h"[m
 [m
 namespace update_system {[m
[36m@@ -107,6 +110,13 @@[m [mnamespace update_system {[m
     void force_update_rh();[m
     void force_update_rhm();[m
 [m
[32m+[m[32m    /**[m
[32m+[m[32m      * @brief Instal implementation for components[m
[32m+[m[32m      *[m
[32m+[m[32m      */[m
[32m+[m[32m    void install(const QString& component_id);[m
[32m+[m[32m    void install_p2p();[m
[32m+[m
   private slots:[m
 [m
     void update_component_timer_timeout(const QString& component_id);[m
[36m@@ -120,4 +130,49 @@[m [mnamespace update_system {[m
   };[m
 }[m
 [m
[32m+[m
[32m+[m[32mclass SilentPackageInstaller : public QObject{[m
[32m+[m[32m  Q_OBJECT[m
[32m+[m[32m  QString dir, file_name;[m
[32m+[m
[32m+[m[32mpublic:[m
[32m+[m[32m  SilentPackageInstaller(QObject *parent = nullptr) : QObject (parent){}[m
[32m+[m[32m  void init (const QString &dir,[m
[32m+[m[32m             const QString &file_name){[m
[32m+[m[32m      this->dir = dir;[m
[32m+[m[32m      this->file_name = file_name;[m
[32m+[m[32m  }[m
[32m+[m
[32m+[m[32m  void startWork() {[m
[32m+[m[32m    QThread* thread = new QThread();[m
[32m+[m[32m    connect(thread, &QThread::started,[m
[32m+[m[32m            this, &SilentPackageInstaller::execute_remote_command);[m
[32m+[m[32m    connect(this, &SilentPackageInstaller::outputReceived,[m
[32m+[m[32m            thread, &QThread::quit);[m
[32m+[m[32m    connect(thread, &QThread::finished,[m
[32m+[m[32m            this, &SilentPackageInstaller::deleteLater);[m
[32m+[m[32m    connect(thread, &QThread::finished,[m
[32m+[m[32m            thread, &QThread::deleteLater);[m
[32m+[m[32m    this->moveToThread(thread);[m
[32m+[m[32m    thread->start();[m
[32m+[m[32m  }[m
[32m+[m
[32m+[m
[32m+[m[32m  void execute_remote_command() {[m
[32m+[m[32m    //QStringList output;[m
[32m+[m[32m    QFutureWatcher<system_call_wrapper_error_t> *watcher[m
[32m+[m[32m        = new QFutureWatcher<system_call_wrapper_error_t>(this);[m
[32m+[m
[32m+[m[32m    QFuture<system_call_wrapper_error_t>  res =[m
[32m+[m[32m        QtConcurrent::run(CSystemCallWrapper::install_package, dir, file_name);[m
[32m+[m[32m    watcher->setFuture(res);[m
[32m+[m[32m    connect(watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished, [this, res](){[m
[32m+[m[32m      emit this->outputReceived(res.result() == SCWE_SUCCESS);[m
[32m+[m[32m    });[m
[32m+[m[32m  }[m
[32m+[m
[32m+[m[32msignals:[m
[32m+[m[32m  void outputReceived(bool success);[m
[32m+[m[32m};[m
[32m+[m
 #endif // HUBCOMPONENTSUPDATER_H[m
[1mdiff --git a/hub/include/updater/IUpdaterComponent.h b/hub/include/updater/IUpdaterComponent.h[m
[1mindex 773a0a8..ef0577c 100644[m
[1m--- a/hub/include/updater/IUpdaterComponent.h[m
[1m+++ b/hub/include/updater/IUpdaterComponent.h[m
[36m@@ -43,6 +43,8 @@[m [mnamespace update_system {[m
     virtual bool update_available_internal() = 0;[m
     virtual chue_t update_internal() = 0;[m
     virtual void update_post_action(bool success) = 0;[m
[32m+[m[32m    virtual chue_t install_internal();[m
[32m+[m[32m    virtual void install_post_interntal(bool success) = 0;[m
 [m
   public:[m
 [m
[36m@@ -72,6 +74,12 @@[m [mnamespace update_system {[m
       return update_internal();[m
     }[m
 [m
[32m+[m[32m    chue_t install() {[m
[32m+[m[32m       if(m_in_progress) return CHUE_IN_PROGRESS;[m
[32m+[m[32m       atomic_locker al(&m_in_progress);[m
[32m+[m[32m       return install_internal();[m
[32m+[m[32m    }[m
[32m+[m
     const QString& component_id() const {return m_component_id;}[m
 [m
   protected slots:[m
[1mdiff --git a/hub/include/updater/UpdaterComponentP2P.h b/hub/include/updater/UpdaterComponentP2P.h[m
[1mindex b28e178..a3a242b 100644[m
[1m--- a/hub/include/updater/UpdaterComponentP2P.h[m
[1m+++ b/hub/include/updater/UpdaterComponentP2P.h[m
[36m@@ -17,6 +17,8 @@[m [mnamespace update_system {[m
     virtual bool update_available_internal();[m
     virtual chue_t update_internal();[m
     virtual void update_post_action(bool success);[m
[32m+[m[32m    virtual chue_t install_internal();[m
[32m+[m[32m    virtual void install_post_interntal(bool success);[m
 [m
   public:[m
     CUpdaterComponentP2P();[m
[1mdiff --git a/hub/src/DlgAbout.cpp b/hub/src/DlgAbout.cpp[m
[1mindex 6a60478..cac1f18 100644[m
[1m--- a/hub/src/DlgAbout.cpp[m
[1m+++ b/hub/src/DlgAbout.cpp[m
[36m@@ -188,12 +188,17 @@[m [mvoid[m
 DlgAbout::update_finished(const QString& file_id,[m
                           bool success) {[m
   if (!success) {[m
[31m-    QString template_str = tr("Couldn't update component %1");[m
[32m+[m[32m      QString template_str;[m
[32m+[m[32m      if(m_dct_fpb.find(file_id) != m_dct_fpb.end() && m_dct_fpb[file_id].lbl->text()=="undefined")[m
[32m+[m[32m        template_str = tr("Couldn't install component %1");[m
[32m+[m[32m      else[m
[32m+[m[32m        template_str = tr("Couldn't update component %1");[m
     CNotificationObserver::Error(template_str.arg(file_id), DlgNotification::N_NO_ACTION);[m
   }[m
 [m
   if (m_dct_fpb.find(file_id) == m_dct_fpb.end()) return;[m
   m_dct_fpb[file_id].btn->setEnabled(false);[m
[32m+[m[32m  m_dct_fpb[file_id].btn->setText(QString("Update %1").arg(file_id));[m
   m_dct_fpb[file_id].pb->setEnabled(false);[m
   m_dct_fpb[file_id].pb->setValue(0);[m
   m_dct_fpb[file_id].pb->setRange(0, 100);[m
[36m@@ -226,6 +231,7 @@[m [mDlgAbout::got_p2p_version_sl(QString version) {[m
   if(version == "undefined"){[m
       ui->btn_p2p_update->setText("Install P2P");[m
   }[m
[32m+[m[32m  else ui->btn_p2p_update->setText("Update P2P");[m
 }[m
 ////////////////////////////////////////////////////////////////////////////[m
 [m
[1mdiff --git a/hub/src/SystemCallWrapper.cpp b/hub/src/SystemCallWrapper.cpp[m
[1mindex 306df33..8ca73e3 100644[m
[1m--- a/hub/src/SystemCallWrapper.cpp[m
[1m+++ b/hub/src/SystemCallWrapper.cpp[m
[36m@@ -37,7 +37,7 @@[m [mstatic QString error_strings[] = {"Success",[m
                                   "System call timeout"};[m
 [m
 system_call_res_t CSystemCallWrapper::ssystem_th(const QString &cmd,[m
[31m-                                                 const QStringList &args,[m
[32m+[m[32m                                                 QStringList &args,[m
                                                  bool read_output, bool log,[m
                                                  unsigned long timeout_msec) {[m
   QFuture<system_call_res_t> f1 =[m
[36m@@ -48,10 +48,17 @@[m [msystem_call_res_t CSystemCallWrapper::ssystem_th(const QString &cmd,[m
 ////////////////////////////////////////////////////////////////////////////[m
 [m
 system_call_res_t CSystemCallWrapper::ssystem(const QString &cmd,[m
[31m-                                              const QStringList &args,[m
[32m+[m[32m                                              QStringList &args,[m
                                               bool read_out, bool log,[m
                                               unsigned long timeout_msec) {[m
   QProcess proc;[m
[32m+[m[32m  if(args.begin() != args.end() && args.size() >= 2){[m
[32m+[m[32m      if(*(args.begin())=="set_working_directory"){[m
[32m+[m[32m          args.erase(args.begin());[m
[32m+[m[32m          proc.setWorkingDirectory(*(args.begin()));[m
[32m+[m[32m          args.erase(args.begin());[m
[32m+[m[32m      }[m
[32m+[m[32m  }[m
   system_call_res_t res = {SCWE_SUCCESS, QStringList(), 0};[m
 [m
   proc.start(cmd, args);[m
[36m@@ -637,11 +644,32 @@[m [msystem_call_wrapper_error_t CSystemCallWrapper::run_sshkey_in_terminal([m
 [m
 ////////////////////////////////////////////////////////////////////////////[m
 template <class OS>[m
[31m-system_call_wrapper_error_t install_package_internal(const QString &dir);[m
[32m+[m[32msystem_call_wrapper_error_t install_package_internal(const QString &dir, const QString &file_name);[m
 //write installation script to install package[m
 template <>[m
[31m-system_call_wrapper_error_t install_package_internal<Os2Type <OS_LINUX> >(const QString &file_info){[m
[32m+[m[32msystem_call_wrapper_error_t install_package_internal<Os2Type <OS_WIN> >(const QString &dir, const QString &file_name){[m
[32m+[m[32m    QString cmd("msiexec");[m
[32m+[m[32m    QStringList args0;[m
[32m+[m[32m    args0 << "set_working_directory"[m
[32m+[m[32m          << dir[m
[32m+[m[32m          << "/i"[m
[32m+[m[32m          << file_name[m
[32m+[m[32m          << "/qn";[m
[32m+[m
[32m+[m[32m    qDebug()[m
[32m+[m[32m            <<"Installing package:"[m
[32m+[m[32m            <<args0;[m
[32m+[m
[32m+[m[32m    system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args0, true, true);[m
[32m+[m[32m    if(res.exit_code != 0 && res.res == SCWE_SUCCESS)[m
[32m+[m[32m        res.res = SCWE_CREATE_PROCESS;[m
[32m+[m[32m    return res.res;[m
[32m+[m[32m}[m
[32m+[m
[32m+[m[32mtemplate <>[m
[32m+[m[32msystem_call_wrapper_error_t install_package_internal<Os2Type <OS_LINUX> >(const QString &dir, const QString &file_name){[m
 [m
[32m+[m[32m    QString file_info = dir + "/" + file_name;[m
     QString gksu_path;[m
     system_call_wrapper_error_t scr = CSystemCallWrapper::which("gksu", gksu_path);[m
     if (scr != SCWE_SUCCESS) {[m
[36m@@ -727,8 +755,8 @@[m [msystem_call_wrapper_error_t install_package_internal<Os2Type <OS_LINUX> >(const[m
     return SCWE_SUCCESS;[m
 }[m
 [m
[31m-system_call_wrapper_error_t CSystemCallWrapper::install_package(const QString &dir){[m
[31m-    return install_package_internal<Os2Type<CURRENT_OS> >(dir);[m
[32m+[m[32msystem_call_wrapper_error_t CSystemCallWrapper::install_package(const QString &dir, const QString &file_name){[m
[32m+[m[32m    return install_package_internal<Os2Type<CURRENT_OS> >(dir, file_name);[m
 }[m
 ////////////////////////////////////////////////////////////////////////////[m
 template <class OS>[m
[1mdiff --git a/hub/src/updater/HubComponentsUpdater.cpp b/hub/src/updater/HubComponentsUpdater.cpp[m
[1mindex cc02eb8..c4f056b 100644[m
[1m--- a/hub/src/updater/HubComponentsUpdater.cpp[m
[1m+++ b/hub/src/updater/HubComponentsUpdater.cpp[m
[36m@@ -13,6 +13,7 @@[m
 #include "updater/UpdaterComponentRH.h"[m
 #include "updater/UpdaterComponentRHManagement.h"[m
 [m
[32m+[m
 using namespace update_system;[m
 [m
 CHubComponentsUpdater::CHubComponentsUpdater() {[m
[36m@@ -194,6 +195,16 @@[m [mvoid CHubComponentsUpdater::force_update_rhm() {[m
 }[m
 [m
 [m
[32m+[m[32m////////////////////////////////////////////////////////////////////////////[m
[32m+[m
[32m+[m[32mvoid CHubComponentsUpdater::install(const QString &component_id){[m
[32m+[m
[32m+[m[32m}[m
[32m+[m
[32m+[m[32mvoid CHubComponentsUpdater::install_p2p(){[m
[32m+[m[32m    install(IUpdaterComponent::P2P);[m
[32m+[m[32m}[m
[32m+[m
 ////////////////////////////////////////////////////////////////////////////[m
 [m
 void[m
[1mdiff --git a/hub/src/updater/UpdaterComponentP2P.cpp b/hub/src/updater/UpdaterComponentP2P.cpp[m
[1mindex 6771851..0853c0e 100644[m
[1m--- a/hub/src/updater/UpdaterComponentP2P.cpp[m
[1m+++ b/hub/src/updater/UpdaterComponentP2P.cpp[m
[36m@@ -10,6 +10,7 @@[m
 #include "NotificationObserver.h"[m
 #include "DownloadFileManager.h"[m
 #include "Commons.h"[m
[32m+[m[32m#include "updater/HubComponentsUpdater.h"[m
 #include "OsBranchConsts.h"[m
 #include "P2PController.h"[m
 [m
[36m@@ -52,8 +53,7 @@[m [mCUpdaterComponentP2P::p2p_path()[m
 QString[m
 CUpdaterComponentP2P::download_p2p_path() {[m
   QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);[m
[31m-  return (lst_temp.isEmpty() ? QApplication::applicationDirPath() : lst_temp[0]) +[m
[31m-                              QDir::separator() + P2P;[m
[32m+[m[32m  return (lst_temp.isEmpty() ? QApplication::applicationDirPath() : lst_temp[0]);[m
 }[m
 ////////////////////////////////////////////////////////////////////////////[m
 [m
[36m@@ -69,6 +69,40 @@[m [mCUpdaterComponentP2P::update_available_internal() {[m
   return md5_current != md5_kurjun;[m
 }[m
 ////////////////////////////////////////////////////////////////////////////[m
[32m+[m[32mchue_t CUpdaterComponentP2P::install_internal(){[m
[32m+[m[32m    qDebug()[m
[32m+[m[32m            << "Starting install P2P";[m
[32m+[m
[32m+[m[32m    QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);[m
[32m+[m[32m    QString file_name = p2p_kurjun_package_name();[m
[32m+[m[32m    QString file_dir = download_p2p_path();[m
[32m+[m[32m    QString str_p2p_downloaded_path = file_dir + "/" + file_name;[m
[32m+[m
[32m+[m[32m    std::vector<CGorjunFileInfo> fi = CRestWorker::Instance()->get_gorjun_file_info(p2p_kurjun_package_name());[m
[32m+[m[32m    if (fi.empty()) {[m
[32m+[m[32m      qCritical("File %s isn't presented on kurjun", m_component_id.toStdString().c_str());[m
[32m+[m[32m      return CHUE_NOT_ON_KURJUN;[m
[32m+[m[32m    }[m
[32m+[m[32m    std::vector<CGorjunFileInfo>::iterator item = fi.begin();[m
[32m+[m
[32m+[m[32m    CDownloadFileManager *dm = new CDownloadFileManager(item->id(),[m
[32m+[m[32m                                                        str_p2p_downloaded_path,[m
[32m+[m[32m                                                        item->size());[m
[32m+[m
[32m+[m[32m    SilentPackageInstaller *silent_installer = new SilentPackageInstaller(this);[m
[32m+[m[32m    silent_installer->init(file_dir, file_name);[m
[32m+[m[32m    connect(dm, &CDownloadFileManager::download_progress_sig,[m
[32m+[m[32m            this, &CUpdaterComponentP2P::update_progress_sl);[m
[32m+[m[32m    connect(dm, &CDownloadFileManager::finished,[silent_installer](){[m
[32m+[m[32m        silent_installer->startWork();[m
[32m+[m[32m    });[m
[32m+[m[32m    connect(silent_installer, &SilentPackageInstaller::outputReceived,[m
[32m+[m[32m            this, &CUpdaterComponentP2P::update_finished_sl);[m
[32m+[m[32m    connect(dm, &CDownloadFileManager::finished,[m
[32m+[m[32m            dm, &CDownloadFileManager::deleteLater);[m
[32m+[m[32m    dm->start_download();[m
[32m+[m[32m    return CHUE_SUCCESS;[m
[32m+[m[32m}[m
 [m
 chue_t[m
 CUpdaterComponentP2P::update_internal() {[m
[36m@@ -83,41 +117,14 @@[m [mCUpdaterComponentP2P::update_internal() {[m
                                              "empty" : str_p2p_path.toStdString().c_str()));[m
     return CHUE_FAILED;[m
   }[m
[31m-// need to install p2p first after if not installed[m
[31m-  if(str_p2p_path == "Not found"){[m
[31m-[m
[31m-      QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);[m
[31m-      QString str_p2p_downloaded_path = (!lst_temp.empty() ?[m
[31m-                  lst_temp[0] : QApplication::applicationDirPath()) + QDir::separator() + p2p_kurjun_package_name();[m
[31m-[m
[31m-      std::vector<CGorjunFileInfo> fi = CRestWorker::Instance()->get_gorjun_file_info(p2p_kurjun_package_name());[m
[31m-      if (fi.empty()) {[m
[31m-        qCritical("File %s isn't presented on kurjun", m_component_id.toStdString().c_str());[m
[31m-        return CHUE_NOT_ON_KURJUN;[m
[31m-      }[m
[31m-      std::vector<CGorjunFileInfo>::iterator item = fi.begin();[m
[31m-[m
[31m-      CDownloadFileManager *dm = new CDownloadFileManager(item->id(),[m
[31m-                                                          str_p2p_downloaded_path,[m
[31m-                                                          item->size());[m
[31m-      connect(dm, &CDownloadFileManager::download_progress_sig,[m
[31m-              this, &CUpdaterComponentP2P::update_progress_sl);[m
[31m-      connect(dm, &CDownloadFileManager::finished,[str_p2p_downloaded_path](){[m
[31m-        CSystemCallWrapper::install_package(str_p2p_downloaded_path);[m
[31m-      });[m
[31m-      connect(dm, &CDownloadFileManager::finished,[m
[31m-              this, &CUpdaterComponentP2P::update_finished_sl);[m
[31m-      connect(dm, &CDownloadFileManager::finished,[m
[31m-              dm, &CDownloadFileManager::deleteLater);[m
[31m-      dm->start_download();[m
[31m-      return CHUE_SUCCESS;[m
[31m-  }[m
 [m
   //original file path[m
   QString str_p2p_executable_path = p2p_path();[m
 [m
   //this file will replace original file[m
[31m-  QString str_p2p_downloaded_path = download_p2p_path();[m
[32m+[m[32m  QString file_name = P2P;[m
[32m+[m[32m  QString file_dir = download_p2p_path();[m
[32m+[m[32m  QString str_p2p_downloaded_path = file_dir + "/" + file_name;[m
 [m
   std::vector<CGorjunFileInfo> fi = CRestWorker::Instance()->get_gorjun_file_info([m
                                       p2p_kurjun_file_name());[m
[36m@@ -194,4 +201,10 @@[m [mCUpdaterComponentP2P::update_post_action(bool success) {[m
     msg_box->exec();[m
   }[m
 }[m
[32m+[m
[32m+[m[32mvoid CUpdaterComponentP2P::install_post_interntal(bool success){[m
[32m+[m[32m    if(!success)[m
[32m+[m[32m        CNotificationObserver::Instance()->Error(tr("P2P installation failed"), DlgNotification::N_NO_ACTION);[m
[32m+[m[32m    else CNotificationObserver::Instance()->Info(tr("P2P has been installed. Wait 15 seconds until it's started."), DlgNotification::N_NO_ACTION);[m
[32m+[m[32m}[m
 ////////////////////////////////////////////////////////////////////////////[m
