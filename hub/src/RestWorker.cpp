#include <QTimer>
#include <QEventLoop>
#include "SettingsManager.h"
#include "RestWorker.h"
#include "ApplicationLog.h"
#include "SystemCallWrapper.h"
#include "NotifiactionObserver.h"

CRestWorker::CRestWorker() :
  m_network_manager(NULL) {
}

CRestWorker::~CRestWorker() {
  free_network_manager();
}
////////////////////////////////////////////////////////////////////////////

void
CRestWorker::create_network_manager() {
  if (m_network_manager == NULL) {
    m_network_manager = new QNetworkAccessManager;
  }
}
////////////////////////////////////////////////////////////////////////////

void
CRestWorker::free_network_manager() {
  if (m_network_manager)
    delete m_network_manager;
  m_network_manager = NULL;
}
////////////////////////////////////////////////////////////////////////////

void
CRestWorker::login(const QString& login,
                   const QString& password,
                   int &http_code,
                   int &err_code,
                   int &network_error) {
  QUrl url_login(CSettingsManager::Instance().post_url().arg("login"));
  QUrlQuery query_login;
  query_login.addQueryItem("email", login);
  query_login.addQueryItem("password", password);
  url_login.setQuery(query_login);
  QNetworkRequest request(url_login);
  request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
  QByteArray arr = send_request(request, false, http_code, err_code, network_error, QByteArray(),
                                false, true);

  static QString str_ok = "\"OK\"";
  if (err_code != RE_SUCCESS)
    return;
  if (QString(arr) != str_ok) {
    err_code = RE_LOGIN_OR_EMAIL;
    return;
  }
}
////////////////////////////////////////////////////////////////////////////

QJsonDocument
CRestWorker::get_request_json_document(const QString &link,
                                       int &http_code,
                                       int &err_code,
                                       int &network_error) {
  QUrl url_env(CSettingsManager::Instance().get_url().arg(link));
  QNetworkRequest req(url_env);
  req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
  QByteArray arr = send_request(req, true, http_code, err_code, network_error,
                                QByteArray(), false, true);
  QJsonDocument doc  = QJsonDocument::fromJson(arr);
  if (doc.isNull()) {
    if (err_code != RE_NETWORK_ERROR && err_code != RE_TIMEOUT)
      err_code = RE_NOT_JSON_DOC;
    CApplicationLog::Instance()->LogInfo("Received not json document from url : %s", link.toStdString().c_str());
    return QJsonDocument();
  }
  return doc;
}

////////////////////////////////////////////////////////////////////////////

std::vector<CSSEnvironment>
CRestWorker::get_environments(int& http_code,
                              int& err_code,
                              int& network_error) {
  std::vector<CSSEnvironment> lst_res;
  QJsonDocument doc = get_request_json_document("environments", http_code, err_code, network_error);
  QJsonArray arr = doc.array();
  for (auto i = arr.begin(); i != arr.end(); ++i) {
    if (i->isNull() || !i->isObject()) continue;
    lst_res.push_back(CSSEnvironment(i->toObject()));
  }
  return lst_res;
}
////////////////////////////////////////////////////////////////////////////

CSSBalance
CRestWorker::get_balance(int& http_code,
                         int& err_code,
                         int& network_error) {
  QJsonDocument doc = get_request_json_document("balance", http_code, err_code, network_error);
  if (err_code != 0) return CSSBalance();
  if (!doc.isObject()) { err_code = RE_NOT_JSON_OBJECT; return CSSBalance(); }
  QJsonObject balance = doc.object();
  return CSSBalance(balance["currentBalance"].toString());
}
////////////////////////////////////////////////////////////////////////////

std::vector<CRHInfo>
CRestWorker::get_ssh_containers(int &http_code,
                                int &err_code,
                                int &network_error) {
  QJsonDocument doc = get_request_json_document("containers", http_code, err_code, network_error);
  if (err_code != 0) return std::vector<CRHInfo>();

  if (!doc.isArray()) {
    err_code = RE_NOT_JSON_OBJECT;
    return std::vector<CRHInfo>();
  }

  std::vector<CRHInfo> lst_res;
  QJsonArray arr = doc.array();

  for (auto i = arr.begin(); i != arr.end(); ++i) {
    if (i->isNull() || !i->isObject()) continue;
    lst_res.push_back(CRHInfo(i->toObject()));
  }
  return lst_res;
}
////////////////////////////////////////////////////////////////////////////

std::vector<CGorjunFileInfo>
CRestWorker::get_gorjun_file_info(const QString &file_name) {
  static const QString str_fi("raw/info");
  int http_code, err_code, network_error;
  QUrl url_gorjun_fi(CSettingsManager::Instance().gorjun_url().arg(str_fi));
  QUrlQuery query_gorjun_fi;
  query_gorjun_fi.addQueryItem("name", file_name);
  url_gorjun_fi.setQuery(query_gorjun_fi);
  QNetworkRequest request(url_gorjun_fi);
  request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
  QByteArray arr = send_request(request, true, http_code, err_code, network_error,
                                QByteArray(), false, true);
  QJsonDocument doc  = QJsonDocument::fromJson(arr);

  std::vector<CGorjunFileInfo> lst_res;
  if (doc.isNull()) {
    err_code = RE_NOT_JSON_DOC;
    return lst_res;
  }

  if (doc.isArray()) {
    QJsonArray json_arr = doc.array();
    for (auto i = json_arr.begin(); i != json_arr.end(); ++i) {
      if (i->isNull() || !i->isObject()) continue;
      lst_res.push_back(CGorjunFileInfo(i->toObject()));
    }
  } else if (doc.isObject()) {
    lst_res.push_back(CGorjunFileInfo(doc.object()));
  }

  return lst_res;
}
////////////////////////////////////////////////////////////////////////////

int
CRestWorker::is_ss_console_ready(const QString &url,
                                 int& err_code,
                                 int& network_err) {
  int http_code;
  QUrl req_url(url);
  QNetworkRequest request(req_url);
  send_request(request, true, http_code, err_code, network_err, QByteArray(), true, false);
  return http_code;
}
////////////////////////////////////////////////////////////////////////////

void
CRestWorker::send_health_request(int &http_code,
                                int &err_code,
                                int &network_err) {

  std::string p2p_version, p2p_status;
  CSystemCallWrapper::p2p_version(p2p_version);
  CSystemCallWrapper::p2p_status(p2p_status);

  CHealthReportData report_data(
        QString(p2p_status.c_str()),
        QString(p2p_version.c_str()),
        QSysInfo::kernelType() + " " + QSysInfo::kernelVersion());

  CTrayReport<CHealthReportData> report(report_data);
  QJsonDocument doc(report.to_json_object());

  QUrl url(CSettingsManager::Instance().health_url());
  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
  send_request(req, false, http_code, err_code, network_err, doc.toJson(), false, true);
}
////////////////////////////////////////////////////////////////////////////

QNetworkReply*
CRestWorker::download_gorjun_file(const QString &file_id) {
  static const QString str_file_url("raw/download");
  QUrl url(CSettingsManager::Instance().gorjun_url().arg(str_file_url));
  QUrlQuery query;
  query.addQueryItem("id", file_id);
  url.setQuery(query);
  return download_file(url);
}
////////////////////////////////////////////////////////////////////////////

QNetworkReply*
CRestWorker::download_file(const QUrl &url) {
  QNetworkRequest request(url);
  return m_network_manager->get(request);
}

const QString&
CRestWorker::rest_err_to_str(rest_error_t err) {
  static QString login_err_str[] = {
    "SUCCESS",
    "HTTP_ERROR",
    "LOGIN_OR_EMAIL_ERROR",
    "TIMEOUT_ERROR",
    "NOT_JSON_DOC_ERROR",
    "NOT_JSON_OBJECT_ERROR",
    "NETWORK_ERROR"
  };
  return login_err_str[err];
}
////////////////////////////////////////////////////////////////////////////

void
CRestWorker::send_ssh_key(const QString &key,
                          int &http_code,
                          int &err_code,
                          int &network_err) {
  QJsonObject obj;
  QJsonArray keys_arr;
  keys_arr.push_back(QJsonValue(key));
  obj["sshKeys"] = keys_arr;
  QJsonDocument doc(obj);
  QByteArray doc_serialized = doc.toJson();
  CApplicationLog::Instance()->LogTrace("%s", doc_serialized.toStdString().c_str());
  QUrl url(CSettingsManager::Instance().post_url().arg("ssh-keys"));
  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
  send_request(req, false, http_code, err_code, network_err, doc_serialized, false, true);
}
////////////////////////////////////////////////////////////////////////////

QByteArray
CRestWorker::send_request(QNetworkRequest &req,
                          bool get,
                          int& http_status_code,
                          int& err_code,
                          int &network_error,
                          QByteArray data,
                          bool ignore_ssl_errors,
                          bool show_network_err_msg) {

  req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
  if (m_network_manager->networkAccessible() != QNetworkAccessManager::Accessible) {
    CApplicationLog::Instance()->LogError("Network isn't accessible : %d", (int)m_network_manager->networkAccessible());
    m_network_manager->setNetworkAccessible(QNetworkAccessManager::Accessible);
  }

  err_code = RE_SUCCESS;
  network_error = 0;
  http_status_code = -1;

  QEventLoop loop;
  QTimer timer(&loop);
  timer.setSingleShot(true);
  timer.start(8000);

  QNetworkReply* reply =
      get ? m_network_manager->get(req) : m_network_manager->post(req, data);

  connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
  connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
  if (!ignore_ssl_errors) {
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(ssl_errors_appeared(QList<QSslError>)));
  }
  else {
    QList<QSslError> errors2ignore;
    errors2ignore << QSslError(QSslError::CertificateUntrusted);
    errors2ignore << QSslError(QSslError::SelfSignedCertificate);
    errors2ignore << QSslError(QSslError::HostNameMismatch);
    reply->ignoreSslErrors();
  }

  loop.exec();

  //timer active if timeout didn't fire
  if (!timer.isActive()) {
    reply->abort();
    err_code = RE_TIMEOUT;
    return QByteArray();
  }

  timer.stop();
  if (reply->error() != QNetworkReply::NoError) {
    network_error = reply->error();
    CApplicationLog::Instance()->LogError("Send request network error : %s",
                              reply->errorString().toStdString().c_str());
    if (show_network_err_msg)
      CNotificationObserver::NotifyAboutError(reply->errorString());
    err_code = RE_NETWORK_ERROR;
    return QByteArray();
  }

  bool parsed = false;
  http_status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&parsed);
  QByteArray res = reply->readAll();
  return res;
}
////////////////////////////////////////////////////////////////////////////

void
CRestWorker::ssl_errors_appeared(QList<QSslError> lst_errors) {
  for (auto i = lst_errors.begin(); i != lst_errors.end(); ++i) {
    CApplicationLog::Instance()->LogError("ssl_error_code : %d, msg : %s",
                                          i->error(), i->errorString().toStdString().c_str());
  }
}
////////////////////////////////////////////////////////////////////////////
