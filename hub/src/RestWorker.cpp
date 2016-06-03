#include <QTimer>
#include <QEventLoop>
#include "SettingsManager.h"
#include "RestWorker.h"
#include "ApplicationLog.h"

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
  QByteArray arr = send_post_request(request, http_code, err_code, network_error);

  static QString str_ok = "\\\"OK\\\"";
  if (err_code != EL_SUCCESS)
    return;
  if (QString(arr) == str_ok) {
    err_code = EL_LOGIN_OR_EMAIL;
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
  QByteArray arr = send_get_request(req, http_code, err_code, network_error);
  QJsonDocument doc  = QJsonDocument::fromJson(arr);
  if (doc.isNull()) {
    err_code = EL_NOT_JSON_DOC;
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
  if (!doc.isObject()) { err_code = EL_NOT_JSON_OBJECT; return CSSBalance(); }
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
    err_code = EL_NOT_JSON_OBJECT;
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
  static const QString str_fi("file/info");
  int http_code, err_code, network_error;
  QUrl url_gorjun_fi(CSettingsManager::Instance().gorjun_url().arg(str_fi));
  QUrlQuery query_gorjun_fi;
  query_gorjun_fi.addQueryItem("name", file_name);
  url_gorjun_fi.setQuery(query_gorjun_fi);
  QNetworkRequest request(url_gorjun_fi);
  request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
  QByteArray arr = send_get_request(request, http_code, err_code, network_error);
  QJsonDocument doc  = QJsonDocument::fromJson(arr);

  std::vector<CGorjunFileInfo> lst_res;
  if (doc.isNull()) {
    err_code = EL_NOT_JSON_DOC;
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

QNetworkReply*
CRestWorker::download_gorjun_file(const QString &file_id) {
  static const QString str_file_url("file/get");
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

const QString &
CRestWorker::login_err_to_str(error_login_t err) {
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

QByteArray
CRestWorker::send_request(const QNetworkRequest &req,
                          bool get,
                          int& http_status_code,
                          int& err_code,
                          int &network_error) {
  err_code = EL_SUCCESS;
  network_error = 0;
  http_status_code = -1;

  QEventLoop loop;
  QTimer timer(&loop);
  timer.setSingleShot(true);
  timer.start(15000);

  QNetworkReply* reply =
      get ? m_network_manager->get(req) : m_network_manager->post(req, QByteArray());

  connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
  connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
  connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(ssl_errors_appeared(QList<QSslError>)));

  loop.exec();

  disconnect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
  disconnect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
  disconnect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(ssl_errors_appeared(QList<QSslError>)));

  //timer active if timeout didn't fire
  if (!timer.isActive()) {
    reply->abort();
    err_code = EL_TIMEOUT;
    return QByteArray();
  }

  timer.stop();
  if (reply->error() != QNetworkReply::NoError) {
    network_error = reply->error();
    CApplicationLog::Instance()->LogError("Send request network error : %s",
                              reply->errorString().toStdString().c_str());
    err_code = EL_NETWORK_ERROR;
    return QByteArray();
  }

  bool parsed = false;
  http_status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&parsed);
  return reply->readAll();
}
////////////////////////////////////////////////////////////////////////////

QByteArray
CRestWorker::send_get_request(const QNetworkRequest &req,
                              int& http_status_code,
                              int& err_code,
                              int &network_error) {
  return send_request(req, true, http_status_code, err_code, network_error);
}
////////////////////////////////////////////////////////////////////////////

QByteArray
CRestWorker::send_post_request(const QNetworkRequest &req,
                               int& http_status_code,
                               int& err_code,
                               int& network_error) {
  return send_request(req, false, http_status_code, err_code, network_error);
}
////////////////////////////////////////////////////////////////////////////

void
CRestWorker::ssl_errors_appeared(QList<QSslError> lst_errors) {
  for (int i = 0; i < lst_errors.size(); ++i) {
    CApplicationLog::Instance()->LogError("ssl_error_code : %d, msg : %s",
                                          lst_errors.at(i).error(),
                                          lst_errors.at(i).errorString().toStdString().c_str());
  }
}
////////////////////////////////////////////////////////////////////////////
