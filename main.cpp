/*need this include to register meta type com::Bstr*/
#include <iostream>
#include <VBox/com/string.h>
#include <QApplication>
#include <QCommandLineParser>
#include <QFile>
#include <QDir>

#include "IVBoxManager.h"
#include "TrayControlWindow.h"
#include "DlgLogin.h"
#include "TrayWebSocketServer.h"
#include "ApplicationLog.h"

/*!
 * \brief main - the entry point of SubutaiTray application
 * \param argc - argument count
 * \param argv - array of arguments
 * \return
 * arguments can be :
 * --v  - uses for getting version of tray application
 * --l  - uses to set log_level. can be 0, 1 and 2. 0 - most detailed. or use "trace", "info" and "error"
 */
int
main(int argc, char *argv[]) {

  QApplication::setApplicationName("SubutaiTray");
  QApplication::setOrganizationName("subut.ai");
  QApplication app(argc, argv);

  QCommandLineParser cmd_parser;

  cmd_parser.setApplicationDescription("This tray application should help users to work with hub");
  QCommandLineOption log_level_opt("l",
                                   "Log level can be TRACE (0), INFO (1) and ERROR (2). Trace is most detailed logs.",
                                   "log_level",
                                   "error");
  QCommandLineOption version_opt("v",
                                 "Version",
                                 "Version");

  cmd_parser.addOption(log_level_opt);
  cmd_parser.addPositionalArgument("log_level", "Log level to use in this application");
  cmd_parser.addOption(version_opt);
  cmd_parser.addHelpOption();
  cmd_parser.parse(QApplication::arguments());

  CApplicationLog::Instance()->SetDirectory(QApplication::applicationDirPath().toStdString().c_str());

  QString ll = cmd_parser.value(log_level_opt);
  if(ll == "trace" || ll == "0")
    CApplicationLog::Instance()->SetLogLevel(CApplicationLog::LT_TRACE);
  else if (ll == "info" || ll == "1")
    CApplicationLog::Instance()->SetLogLevel(CApplicationLog::LT_INFO);
  else if (ll == "error" || ll == "2")
    CApplicationLog::Instance()->SetLogLevel(CApplicationLog::LT_ERROR);

  if (cmd_parser.isSet(version_opt)) {
    std::cout << GIT_VERSION << std::endl;
    return 0;
  }
  CApplicationLog::Instance()->LogTrace("Tray application %s launched\n", GIT_VERSION);

  app.setQuitOnLastWindowClosed(false);
  qRegisterMetaType<com::Bstr>("com::Bstr");  

  QString tmp_file_path = CCommons::AppNameTmp();

  QFile tmp_file(tmp_file_path);
  if (tmp_file.exists()) {
    if (!tmp_file.remove()) {
      CApplicationLog::Instance()->LogError("Couldn't remove file %s", tmp_file_path.toStdString().c_str());
    }
  }

  DlgLogin dlg;
  dlg.setModal(true);
  dlg.run_dialog();
  if (dlg.result() == QDialog::Rejected)
    return 0;

  CTrayServer::Instance()->Init();
  CVBoxManagerSingleton::Instance()->init_com();
  TrayControlWindow tcw;
  return app.exec();
}
////////////////////////////////////////////////////////////////////////////
