/*need this include to register meta type com::Bstr*/
#include <VBox/com/string.h>

#include <QApplication>
#include <QCommandLineParser>
#include "IVBoxManager.h"
#include "TrayControlWindow.h"
#include "DlgLogin.h"
#include "TrayWebSocketServer.h"
#include "ApplicationLog.h"


int main(int argc, char *argv[]) {

  QApplication::setApplicationName("SubutaiTray");
  QApplication::setOrganizationName("subut.ai");
  QApplication app(argc, argv);

  QCommandLineParser cmd_parser;

  cmd_parser.setApplicationDescription("This tray application should help users to work with hub");
  QCommandLineOption log_level_opt("l",
                                   "Log level can be TRACE (0), INFO (1) and ERROR (2). Trace is most detailed logs.",
                                   "log_level",
                                   "ERROR");
  cmd_parser.addOption(log_level_opt);
  cmd_parser.addPositionalArgument("log_level", "Log level to use in this application");
  cmd_parser.parse(QApplication::arguments());

  CApplicationLog::Instance()->SetDirectory(QApplication::applicationDirPath().toStdString().c_str());

  QString ll = cmd_parser.value(log_level_opt).toLower().trimmed();
  if(ll == "trace" || ll == "0")
    CApplicationLog::Instance()->SetLogLevel(CApplicationLog::LT_TRACE);
  else if (ll == "info" || ll == "1")
    CApplicationLog::Instance()->SetLogLevel(CApplicationLog::LT_INFO);
  else if (ll == "error" || ll == "2")
    CApplicationLog::Instance()->SetLogLevel(CApplicationLog::LT_ERROR);

  cmd_parser.addHelpOption();

  app.setQuitOnLastWindowClosed(false);
  qRegisterMetaType<com::Bstr>("com::Bstr");

  DlgLogin dlg;
  dlg.setModal(true);
  dlg.exec();
  if (dlg.result() == QDialog::Rejected)
    return 0;

  CTrayServer::Instance()->Init();
  CVBoxManagerSingleton::Instance()->init_com();
  TrayControlWindow tcw;
  return app.exec();
}
////////////////////////////////////////////////////////////////////////////
