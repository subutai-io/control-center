#include "Commons.h"
#include <time.h>

#include <QApplication>
#include <QDir>
#include <QCryptographicHash>
#include <QProcess>
#include <QNetworkReply>

bool CCommons::QuitAppFlag = false;
const char* CCommons::RESTARTED_ARG = "restarted";

static const char* date_format = "%d.%m.%Y.txt";
static char date_str_buffer[15];
char*
CCommons::CurrentDateFileNameString() {
  time_t ct = time(NULL); //system now
  tm now;
#ifndef RT_OS_WINDOWS
  localtime_r(&ct, &now);
#else
  localtime_s(&now, &ct);
#endif
  strftime(date_str_buffer, 15, date_format, &now);
  return date_str_buffer;
}
////////////////////////////////////////////////////////////////////////////

static const char* date_time_format = "%d.%m.%Y %H:%M:%S";
static char date_time_str_buffer[20];
char*
CCommons::CurrentDateTimeString() {
  time_t ct = time(NULL); //system now
  tm now;
#ifndef RT_OS_WINDOWS
  localtime_r(&ct, &now);
#else
  localtime_s(&now, &ct);
#endif
  strftime(date_time_str_buffer, 20, date_time_format, &now);
  return date_time_str_buffer;
}
////////////////////////////////////////////////////////////////////////////

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
    case QNetworkReply::NoError : return "No error";

      // network layer errors [relating to the destination server] (1-99):
    case QNetworkReply::ConnectionRefusedError: return "Connection Refused Error";
    case QNetworkReply::RemoteHostClosedError : return "Remote Host Closed Error";
    case QNetworkReply::HostNotFoundError: return "Host Not Found Error";
    case QNetworkReply::TimeoutError: return "Timeout Error";
    case QNetworkReply::OperationCanceledError: return "Operation Canceled Error";
    case QNetworkReply::SslHandshakeFailedError: return "Ssl Handshake Failed Error";
    case QNetworkReply::TemporaryNetworkFailureError: return "Temporary Network Failure Error";
    case QNetworkReply::NetworkSessionFailedError: return "Network Session Failed Error";
    case QNetworkReply::BackgroundRequestNotAllowedError: return "Background Request Not Allowed Error";
    case QNetworkReply::TooManyRedirectsError: return "TooMany Redirects Error";
    case QNetworkReply::InsecureRedirectError: return "Insecure Redirect Error";
    case QNetworkReply::UnknownNetworkError: return "Unknown Network Error";

      // proxy errors (101-199):
    case QNetworkReply::ProxyConnectionRefusedError: return "Proxy Connection Refused Error";
    case QNetworkReply::ProxyConnectionClosedError: return "Proxy Connection Closed Error";
    case QNetworkReply::ProxyNotFoundError: return "Proxy Not Found Error";
    case QNetworkReply::ProxyTimeoutError: return "Proxy Timeout Error";
    case QNetworkReply::ProxyAuthenticationRequiredError: return "Proxy Authentication Required Error";
    case QNetworkReply::UnknownProxyError: return "Unknown Proxy Error";

      // content errors (201-299):
    case QNetworkReply::  ContentAccessDenied: return "Content Access Denied";
    case QNetworkReply::ContentOperationNotPermittedError: return "Content Operation Not Permitted Error";
    case QNetworkReply::ContentNotFoundError: return "Content Not Found Error";
    case QNetworkReply::AuthenticationRequiredError: return "Authentication Required Error";
    case QNetworkReply::ContentReSendError: return "Content Resend Error";
    case QNetworkReply::ContentConflictError: return "Content Conflict Error";
    case QNetworkReply::ContentGoneError: return "Content Gone Error";
    case QNetworkReply::UnknownContentError: return "Unknown Content Error";

      // protocol errors
    case QNetworkReply::ProtocolUnknownError: return "Protocol Unknown Error";
    case QNetworkReply::ProtocolInvalidOperationError: return "Protocol Invalid Operation Error";
    case QNetworkReply::ProtocolFailure: return "Protocol Failure";

      // Server side errors (401-499)
    case QNetworkReply::InternalServerError: return "Internal Server Error";
    case QNetworkReply::OperationNotImplementedError: return "Operation not implemented error";
    case QNetworkReply::ServiceUnavailableError: return "Service unavailable error";
    case QNetworkReply::UnknownServerError: return "Unknown server error";
    default: return "Unknown network error";
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
