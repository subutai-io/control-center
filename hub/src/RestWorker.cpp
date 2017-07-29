#include <QTimer>
#include <QEventLoop>

#include "RestWorker.h"
#include "ApplicationLog.h"
#include "NotificationObserver.h"
#include "OsBranchConsts.h"
#include "Locker.h"

CRestWorker::CRestWorker() {
  m_network_manager = create_network_manager();
}

CRestWorker::~CRestWorker() {
  free_network_manager(m_network_manager);
}
////////////////////////////////////////////////////////////////////////////

void
CRestWorker::login_finished_sl() {
  static QString str_ok = "\"OK\"";
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (reply == nullptr) {
    //todo handle this;
  }

  int http_code, err_code, network_error;
  pre_handle_reply(reply, http_code, err_code, network_error);
  QByteArray arr = reply->readAll();
  if (err_code == RE_SUCCESS)
    err_code = (QString(arr) != str_ok) ? RE_LOGIN_OR_EMAIL : RE_SUCCESS;
  emit on_login_finished(http_code, err_code, network_error);
  reply->deleteLater();
}
////////////////////////////////////////////////////////////////////////////

void
CRestWorker::get_environments_finished_sl() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (reply == nullptr) {
    return;
  }

  int http_code, err_code, network_error;
  pre_handle_reply(reply, http_code, err_code, network_error);

  QByteArray reply_arr = reply->readAll();
  QJsonDocument doc = qjson_doc_from_arr(reply_arr, err_code);
  QJsonArray doc_arr = doc.array();

  std::vector<CEnvironment> lst_res;
  for (auto i = doc_arr.begin(); i != doc_arr.end(); ++i) {
    if (i->isNull() || !i->isObject()) continue;
    lst_res.push_back(CEnvironment(i->toObject()));
  }

  emit on_get_environments_finished(lst_res,
                                    http_code,
                                    err_code,
                                    network_error);
  reply->deleteLater();
}
////////////////////////////////////////////////////////////////////////////

void
CRestWorker::get_balance_finished_sl() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (reply == nullptr) {
    return;
  }

  int http_code, err_code, network_error;
  pre_handle_reply(reply, http_code, err_code, network_error);
  QByteArray reply_arr = reply->readAll();
  QJsonDocument doc = qjson_doc_from_arr(reply_arr, err_code);
  CHubBalance res_balance;
  do {
    if (err_code != 0) break;
    if (!doc.isObject()) { err_code = RE_NOT_JSON_OBJECT; break; }
    QJsonObject balance = doc.object();
    res_balance = CHubBalance(balance["currentBalance"].toString());
  } while (0);
  emit on_get_balance_finished(res_balance, http_code, err_code, network_error);
  reply->deleteLater();
}
////////////////////////////////////////////////////////////////////////////

void
CRestWorker::check_if_ss_console_is_ready_finished_sl() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (reply == nullptr) {
    return;
  }

  int http_code, err_code, network_err;
  pre_handle_reply(reply, http_code, err_code, network_err);
  bool is_ready = false;
  QString err_msg;
  if (network_err != 0 || err_code != 0 || http_code != 200) {
    if (network_err == 0 && err_code == 0) {
      switch (http_code) {
        case 500:
          err_msg = "Some modules failed (SS restart might be needed)";
          break;
        case 503:
          err_msg = "Not ready yet/ loading";
          break;
        case 404:
          err_msg = "Endpoint itself not loaded yet (edited)";
          break;
        default:
          err_msg = QString("Undefined error. Code : %1").arg(http_code);
      }
    } else {
      err_msg = QString("Can't get SS console's status. Err : %1").arg(CRestWorker::rest_err_to_str((rest_error_t)err_code));
    }
  } else {
    is_ready = true;
  }

  emit on_got_ss_console_readiness(is_ready, err_msg);
  reply->deleteLater();
}
////////////////////////////////////////////////////////////////////////////

void
CRestWorker::login(const QString& login,
                   const QString& password,
                   int &http_code,
                   int &err_code,
                   int &network_error) {
  static const QString str_url(hub_post_url().arg("login"));
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

void
CRestWorker::update_environments() {
  QUrl url_env(hub_get_url().arg("environments"));
  QNetworkRequest req(url_env);
  req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
  QNetworkReply* reply = get_reply(m_network_manager, req);
  reply->ignoreSslErrors();
  connect(reply, &QNetworkReply::finished, this, &CRestWorker::get_environments_finished_sl);
}
////////////////////////////////////////////////////////////////////////////

void
CRestWorker::update_balance() {
  QUrl url_env(hub_get_url().arg("balance"));
  QNetworkRequest req(url_env);
  req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
  QNetworkReply* reply = get_reply(m_network_manager, req);
  reply->ignoreSslErrors();
  connect(reply, &QNetworkReply::finished, this, &CRestWorker::get_balance_finished_sl);
}
////////////////////////////////////////////////////////////////////////////

std::vector<CGorjunFileInfo>
CRestWorker::get_gorjun_file_info(const QString &file_name) {
  static const QString str_fi("raw/info");
  int http_code, err_code, network_error;
  QUrl url_gorjun_fi(hub_gorjun_url().arg(str_fi));
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

void
CRestWorker::check_if_ss_console_is_ready(const QString &url) {
  QUrl req_url(url);
  QNetworkRequest req(req_url);
  QNetworkReply* reply = get_reply(m_network_manager, req);
  connect(reply, &QNetworkReply::finished, this, &CRestWorker::check_if_ss_console_is_ready_finished_sl);
}
////////////////////////////////////////////////////////////////////////////

void
CRestWorker::send_health_request(const QString& p2p_version,
                                 const QString& p2p_status) {

  CHealthReportData report_data(
        p2p_status,
        p2p_version,
        QSysInfo::kernelType() + " " + QSysInfo::kernelVersion());

  CTrayReport<CHealthReportData> report(report_data);
  QJsonDocument doc(report.to_json_object());

  QUrl url(hub_health_url());
  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
  QNetworkReply* reply = post_reply(m_network_manager, doc.toJson(), req);
  connect(reply, &QNetworkReply::finished, [this]() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (reply == nullptr) {
      return;
    }

    int http_code, err_code, network_error;
    pre_handle_reply(reply, http_code, err_code, network_error);
    if (err_code != RE_SUCCESS) {
      CApplicationLog::Instance()->LogError("send_health_request failed. %d, %d, %d",
                                            http_code, err_code, network_error);
    }
    reply->deleteLater();
  });
}
////////////////////////////////////////////////////////////////////////////

QNetworkReply*
CRestWorker::download_gorjun_file(const QString &file_id) {
  static const QString str_file_url("raw/download");
  QUrl url(hub_gorjun_url().arg(str_file_url));
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
////////////////////////////////////////////////////////////////////////////

const QString&
CRestWorker::rest_err_to_str(rest_error_t err) {
  static QString login_err_str[] = {
    "SUCCESS", "HTTP_ERROR",
    "LOGIN_OR_EMAIL_ERROR", "TIMEOUT_ERROR",
    "NOT_JSON_DOC_ERROR", "NOT_JSON_OBJECT_ERROR",
    "NETWORK_ERROR"
  };
  return login_err_str[err];
}
////////////////////////////////////////////////////////////////////////////

std::vector<bool>
CRestWorker::is_sshkeys_in_environment(const QStringList &keys,
                                       const QString &env) {
  static const QString str_url(hub_post_url().arg("environments/check-key"));
  std::vector<bool> lst_res;
  QJsonObject obj;
  QJsonArray json_keys;
  for (auto i = keys.begin(); i != keys.end(); ++i)
    json_keys.push_back(QJsonValue(*i));

  obj["sshKeys"] = json_keys;
  obj["envId"]  = QJsonValue(env);

  QJsonDocument doc(obj);
  QByteArray doc_serialized = doc.toJson();
  QUrl url(str_url);
  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
  int http_code, err_code, network_err;
  QByteArray res_arr = send_request(m_network_manager, req, false,
                                    http_code, err_code, network_err,
                                    doc_serialized, false);

  QJsonDocument res_doc = QJsonDocument::fromJson(res_arr);
  if (res_doc.isEmpty()) return lst_res;
  if (!res_doc.isArray()) return lst_res;
  QJsonArray json_arr = res_doc.array();
  for (auto i = json_arr.begin(); i != json_arr.end(); ++i)
    lst_res.push_back(i->toBool());

  return lst_res;
}
////////////////////////////////////////////////////////////////////////////

void
CRestWorker::add_sshkey_to_environments(const QString &key_name,
                                        const QString &key,
                                        const std::vector<QString> &lst_environments) {
  static const QString str_url(hub_post_url().arg("environments/ssh-keys"));
  QJsonObject obj;
  QJsonArray arr_environments;

  for (auto i = lst_environments.begin(); i != lst_environments.end(); ++i)
    arr_environments.push_back(QJsonValue(*i));

  obj["key_name"] = key_name;
  obj["sshKey"] = QJsonValue(key);
  obj["environments"] = arr_environments;

  QJsonDocument doc(obj);
  QByteArray doc_serialized = doc.toJson();
  QUrl url(str_url);
  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

  QNetworkReply* reply = post_reply(m_network_manager, doc_serialized, req);
  connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
}
////////////////////////////////////////////////////////////////////////////

void
CRestWorker::remove_sshkey_from_environments(const QString &key_name,
                                             const QString &key,
                                             const std::vector<QString> &lst_environments) {
  static const QString str_url(hub_post_url().arg("environments/remove-keys"));
  QJsonObject obj;
  QJsonArray arr_environments;

  for (auto i = lst_environments.begin(); i != lst_environments.end(); ++i)
    arr_environments.push_back(QJsonValue(*i));

  obj["key_name"] = key_name;
  obj["sshKey"] = QJsonValue(key);
  obj["environments"] = arr_environments;

  QJsonDocument doc(obj);
  QByteArray doc_serialized = doc.toJson();
  QUrl url(str_url);
  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

  QNetworkReply* reply = post_reply(m_network_manager, doc_serialized, req);
  connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
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

void
CRestWorker::pre_handle_reply(const QNetworkReply *reply,
                              int& http_code,
                              int& err_code,
                              int& network_error) {
  bool parsed = false;
  http_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&parsed);
  err_code = RE_SUCCESS;
  network_error = 0;
  if (reply->error() != QNetworkReply::NoError) {
    network_error = reply->error();
    err_code = RE_NETWORK_ERROR;
    CApplicationLog::Instance()->LogError("Send request network error : %s",
                                          reply->errorString().toStdString().c_str());
    CNotificationObserver::Error(reply->errorString());
  }
}
////////////////////////////////////////////////////////////////////////////

QJsonDocument
CRestWorker::qjson_doc_from_arr(const QByteArray &arr, int& err_code) {
  QJsonDocument doc  = QJsonDocument::fromJson(arr);
  if (doc.isNull()) {
    if (err_code != RE_NETWORK_ERROR &&
        err_code != RE_TIMEOUT &&
        QString(arr) != "[]") {
      err_code = RE_NOT_JSON_DOC;
    }
    return QJsonDocument();
  }
  return doc;
}
////////////////////////////////////////////////////////////////////////////

static SynchroPrimitives::CriticalSection cs_reply;

QNetworkReply *
CRestWorker::get_reply(QNetworkAccessManager *nam,
                       QNetworkRequest &req) {
  SynchroPrimitives::Locker lock(&cs_reply);
  req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
  if (nam->networkAccessible() != QNetworkAccessManager::Accessible) {
    CApplicationLog::Instance()->LogError("Network isn't accessible : %d", (int)nam->networkAccessible());
    nam->setNetworkAccessible(QNetworkAccessManager::Accessible);
  }
  QNetworkReply* reply = nam->get(req);
  reply->ignoreSslErrors();
  return reply;
}

QNetworkReply *
CRestWorker::post_reply(QNetworkAccessManager *nam,
                        const QByteArray& data,
                        QNetworkRequest &req) {
  SynchroPrimitives::Locker lock(&cs_reply);
  req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
  if (nam->networkAccessible() != QNetworkAccessManager::Accessible) {
    CApplicationLog::Instance()->LogError("Network isn't accessible : %d", (int)nam->networkAccessible());
    nam->setNetworkAccessible(QNetworkAccessManager::Accessible);
  }
  QNetworkReply* reply = nam->post(req, data);
  reply->ignoreSslErrors();
  return reply;
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

  QNetworkReply* reply = get ? get_reply(nam, req) : post_reply(nam, data, req);

  QObject::connect(timer, &QTimer::timeout, loop, &QEventLoop::quit);
  QObject::connect(reply, &QNetworkReply::finished, loop, &QEventLoop::quit);

  reply->ignoreSslErrors();

  loop->exec();
  loop->deleteLater();

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
      CNotificationObserver::Error(reply->errorString());
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
