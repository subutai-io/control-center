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
  if(file_path == "Not found")
      return "";
  QFile f(file_path);
  if (!f.exists()) return "";
  if (!f.open(QIODevice::ReadOnly)) return "";
  QString hash = QCryptographicHash::hash(f.readAll(), QCryptographicHash::Md5).toHex().constData();

  qDebug()
          <<"Asking md5 of"
          <<f.fileName()
          <<"and result is:"
          <<hash;
  f.close();
  return hash;
}
////////////////////////////////////////////////////////////////////////////

QString
CCommons::NetworkErrorToString(int err_code) {
  switch (err_code) {
    case QNetworkReply::NoError : return QObject::tr("No error");

      // network layer errors [relating to the destination server] (1-99):
    case QNetworkReply::ConnectionRefusedError: return QObject::tr("Error: Connection Refused");
    case QNetworkReply::RemoteHostClosedError : return QObject::tr("Error: Remote Host Closed");
    case QNetworkReply::HostNotFoundError: return QObject::tr("Error: Host Not Found");
    case QNetworkReply::TimeoutError: return QObject::tr("Error: Timeout");
    case QNetworkReply::OperationCanceledError: return QObject::tr("Error: Operation Canceled");
    case QNetworkReply::SslHandshakeFailedError: return QObject::tr("Error: SSL Handshake Failed");
    case QNetworkReply::TemporaryNetworkFailureError: return QObject::tr("Error: Temporary Network Failure");
    case QNetworkReply::NetworkSessionFailedError: return QObject::tr("Error: Network Session Failed");
    case QNetworkReply::BackgroundRequestNotAllowedError: return QObject::tr("Error: Background Request Not Allowed");
    case QNetworkReply::TooManyRedirectsError: return QObject::tr("Error: Too Many Redirects");
    case QNetworkReply::InsecureRedirectError: return QObject::tr("Error: Insecure Redirect");
    case QNetworkReply::UnknownNetworkError: return QObject::tr("Error: Unknown Network");

      // proxy errors (101-199):
    case QNetworkReply::ProxyConnectionRefusedError: return QObject::tr("Error: Proxy Connection Refused");
    case QNetworkReply::ProxyConnectionClosedError: return QObject::tr("Error: Proxy Connection Closed");
    case QNetworkReply::ProxyNotFoundError: return QObject::tr("Error: Proxy Not Found");
    case QNetworkReply::ProxyTimeoutError: return QObject::tr("Error: Proxy Timeout");
    case QNetworkReply::ProxyAuthenticationRequiredError: return QObject::tr("Error: Proxy Authentication Required Error");
    case QNetworkReply::UnknownProxyError: return QObject::tr("Error: Unknown Proxy");

      // content errors (201-299):
    case QNetworkReply::  ContentAccessDenied: return QObject::tr("Content Access Denied");
    case QNetworkReply::ContentOperationNotPermittedError: return QObject::tr("Error: Content Operation Not Permitted");
    case QNetworkReply::ContentNotFoundError: return QObject::tr("Error: Content Not Found");
    case QNetworkReply::AuthenticationRequiredError: return QObject::tr("Error: Authentication Required");
    case QNetworkReply::ContentReSendError: return QObject::tr("Error: Content Resend");
    case QNetworkReply::ContentConflictError: return QObject::tr("Error: Content Conflict");
    case QNetworkReply::ContentGoneError: return QObject::tr("Error: Content Gone");
    case QNetworkReply::UnknownContentError: return QObject::tr("Error: Unknown Content");

      // protocol errors
    case QNetworkReply::ProtocolUnknownError: return QObject::tr("Error: Protocol Unknown ");
    case QNetworkReply::ProtocolInvalidOperationError: return QObject::tr("Error: Protocol Invalid Operation Error");
    case QNetworkReply::ProtocolFailure: return QObject::tr("Protocol Failure");

      // Server side errors (401-499)
    case QNetworkReply::InternalServerError: return QObject::tr("Error: Internal Server");
    case QNetworkReply::OperationNotImplementedError: return QObject::tr("Error: Operation not implemented");
    case QNetworkReply::ServiceUnavailableError: return QObject::tr("Error: Service unavailable");
    case QNetworkReply::UnknownServerError: return QObject::tr("Error: Unknown server");

    default: return QObject::tr("Error: Unknown network");
  }
}
////////////////////////////////////////////////////////////////////////////

void CCommons::RestartTray() {
  QStringList args;
  args << RESTARTED_ARG;
  QProcess::startDetached(QApplication::applicationFilePath(), args);
  QApplication::exit(0);
}

////////////////////////////////////////////////////////////////

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
  while (indexUid < uid.length() && uid[indexUid] != ':') {
    res.append(uid[indexUid]);
    indexUid ++;
  }
  return res;
}


////////////////////////////////////////////////////////////////////////////
