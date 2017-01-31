#include <QTimer>
#include <QEventLoop>
#include "RestWorker.h"
#include "ApplicationLog.h"
#include "NotifiactionObserver.h"
#include "IFunctor.h"
#include "FunctorWithResult.h"

static const QString POST_URL("https://hub.subut.ai/rest/v1/tray/%1");
static const QString GET_URL("https://hub.subut.ai/rest/v1/tray/%1");
static const QString HEALTH_URL("https://hub.subut.ai/rest/v1/tray/tray-data");
static const QString GORJUN_URL("https://cdn.subut.ai:8338/kurjun/rest/%1");

#ifndef RT_OS_WINDOWS
#define EVENT_LOOP CEventLoop<SynchroPrimitives::CLinuxManualResetEvent>
#else
#define EVENT_LOOP CEventLoop<SynchroPrimitives::CWindowsManualResetEvent>
#endif

CRestWorker::CRestWorker() {
  m_el = new EVENT_LOOP(NULL, NULL, NULL, 15000, false);
  m_el->Run();

  IFunctor* functor = new FunctorWithResult<QNetworkAccessManager*>(
                        CRestWorker::create_network_manager, "create network manager");
  m_network_manager = EVENT_LOOP::GetSyncResult<QNetworkAccessManager*>(
                        m_el, functor, true);
}

CRestWorker::~CRestWorker() {
  IFunctor* functor = new FunctorWithResult<int, QNetworkAccessManager*>(free_network_manager,
                                                                         m_network_manager,
                                                                         "free network manager");
  EVENT_LOOP::GetSyncResult<int>(m_el, functor, true);
}
////////////////////////////////////////////////////////////////////////////

void
CRestWorker::login(const QString& login,
                   const QString& password,
                   int &http_code,
                   int &err_code,
                   int &network_error) {
  static const QString str_url(POST_URL.arg("login"));
  QUrl url_login(str_url);
  QUrlQuery query_login;
  query_login.addQueryItem("email", login);
  query_login.addQueryItem("password", password);
  url_login.setQuery(query_login);
  QNetworkRequest request(url_login);
  request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
  QByteArray arr = send_request(m_network_manager, request, false,
                                http_code, err_code, network_error,
                                QByteArray(), true);

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
  QUrl url_env(GET_URL.arg(link));
  QNetworkRequest req(url_env);
  req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
  QByteArray arr = send_request(m_network_manager, req, true,
                                http_code, err_code, network_error,
                                QByteArray(), true);

  QJsonDocument doc  = QJsonDocument::fromJson(arr);
  if (doc.isNull()) {
    if (err_code != RE_NETWORK_ERROR &&
        err_code != RE_TIMEOUT &&
        QString(arr) != "[]") {
      err_code = RE_NOT_JSON_DOC;
    }
    CApplicationLog::Instance()->LogInfo("Received not json document from url : %s", link.toStdString().c_str());
    return QJsonDocument();
  }
  return doc;
}

////////////////////////////////////////////////////////////////////////////

std::vector<CEnvironment>
CRestWorker::get_environments(int& http_code,
                              int& err_code,
                              int& network_error) {
  std::vector<CEnvironment> lst_res;
  QJsonDocument doc = get_request_json_document("environments", http_code, err_code, network_error);
  QJsonArray arr = doc.array();
  for (auto i = arr.begin(); i != arr.end(); ++i) {
    if (i->isNull() || !i->isObject()) continue;
    lst_res.push_back(CEnvironment(i->toObject()));
  }
  return lst_res;
}
////////////////////////////////////////////////////////////////////////////

CHubBalance
CRestWorker::get_balance(int& http_code,
                         int& err_code,
                         int& network_error) {
  QJsonDocument doc = get_request_json_document("balance", http_code, err_code, network_error);
  if (err_code != 0) return CHubBalance();
  if (!doc.isObject()) { err_code = RE_NOT_JSON_OBJECT; return CHubBalance(); }
  QJsonObject balance = doc.object();
  return CHubBalance(balance["currentBalance"].toString());
}
////////////////////////////////////////////////////////////////////////////

std::vector<CRHInfo>
CRestWorker::get_containers(int &http_code,
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
  QUrl url_gorjun_fi(GORJUN_URL.arg(str_fi));
  QUrlQuery query_gorjun_fi;
  query_gorjun_fi.addQueryItem("name", file_name);
  url_gorjun_fi.setQuery(query_gorjun_fi);
  QNetworkRequest request(url_gorjun_fi);
  request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
  QByteArray arr = send_request(m_network_manager, request, true,
                                http_code, err_code, network_error,
                                QByteArray(), true);
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
  send_request(m_network_manager, request, true,
               http_code, err_code, network_err,
               QByteArray(), false);
  return http_code;
}
////////////////////////////////////////////////////////////////////////////

void
CRestWorker::send_health_request(int &http_code,
                                 int &err_code,
                                 int &network_err,
                                 const QString& p2p_version,
                                 const QString& p2p_status) {

  CHealthReportData report_data(
        p2p_status,
        p2p_version,
        QSysInfo::kernelType() + " " + QSysInfo::kernelVersion());

  CTrayReport<CHealthReportData> report(report_data);
  QJsonDocument doc(report.to_json_object());

  QUrl url(HEALTH_URL);
  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
  send_request(m_network_manager, req, false,
               http_code, err_code, network_err,
               doc.toJson(), true);
}
////////////////////////////////////////////////////////////////////////////

QNetworkReply*
CRestWorker::download_gorjun_file(const QString &file_id) {
  static const QString str_file_url("raw/download");
  QUrl url(GORJUN_URL.arg(str_file_url));
  QUrlQuery query;
  query.addQueryItem("id", file_id);
  url.setQuery(query);
  return download_file(url);
}

QNetworkReply *
CRestWorker::download_file_aux(QNetworkAccessManager *nam,
                               const QUrl &url) {
  QNetworkRequest request(url);
  return nam->get(request);
}
////////////////////////////////////////////////////////////////////////////

QNetworkReply*
CRestWorker::download_file(const QUrl &url) {
  IFunctor* functor = new FunctorWithResult<QNetworkReply*, QNetworkAccessManager*, const QUrl&>
      (download_file_aux, m_network_manager, url, "download file");
  return EVENT_LOOP::GetSyncResult<QNetworkReply*>(m_el, functor, true);
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
  static const QString str_url(POST_URL.arg("ssh-keys"));
  QJsonObject obj;
  QJsonArray keys_arr;
  keys_arr.push_back(QJsonValue(key));
  obj["sshKeys"] = keys_arr;
  QJsonDocument doc(obj);
  QByteArray doc_serialized = doc.toJson();
  QUrl url(str_url);
  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
  send_request(m_network_manager, req, false,
               http_code, err_code, network_err,
               doc_serialized, true);
}
////////////////////////////////////////////////////////////////////////////

std::vector<bool>
CRestWorker::is_sshkeys_in_environment(const QStringList &keys,
                                       const QString &env) {
  static const QString str_url(POST_URL.arg("environments/check-key"));
  std::vector<bool> lst_res;
  QJsonObject obj;
  QJsonArray json_keys;
  for (auto i = keys.begin(); i != keys.end(); ++i)
    json_keys.push_back(QJsonValue(*i));

  obj["sshKeys"] = json_keys;
  obj["envId"]  = QJsonValue(env);

  qDebug() << "****";
  qDebug() << obj;
  qDebug() << "****";

  QJsonDocument doc(obj);
  QByteArray doc_serialized = doc.toJson();
  QUrl url(str_url);
  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
  int http_code, err_code, network_err;
  QByteArray res_arr = send_request(m_network_manager, req, false,
                                    http_code, err_code, network_err,
                                    doc_serialized, false);

  qDebug() << res_arr;
  QJsonDocument res_doc = QJsonDocument::fromJson(res_arr);
  if (res_doc.isEmpty()) return lst_res;
  if (!res_doc.isArray()) return lst_res;  
  QJsonArray json_arr = res_doc.array();
  for (auto i = json_arr.begin(); i != json_arr.end(); ++i)
    lst_res.push_back(i->toString() == "true");
  return lst_res;
}
////////////////////////////////////////////////////////////////////////////

void
CRestWorker::add_sshkey_to_environments(const QString &key,
                                        const std::vector<QString> &lst_environments,
                                        int &http_code,
                                        int &err_code,
                                        int &network_err) {
  static const QString str_url(POST_URL.arg("environments/ssh-keys"));
  QJsonObject obj;
  QJsonArray arr_environments;

  for (auto i = lst_environments.begin(); i != lst_environments.end(); ++i)
    arr_environments.push_back(QJsonValue(*i));

  obj["sshKey"] = QJsonValue(key);
  obj["environments"] = arr_environments;

  QJsonDocument doc(obj);
  QByteArray doc_serialized = doc.toJson();
  QUrl url(str_url);
  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

  QByteArray res_arr = send_request(m_network_manager, req, false,
                                    http_code, err_code, network_err,
                                    doc_serialized, false);
}
////////////////////////////////////////////////////////////////////////////

QNetworkAccessManager*
CRestWorker::create_network_manager() {
  return new QNetworkAccessManager;
}
////////////////////////////////////////////////////////////////////////////

int
CRestWorker::free_network_manager(QNetworkAccessManager *nam) {
  if (nam) delete nam;
  return 0;
}
////////////////////////////////////////////////////////////////////////////

QByteArray
CRestWorker::send_request_aux(QNetworkAccessManager *nam,
                              QNetworkRequest &req,
                              bool get,
                              int &http_status_code,
                              int &err_code,
                              int &network_error,
                              QByteArray data,
                              bool show_network_err_msg) {
  req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
  if (nam->networkAccessible() != QNetworkAccessManager::Accessible) {
    CApplicationLog::Instance()->LogError("Network isn't accessible : %d", (int)nam->networkAccessible());
    nam->setNetworkAccessible(QNetworkAccessManager::Accessible);
  }

  err_code = RE_SUCCESS;
  network_error = 0;
  http_status_code = -1;

  QEventLoop* loop = new QEventLoop;
  QTimer* timer = new QTimer;
  timer->setSingleShot(true);
  timer->start(8000);

  QNetworkReply* reply = get ? nam->get(req) : nam->post(req, data);

  QObject::connect(timer, SIGNAL(timeout()), loop, SLOT(quit()));
  QObject::connect(reply, SIGNAL(finished()), loop, SLOT(quit()));

  QObject::connect(timer, SIGNAL(timeout()), loop, SLOT(deleteLater()));
  QObject::connect(reply, SIGNAL(finished()), loop, SLOT(deleteLater()));

  QList<QSslError> errors2ignore;
  errors2ignore << QSslError(QSslError::CertificateUntrusted);
  errors2ignore << QSslError(QSslError::SelfSignedCertificate);
  errors2ignore << QSslError(QSslError::HostNameMismatch);
  reply->ignoreSslErrors();

  loop->exec();

  //timer is active if timeout didn't fire
  if (!timer->isActive()) {
    reply->abort();
    err_code = RE_TIMEOUT;
    reply->deleteLater();
    timer->deleteLater();
    return QByteArray();
  }

  timer->stop();
  timer->deleteLater();
  if (reply->error() != QNetworkReply::NoError) {
    network_error = reply->error();
    CApplicationLog::Instance()->LogError("Send request network error : %s",
                                          reply->errorString().toStdString().c_str());
    if (show_network_err_msg)
      CNotificationObserver::NotifyAboutError(reply->errorString());
    err_code = RE_NETWORK_ERROR;
    reply->deleteLater();
    return QByteArray();
  }

  bool parsed = false;
  http_status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&parsed);
  QByteArray res = reply->readAll();
  reply->deleteLater();
  return res;
}
////////////////////////////////////////////////////////////////////////////

QByteArray
CRestWorker::send_request(QNetworkAccessManager *nam,
                          QNetworkRequest &req,
                          bool get,
                          int& http_status_code,
                          int& err_code,
                          int &network_error,
                          QByteArray data,
                          bool show_network_err_msg) {
  IFunctor* functor = new FunctorWithResult<QByteArray, QNetworkAccessManager*,
            QNetworkRequest&, bool, int&, int&, int &, QByteArray, bool>(send_request_aux,
                                                                        nam, req, get, http_status_code,
                                                                        err_code, network_error, data,
                                                                        show_network_err_msg, "send request");
  return EVENT_LOOP::GetSyncResult<QByteArray>(m_el, functor, true);
}
////////////////////////////////////////////////////////////////////////////
