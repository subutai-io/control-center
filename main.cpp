#include <QApplication>
/*need this include to register meta type com::Bstr*/
#include <VBox/com/string.h>
#include <QFile>
#include <QDir>
#include <QMessageLogger>
#include <QMessageLogContext>
#include <QMap>
#include <QCommandLineParser>

#include "IVBoxManager.h"
#include "TrayControlWindow.h"
#include "DlgLogin.h"
#include "SystemCallWrapper.h"
#include "TrayWebSocketServer.h"

void hub_msg_log(QtMsgType type, const QMessageLogContext &context, const QString &msg);

static const char* date_format = "dd_MM_yyyy";
typedef struct msg_function_handler {
  QString file_prefix;
  QString msg_prefix;
} msg_function_handler_t;

QMap<QtMsgType, msg_function_handler_t> dct_lh;
void init_dct_lg() {
  dct_lh[QtDebugMsg] = {"debug_", "[DEBUG:]"};
  dct_lh[QtWarningMsg] = {"warning_", "[WARNING:]"};
  dct_lh[QtCriticalMsg] = {"critical_", "[CRITICAL:]"};
  dct_lh[QtFatalMsg] = {"fatal_", "[FATAL:]"};
  dct_lh[QtInfoMsg] = {"info_", "[INFO:]"};
  dct_lh[QtSystemMsg] = {"system_", "[SYSTEM:]"};
}

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

  init_dct_lg();

  bool d_flag = cmd_parser.isSet(use_log_files_opt);
  if (d_flag)
    qInstallMessageHandler(hub_msg_log);

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

void hub_msg_log(QtMsgType type,
                 const QMessageLogContext &context,
                 const QString &msg) {

  QString file_name = QApplication::applicationDirPath() +
                      QDir::separator() +
                      dct_lh[type].file_prefix +
                      QDate::currentDate().toString(date_format) +
                      ".log";
  QFile file(file_name);
  if (!file.open(QIODevice::ReadWrite))
    return;
  QString log_msg = QString("%1 %2 %3 %4 %5 %6\n").
                    arg(QTime::currentTime().toString("hh:mm:ss")).
                    arg(dct_lh[type].msg_prefix).
                    arg(msg).
                    arg(context.file).
                    arg(context.line).
                    arg(context.function);

  file.seek(file.size());
  file.write(log_msg.toUtf8());
  file.flush();
  file.close();
}
////////////////////////////////////////////////////////////////////////////
