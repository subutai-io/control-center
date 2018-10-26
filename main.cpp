#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QProcess>
#include <QSharedMemory>
#include <QSplashScreen>
#include <QSystemSemaphore>
#include <QTranslator>
#include <exception>
#include <iostream>
#include <string>
#include "DlgLogin.h"
#include "SettingsManager.h"
#include "TrayControlWindow.h"
#include "TrayWebSocketServer.h"
#include "updater/UpdaterComponentTray.h"

#include "LanguageController.h"
#include "LibsshController.h"
#include "Logger.h"
#include "NotificationLogger.h"
#include "OsBranchConsts.h"
#include "P2PController.h"
#include "PeerController.h"
#include "RhController.h"
#include "SystemCallWrapper.h"


////////////////////////////////////////////////////////////////////////////

/*!
 * \brief main - the entry point of SubutaiTray application
 * \param argc - argument count
 * \param argv - array of arguments
 * \return
 * arguments can be :
 * --v  - uses for getting version of tray application
 * --l  - uses to set log_level. can be 0, 1 and 2. 0 - most detailed. or use
 * "trace", "info" and "error"
 */

int main(int argc, char* argv[]) {
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

  QApplication::setApplicationName("SubutaiControlCenter");
  QApplication::setOrganizationName("subutai.io");
  QApplication app(argc, argv);

  QCommandLineParser cmd_parser;
  cmd_parser.setApplicationDescription(QObject::tr(
      "This Control Center application should help users to work with bazaar"));

  cmd_parser.addHelpOption();
  // version
  QString tray_build_number_str = TRAY_BUILD_NUMBER;
  QApplication::setApplicationVersion(QString("version: %1 build: %2").
    arg(TRAY_VERSION, tray_build_number_str));
  cmd_parser.addVersionOption();
  // log level
  QCommandLineOption log_level_opt("l");
  log_level_opt.setDescription("Adjusts displayed logs' level. logs_level "
                               "can be DEBUG (0), WARNING (1), CRITICAL (2), "
                               "FATAL (3), INFO (4). Logs with lover level than "
                               "logs_level will not be shown. Default value is '1'.");
  log_level_opt.setValueName("logs_level");
  log_level_opt.setDefaultValue("0");
  cmd_parser.addOption(log_level_opt);
  // mode
  QCommandLineOption app_mode("mode");
  app_mode.setDescription("Set application mode for an advanced users. Mode can be "
                          "\"production\", \"stage\", \"development\".");
  app_mode.setValueName("mode");
  app_mode.setDefaultValue("undefined");
  cmd_parser.addOption(app_mode);
  // process app
  cmd_parser.process(app);

  QString branch = cmd_parser.value(app_mode);
  if (branch != "undefined") { //if user specified branch
    if (branch != "development" &&
        branch != "stage" &&
        branch != "production") {
      std::cout << QString("%1: invalid argument '%2' for '--mode'").
                   arg(QApplication::applicationName(), branch).
                   toStdString() << "\n";
      std::cout <<"Valid arguments are:\n"
                  "  - 'production'\n"
                  "  - 'stage'\n"
                  "  - 'development'\n"
                  "Usage: SubutaiControlCenter --mode <mode>\n"
                  "Try 'SubutaiControlCenter --help' for more information.\n";
      return 0;
    }
  }

  bool ok = false;
  QString log_level_str = cmd_parser.value(log_level_opt);
  if (log_level_str != "undefined") { // if user specified log level
    int a_logs_level = log_level_str.toInt(&ok);

    if (a_logs_level < 0 || a_logs_level > 4 || !ok) {
      std::cout << QString("%1: invalid argument '%2' for '-l'").
                   arg(QApplication::applicationName(), log_level_str).
                   toStdString() << "\n";
      std::cout <<"Valid arguments are:\n"
                  "  - '0'\n"
                  "  - '1'\n"
                  "  - '2'\n"
                  "  - '3'\n"
                  "  - '4'\n"
                  "Usage: SubutaiControlCenter -l <log_level>\n"
                  "Try 'SubutaiControlCenter --help' for more information.\n";
      return 0;
    } else {
      std::cout << QString("%1: logs level set to '%2'.").
        arg(QApplication::applicationName(), log_level_str).toStdString() << "\n";
    }
    CSettingsManager::Instance().set_logs_level(a_logs_level);
  }
  set_application_branch(branch);

  Logger::Instance()->Init();

  QTranslator translator;
  QString locale = LanguageController::CurrentLocale();
  translator.load(QString("SubutaiControlCenter_%1.qm").arg(locale),
                  QApplication::applicationDirPath());
  app.installTranslator(&translator);

  qInstallMessageHandler(Logger::LoggerMessageOutput);

  if (is_first &&
      !QApplication::arguments().contains(CCommons::RESTARTED_ARG)) {
    QMessageBox* msg_box = new QMessageBox(
        QMessageBox::Information, QObject::tr("Subutai Control Center Alert"),
        QObject::tr(
            "An instance of the Control Center is already running in your system."),
        QMessageBox::Ok);
    QObject::connect(msg_box, &QMessageBox::finished, msg_box,
                     &QMessageBox::deleteLater);
    msg_box->exec();
    return 0;
  }

  CNotificationLogger::Instance()->init();
  CRhController::Instance()->init();
  qInfo("Tray application %s launched", TRAY_VERSION);
  app.setQuitOnLastWindowClosed(false);
  qRegisterMetaType<CNotificationObserver::notification_level_t>(
      "CNotificationObserver::notification_level_t");
  qRegisterMetaType<DlgNotification::NOTIFICATION_ACTION_TYPE>(
      "DlgNotification::NOTIFICATION_ACTION_TYPE");
  qRegisterMetaType<CEnvironment>("CEnvironment");
  qRegisterMetaType<system_call_wrapper_error_t>("system_call_wrapper_error_t");

  QString tmp[] = {".tmp", "_download"};
  for (int i = 0; i < 2; ++i) {
    QString tmp_file_path = QString(tray_kurjun_file_name()) + tmp[i];
    QFile tmp_file(tmp_file_path);
    if (tmp_file.exists()) {
      if (!tmp_file.remove()) {
        qCritical("Couldn't remove file %s",
                  tmp_file_path.toStdString().c_str());
      }
    }
  }

  int result = 0;
  try {
    do {
      DlgLogin dlg;
      dlg.setModal(true);

      QPixmap pm(":/hub/cc_splash.png");
      QSplashScreen sc(pm);
      sc.show();
      sc.finish(&dlg);

      dlg.run_dialog(&sc);
      if (dlg.result() == QDialog::Rejected) break;

      CTrayServer::Instance()->Init();
      TrayControlWindow::Instance()->Init();

      P2PController::Instance().init();
      P2PStatus_checker::Instance().update_status();

      result = app.exec();
    } while (0);
  } catch (std::exception& ge) {
    qCritical("Global Exception : %s", ge.what());
  }

  return result;
}
////////////////////////////////////////////////////////////////////////////
