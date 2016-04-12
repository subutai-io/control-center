/*need this include to register meta type com::Bstr*/
#include <VBox/com/string.h>

#include <QApplication>
#include <QCommandLineParser>
#include "IVBoxManager.h"
#include "TrayControlWindow.h"
#include "DlgLogin.h"
#include "SystemCallWrapper.h"
#include "TrayWebSocketServer.h"
#include "ApplicationLog.h"


int main(int argc, char *argv[]) {

  QApplication::setApplicationName("SubutaiTray");
  QApplication::setOrganizationName("subut.ai");
  QApplication app(argc, argv);

  QCommandLineParser cmd_parser;

  cmd_parser.setApplicationDescription("This tray application should help users to work with hub");
  cmd_parser.addHelpOption();
  cmd_parser.addVersionOption();

  QCommandLineOption use_log_files_opt("d", QApplication::translate("main", "Use files for debug logs"));
  cmd_parser.addOption(use_log_files_opt);
  cmd_parser.parse(QApplication::arguments());

  CApplicationLog::Instance()->SetDirectory(QApplication::applicationDirPath().toStdString().c_str());
  bool d_flag = cmd_parser.isSet(use_log_files_opt);
  CApplicationLog::Instance()->SetEnabled(d_flag);

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
