#include <iostream>
#include <QApplication>
#include <QCommandLineParser>
#include <QFile>
#include <QDir>
#include <QSplashScreen>
#include <QSharedMemory>
#include <QSystemSemaphore>
#include <QMessageBox>
#include <exception>
#include <QProcess>
#include <QTranslator>
#include <string>
#include "TrayControlWindow.h"
#include "DlgLogin.h"
#include "TrayWebSocketServer.h"
#include "SettingsManager.h"
#include "updater/UpdaterComponentTray.h"

#include "OsBranchConsts.h"
#include "RhController.h"
#include "NotificationLogger.h"
#include "LibsshController.h"
#include "SystemCallWrapper.h"
#include "Logger.h"
#include "LanguageController.h"

////////////////////////////////////////////////////////////////////////////

/*!
 * \brief main - the entry point of SubutaiTray application
 * \param argc - argument count
 * \param argv - array of arguments
 * \return
 * arguments can be :
 * --v  - uses for getting version of tray application
 * --l  - uses to set log_level. can be 0, 1 and 2. 0 - most detailed. or use "trace", "info" and "error"
 */
#ifndef TESTING_MODE
#include "Tester.h"
int
main(int argc, char *argv[]) {
    QApplication::setApplicationName("TestingTray");
    QApplication::setOrganizationName("subut.ai");
    QApplication app(argc, argv);
    Tester::Instance()->runLast();
    app.exec();
    return 0;
}
#else


int
main(int argc, char *argv[]) {
  static const char* sem_guid = "6a27ccc9-8b72-4e9f-8d2a-5e25cb389b77";
  static const char* shmem_guid = "6ad2b325-682e-4acf-81e7-3bd368ee07d7";
  QSystemSemaphore sema(sem_guid, 1);
  bool is_first;
  sema.acquire();

  {
    QSharedMemory shmem(shmem_guid);
    shmem.attach();
  }

  QSharedMemory shmem(shmem_guid);
  if (shmem.attach()) {
    is_first = true;
  } else {
    shmem.create(1);
    is_first = false;
  }
  sema.release();

  QApplication::setApplicationName("SubutaiTray");
  QApplication::setOrganizationName("subut.ai");
  QApplication app(argc, argv);
  Logger::Instance()->Init();
  qInstallMessageHandler(Logger::LoggerMessageOutput);

  QTranslator translator;
  QString locale = LanguageController::CurrentLocale();
  translator.load("SubutaiTray_"+locale);
  app.installTranslator(&translator);

  if (is_first && !QApplication::arguments().contains(CCommons::RESTARTED_ARG)) {
    QMessageBox* msg_box = new QMessageBox(QMessageBox::Information, QObject::tr("Already running"),
                        QObject::tr("One instance of tray application is already running"),
                        QMessageBox::Ok);
    QObject::connect(msg_box, &QMessageBox::finished, msg_box, &QMessageBox::deleteLater);
    msg_box->exec();
    return 0;
  }

  QCommandLineParser cmd_parser;
  cmd_parser.setApplicationDescription(QObject::tr("This tray application should help users to work with hub"));
  QCommandLineOption log_level_opt("l",
                                   "Log level can be DEBUG (0), WARNING (1), CRITICAL (2), FATAL (3), INFO (4). Trace is most detailed logs.",
                                   "log_level",
                                   "1");
  QCommandLineOption version_opt("v",
                                 "Version",
                                 "Version");

  cmd_parser.addOption(log_level_opt);
  cmd_parser.addPositionalArgument("log_level", "Log level to use in this application");
  cmd_parser.addOption(version_opt);
  cmd_parser.addHelpOption();
  if (cmd_parser.isSet(version_opt)) {
    std::cout << TRAY_VERSION << std::endl;
    return 0;
  }

  CRhController::Instance()->init();
  CNotificationLogger::Instance()->init();
  qInfo("Tray application %s launched", TRAY_VERSION);
  app.setQuitOnLastWindowClosed(false);
  qRegisterMetaType<CNotificationObserver::notification_level_t>("CNotificationObserver::notification_level_t");

  QString tmp[] = {".tmp", "_download"};
  for (int i = 0; i < 2; ++i) {
    QString tmp_file_path = QString(tray_kurjun_file_name()) + tmp[i];
    QFile tmp_file(tmp_file_path);
    if (tmp_file.exists()) {
      if (!tmp_file.remove()) {
        qCritical("Couldn't remove file %s", tmp_file_path.toStdString().c_str());
      }
    }
  }

  int result = 0;
  try {
    do {
      DlgLogin dlg;
      dlg.setModal(true);

      QPixmap pm(":/hub/tray_splash.png");
      QSplashScreen sc(pm);
      sc.show();

      dlg.run_dialog(&sc);
      if (dlg.result() == QDialog::Rejected)
        break;

      CTrayServer::Instance()->Init();
      TrayControlWindow tcw;

      if (!CSystemCallWrapper::p2p_daemon_check()) {
        CNotificationObserver::Error(QObject::tr("Can't operate without the p2p daemon. "
                                             "Either change the path setting in Settings or install the daemon it is not installed. "
                                             "You can get the [production|dev|stage] daemon from <a href=\"%1\">here</a>.").
                                     arg(p2p_package_url()));
      }

      result = app.exec();
    } while (0);
  } catch (std::exception& ge) {
    qCritical("Global Exception : %s",
                                          ge.what());
  }

  return result;
}
#endif
////////////////////////////////////////////////////////////////////////////
