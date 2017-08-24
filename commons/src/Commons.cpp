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
