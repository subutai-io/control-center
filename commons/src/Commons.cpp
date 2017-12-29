#include "Commons.h"
#include "SystemCallWrapper.h"

#include <time.h>
#include <QApplication>
#include <QDir>
#include <QCryptographicHash>
#include <QProcess>
#include <QNetworkReply>



const char* CCommons::RESTARTED_ARG = "restarted";

QString
CCommons::FileMd5(const QString &file_path) {
  QFile f(file_path);
  if (!f.exists()) return "";
  if (!f.open(QIODevice::ReadOnly)) return "";
  QString hash = QCryptographicHash::hash(f.readAll(), QCryptographicHash::Md5).toHex().constData();
  f.close();
  return hash;
}
////////////////////////////////////////////////////////////////////////////

QString
CCommons::NetworkErrorToString(int err_code) {
  switch (err_code) {
    case QNetworkReply::NoError : return QObject::tr("No error");

      // network layer errors [relating to the destination server] (1-99):
    case QNetworkReply::ConnectionRefusedError: return QObject::tr("Connection Refused Error");
    case QNetworkReply::RemoteHostClosedError : return QObject::tr("Remote Host Closed Error");
    case QNetworkReply::HostNotFoundError: return QObject::tr("Host Not Found Error");
    case QNetworkReply::TimeoutError: return QObject::tr("Timeout Error");
    case QNetworkReply::OperationCanceledError: return QObject::tr("Operation Canceled Error");
    case QNetworkReply::SslHandshakeFailedError: return QObject::tr("Ssl Handshake Failed Error");
    case QNetworkReply::TemporaryNetworkFailureError: return QObject::tr("Temporary Network Failure Error");
    case QNetworkReply::NetworkSessionFailedError: return QObject::tr("Network Session Failed Error");
    case QNetworkReply::BackgroundRequestNotAllowedError: return QObject::tr("Background Request Not Allowed Error");
    case QNetworkReply::TooManyRedirectsError: return QObject::tr("TooMany Redirects Error");
    case QNetworkReply::InsecureRedirectError: return QObject::tr("Insecure Redirect Error");
    case QNetworkReply::UnknownNetworkError: return QObject::tr("Unknown Network Error");

      // proxy errors (101-199):
    case QNetworkReply::ProxyConnectionRefusedError: return QObject::tr("Proxy Connection Refused Error");
    case QNetworkReply::ProxyConnectionClosedError: return QObject::tr("Proxy Connection Closed Error");
    case QNetworkReply::ProxyNotFoundError: return QObject::tr("Proxy Not Found Error");
    case QNetworkReply::ProxyTimeoutError: return QObject::tr("Proxy Timeout Error");
    case QNetworkReply::ProxyAuthenticationRequiredError: return QObject::tr("Proxy Authentication Required Error");
    case QNetworkReply::UnknownProxyError: return QObject::tr("Unknown Proxy Error");

      // content errors (201-299):
    case QNetworkReply::  ContentAccessDenied: return QObject::tr("Content Access Denied");
    case QNetworkReply::ContentOperationNotPermittedError: return QObject::tr("Content Operation Not Permitted Error");
    case QNetworkReply::ContentNotFoundError: return QObject::tr("Content Not Found Error");
    case QNetworkReply::AuthenticationRequiredError: return QObject::tr("Authentication Required Error");
    case QNetworkReply::ContentReSendError: return QObject::tr("Content Resend Error");
    case QNetworkReply::ContentConflictError: return QObject::tr("Content Conflict Error");
    case QNetworkReply::ContentGoneError: return QObject::tr("Content Gone Error");
    case QNetworkReply::UnknownContentError: return QObject::tr("Unknown Content Error");

      // protocol errors
    case QNetworkReply::ProtocolUnknownError: return QObject::tr("Protocol Unknown Error");
    case QNetworkReply::ProtocolInvalidOperationError: return QObject::tr("Protocol Invalid Operation Error");
    case QNetworkReply::ProtocolFailure: return QObject::tr("Protocol Failure");

      // Server side errors (401-499)
    case QNetworkReply::InternalServerError: return QObject::tr("Internal Server Error");
    case QNetworkReply::OperationNotImplementedError: return QObject::tr("Operation not implemented error");
    case QNetworkReply::ServiceUnavailableError: return QObject::tr("Service unavailable error");
    case QNetworkReply::UnknownServerError: return QObject::tr("Unknown server error");
    default: return QObject::tr("Unknown network error");
  }
}
////////////////////////////////////////////////////////////////////////////

void CCommons::RestartTray() {
  QStringList args;
  args << RESTARTED_ARG;
  QProcess::startDetached(QApplication::applicationFilePath(), args);
  QApplication::exit(0);
}

////////////////////////////////////////////////////////////////////////////

bool
CCommons::IsApplicationLaunchable(const QString &file_path) {
  QFileInfo fi(file_path);
  if (fi.exists() && fi.isExecutable())
    return true;
  QString cmd;
  system_call_wrapper_error_t which_res =
      CSystemCallWrapper::which(file_path, cmd);
  if (which_res != SCWE_SUCCESS) return false;
  QFileInfo fi2(cmd);
  return fi2.exists() && fi2.isExecutable();
}
////////////////////////////////////////////////////////////////////////////

bool
CCommons::IsTerminalLaunchable(const QString &terminal) {
  system_call_wrapper_error_t open_res =
      CSystemCallWrapper::open(terminal);
  if (open_res == SCWE_SUCCESS) return true;

  return false;
}
////////////////////////////////////////////////////////////////////////////

static std::map<QString, QString> dct_term_arg = {
  //linux
  {"xterm", "-e"},
  {"terminator", "-e"},
  {"gnome-terminal", "-x bash -c"},
  {"mate-terminal", "-x bash -c"},
  {"xfce4-terminal", "-x bash -c"},
  {"guake", "-e"},
  {"kterm", "-e bash -c"},
  {"konsole", "-e bash -c"},
  {"termit", "-e bash -c"},
  {"roxterm", "-e bash -c"},
  {"rxvt", "-e bash -c"},
  {"evilvte", "-e bash -c"},
  {"aterm", "-e bash -c"},
  {"lxterminal", "-l -e"},
  {"Terminal", "do script"}, // macos terminal
  {"iTerm", "create window with default profile command"}, // macos terminal
};

bool
CCommons::HasRecommendedTerminalArg(const QString &terminalCmd,
                                 QString& recommendedArg) {
  QString cmd = terminalCmd;
  QFileInfo fi(terminalCmd);
  if (fi.exists())
    cmd = fi.fileName();

  if (dct_term_arg.find(cmd) != dct_term_arg.end()) {
    recommendedArg = dct_term_arg.at(cmd);
    return true;
  }
  return false;
}

QStringList
CCommons::SupportTerminals() {
  QStringList lst_res;
  for (auto i : dct_term_arg) {
#ifdef RT_OS_DARWIN
    if (CCommons::IsTerminalLaunchable(i.first))
      lst_res << i.first;
#endif
#ifdef RT_OS_LINUX
    if (CCommons::IsApplicationLaunchable(i.first))
      lst_res << i.first;
#endif
  }
  return lst_res;
}

QStringList
CCommons::DefaultTerminals() {
  QStringList lst_res;
  for (auto i : dct_term_arg)
    lst_res << i.first;
  return lst_res;
}


QString CCommons::GetFingerprintFromUid(const QString &uid) {
  QString res = "";
  if (uid.indexOf("uid") == -1)
    return res;
  quint16 indexUid = uid.indexOf("uid:") + QString("uid:").size();
  while (indexUid < uid.length() && uid[indexUid] != ":") {
    res.append(uid[indexUid]);
    indexUid ++;
  }
  return res;
}


////////////////////////////////////////////////////////////////////////////
