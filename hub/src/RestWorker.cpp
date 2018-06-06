#include <QEventLoop>
#include <QTimer>
#include <QNetworkProxy>
#include <QApplication>

#include "Locker.h"
#include "NotificationObserver.h"
#include "OsBranchConsts.h"
#include "RestWorker.h"
#include "DlgNotification.h"

CRestWorker::CRestWorker() { m_network_manager = create_network_manager(); m_network_manager->setProxy(QNetworkProxy::NoProxy); }

CRestWorker::~CRestWorker() { free_network_manager(m_network_manager); }
////////////////////////////////////////////////////////////////////////////

void CRestWorker::get_p2p_status_finished_sl() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (reply == nullptr) {
    return;
  }
  int http_code, err_code, network_error;
  pre_handle_reply(reply, http_code, err_code, network_error);

  QByteArray reply_arr = reply->readAll();

  QJsonDocument doc = qjson_doc_from_arr(reply_arr, err_code);
  qDebug() << "Json: " << doc;

  std::vector<CP2PInstance> lst_res;
  QJsonObject obj = doc.object();

  if (obj["code"].toInt() == 0){
    QJsonArray doc_arr = obj["instances"].toArray();
    for (auto i : doc_arr) {
      if (i.isNull() || !i.isObject()) continue;
      lst_res.push_back(CP2PInstance(i.toObject()));
    }
  }
  else {
    err_code = obj["code"].toInt();
  }

  emit on_get_p2p_status_finished(lst_res, http_code, err_code, network_error);
}

////////////////////////////////////////////////////////////////////////////

void CRestWorker::get_my_peers_finished_sl() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (reply == nullptr) {
    return;
  }
  int http_code, err_code, network_error;
  pre_handle_reply(reply, http_code, err_code, network_error);

  QByteArray reply_arr = reply->readAll();
  QJsonDocument doc = qjson_doc_from_arr(reply_arr, err_code);
  QJsonArray doc_arr = doc.array();

  std::vector<CMyPeerInfo> lst_res;
  for (auto i : doc_arr) {
    if (i.isNull() || !i.isObject()) continue;
    lst_res.push_back(CMyPeerInfo(i.toObject()));
  }

  emit on_get_my_peers_finished(lst_res, http_code, err_code, network_error);
}
////////////////////////////////////////////////////////////////////////////

void CRestWorker::get_environments_finished_sl() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  qDebug()
      << "Is reply null " << (reply == nullptr);

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

  emit on_get_environments_finished(lst_res, http_code, err_code,
                                    network_error);
}

////////////////////////////////////////////////////////////////////////////

void CRestWorker::get_balance_finished_sl() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  qDebug()
      << "Is reply null " << (reply == nullptr);
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
    if (!doc.isObject()) {
      err_code = RE_NOT_JSON_OBJECT;
      break;
    }
    QJsonObject balance = doc.object();
    res_balance = CHubBalance(balance["currentBalance"].toString());
  } while (0);
  emit on_get_balance_finished(res_balance, http_code, err_code, network_error);
}
////////////////////////////////////////////////////////////////////////////

void CRestWorker::check_if_ss_console_is_ready_finished_sl() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  qDebug()
      << "Is reply null " << (reply == nullptr);
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
      err_msg = QString("Can't get subutai console's status. Err : %1")
                    .arg(CRestWorker::rest_err_to_str((rest_error_t)err_code));
    }
  } else {
    is_ready = true;
  }

  emit on_got_ss_console_readiness(is_ready, err_msg);
  reply->deleteLater();
}
////////////////////////////////////////////////////////////////////////////

void CRestWorker::login(const QString& login, const QString& password,
                        int& http_code, int& err_code, int& network_error) {
  static const QString str_url(hub_post_url().arg("login"));
  QUrl url_login(str_url);
  QUrlQuery query_login;
  query_login.addQueryItem("email", login);
  query_login.addQueryItem("password", password);

  QNetworkRequest request(url_login);
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    "application/x-www-form-urlencoded");
  QByteArray arr = send_request(
      m_network_manager, request, false, http_code, err_code, network_error,
      query_login.toString(QUrl::FullyEncoded).toUtf8(), true);
  qDebug()
      << "Http code " << http_code
      << "Error code " << err_code
      << "Network Error " << network_error;

  static QString str_ok = "\"OK\"";

  if (err_code != RE_SUCCESS) {
    if (http_code == 403) err_code = RE_LOGIN_OR_EMAIL;
    return;
  }

  if (QString(arr) != str_ok) {
    err_code = RE_LOGIN_OR_EMAIL;
    return;
  }
}

////////////////////////////////////////////////////////////////////////////
///////////////////////* console rest API */////////////////////////////////
void CRestWorker::peer_token(const QString &port,const QString &login, const QString &password,
                                 QString &token, int &err_code,
                                 int &http_code, int &network_error){
    qInfo()
            << tr("Getting token of %1").arg(port);

    const QString str_url(QString("https://localhost:%1/rest/v1/identity/gettoken").arg(port));

    QUrl url_login(str_url);
    QUrlQuery query_login;

    query_login.addQueryItem("username", login);
    query_login.addQueryItem("password", password);

    QNetworkRequest request(url_login);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/x-www-form-urlencoded");
    QByteArray arr = send_request(m_network_manager, request, false,
                                  http_code, err_code, network_error,
                                  query_login.toString(QUrl::FullyEncoded).toUtf8(), false, 60000);

    qDebug()
        << "Http code " << http_code
        << "Error code " << err_code
        << "Network Error " << network_error;

    token=QString(arr);
    static const QString str_acces_denied="Access Denied to the resource!";
    if (err_code != RE_SUCCESS) {
      if (http_code == 500) err_code = RE_LOGIN_OR_EMAIL;
      return;
    }

    if(QString(arr) == ""){
        err_code = RE_NETWORK_ERROR;
        return;
    }

    if (QString(arr) == str_acces_denied) {
      err_code = RE_LOGIN_OR_EMAIL;
      return;
    }
}


void CRestWorker::peer_unregister(const QString &port, const QString &token,
                                  int &err_code, int &http_code, int &network_error){
    qInfo()
            << tr("Unregistering peer %1").arg(port);

    const QString str_url(QString("https://localhost:%1/rest/v1/hub/unregister?sptoken=%2").arg(port, token));

    QUrl url_login(str_url);
    QNetworkRequest request(url_login);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QByteArray arr = send_request(m_network_manager, request, 3,
                                  http_code, err_code, network_error,
                                  QByteArray(), false, 60000);
    UNUSED_ARG(arr);
    qDebug()
        << "Http code " << http_code
        << "Error code " << err_code
        << "Network Error " << network_error;
}


void CRestWorker::peer_register(const QString &port,
                                const QString &token, const QString &login,
                                const QString &password, const QString &peer_name,
                                const QString &peer_scope, int &err_code,
                                int &http_code, int &network_error){
    qInfo()
            << tr("Registering peer %1").arg(port);

    const QString str_url(QString("https://localhost:%1/rest/v1/hub/register?sptoken=%2").arg(port, token));
    QUrl url_login(str_url);
    QUrlQuery query;

    query.addQueryItem("email", login);
    query.addQueryItem("password", password);
    query.addQueryItem("peerName",peer_name);
    query.addQueryItem("peerScope",peer_scope);

    QNetworkRequest request(url_login);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/x-www-form-urlencoded");
    QByteArray arr = send_request(m_network_manager, request, false,
                                  http_code, err_code, network_error,
                                  query.toString(QUrl::FullyEncoded).toUtf8(), false, 60000);
    UNUSED_ARG(arr);
    qDebug()
        << "Http code " << http_code
        << "Error code " << err_code
        << "Network Error " << network_error;
}


bool CRestWorker::peer_finger(const QString &port, QString &finger){
    qInfo()
            << tr("Getting finger from %1").arg(port);

    const QString str_url(QString("https://localhost:%1/rest/v1/security/keyman/getpublickeyfingerprint").arg(port));
    int http_code, err_code, network_error;
    QUrl url_finger(str_url);
    QByteArray nothing;
    QNetworkRequest request(url_finger);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QByteArray arr = send_request(m_network_manager, request, 1,
                                  http_code, err_code, network_error,
                                  nothing, false);

    qDebug()
        << "Http code " << http_code
        << "Error code " << err_code
        << "Network Error " << network_error;
    try {
     finger = QString(arr);
    } catch(...){
        qCritical() << "failed to get finger of " << port;
        finger = "";
    }
    return true;
}

bool CRestWorker::peer_set_pass(const QString &port,
                                const QString &username,
                                const QString &old_pass,
                                const QString &new_pass){
    qInfo()<< tr("Setting password for %1").arg(port);

    const QString str_url(QString("https://localhost:%1/login").arg(port));
    QUrl url_finger(str_url);
    url_finger.setPort(port.toInt());
    QUrlQuery query;
    query.addQueryItem("username", username);
    query.addQueryItem("password", old_pass);
    query.addQueryItem("newpassword", new_pass);
    QNetworkRequest request(url_finger);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/x-www-form-urlencoded");
    int http_code, err_code, network_error;

    QByteArray arr = send_request(m_network_manager, request, false,
                                  http_code, err_code, network_error,
                                  query.toString(QUrl::FullyEncoded).toUtf8(), false, 60000);

    UNUSED_ARG(arr);

    qDebug()
        << "Http code " << http_code
        << "Error code " << err_code
        << "Network Error " << network_error;

    return true;
}

////////////////////////////////////////////////////////////////////////////

bool CRestWorker::get_user_info(QString user_info_type, QString& user_info_str) {
  int http_code, err_code, network_error;
  user_info_str = "";
  static const QString str_url(hub_get_url().arg("user-info"));
  QUrl url_login(str_url);
  QNetworkRequest request(url_login);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  QByteArray arr = send_request(m_network_manager, request, true, http_code,
                                err_code, network_error, QByteArray(), true);

  QJsonDocument doc = QJsonDocument::fromJson(arr);
  qDebug()
      << "Json file: " << doc;
  if (doc.isNull() || doc.isEmpty() || !doc.isObject()) {
    qCritical("Get user %s failed. URL : %s", user_info_type.toStdString().c_str(),
                                          str_url.toStdString().c_str());
    return false;
  }

  QJsonObject obj = doc.object();
  if (obj.find(user_info_type) != obj.end())
    user_info_str = QString("%1").arg(obj[user_info_type].toString());
  return true;
}

////////////////////////////////////////////////////////////////////////////
/**
bool CRestWorker::get_user_id(QString& user_id_str) {
  int http_code, err_code, network_error;
  user_id_str = "";
  static const QString str_url(hub_get_url().arg("user-info"));
  QUrl url_login(str_url);
  QNetworkRequest request(url_login);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  QByteArray arr = send_request(m_network_manager, request, true, http_code,
                                err_code, network_error, QByteArray(), true);

  QJsonDocument doc = QJsonDocument::fromJson(arr);
  qDebug()
      << "Json file: " << doc;
  if (doc.isNull() || doc.isEmpty() || !doc.isObject()) {
    qCritical("Get user id failed. URL : %s",
                                          str_url.toStdString().c_str());
    return false;
  }

  QJsonObject obj = doc.object();
  if (obj.find("id") != obj.end())
    user_id_str = QString("%1").arg(obj["id"].toInt());
  return true;
}
////////////////////////////////////////////////////////////////////////////

bool CRestWorker::get_user_email(QString& user_email_str) {
  int http_code, err_code, network_error;
  user_email_str = "";
  static const QString str_url(hub_get_url().arg("user-info"));
  QUrl url_login(str_url);
  QNetworkRequest request(url_login);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  QByteArray arr = send_request(m_network_manager, request, true, http_code,
                                err_code, network_error, QByteArray(), true);

  QJsonDocument doc = QJsonDocument::fromJson(arr);
  qDebug()
      << "Json file: " << doc;
  if (doc.isNull() || doc.isEmpty() || !doc.isObject()) {
    qCritical("Get user id failed. URL : %s",
                                          str_url.toStdString().c_str());
    return false;
  }

  QJsonObject obj = doc.object();
  if (obj.find("email") != obj.end())
    user_email_str = QString("%1").arg(obj["email"].toString());
  return true;
} */
////////////////////////////////////////////////////////////////////////////
void CRestWorker::update_my_peers() {
  QUrl url_env(hub_get_url().arg("my-peers"));
  QNetworkRequest req(url_env);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  QNetworkReply* reply = get_reply(m_network_manager, req);
  reply->ignoreSslErrors();
  QTimer *timer = new QTimer(this);
  timer->setInterval(30000);
  timer->setSingleShot(true);

  connect(timer, &QTimer::timeout, [reply](){
    CNotificationObserver::Instance()->
        Info("Connection timeout, can't connect to bazaar", DlgNotification::N_NO_ACTION);
    if (reply)
      reply->close();
  });

  timer->start();

  connect(reply, &QNetworkReply::finished,
          timer, &QTimer::stop);
  connect(reply, &QNetworkReply::finished,
          timer, &QTimer::deleteLater);
  connect(reply, &QNetworkReply::finished, this,
          &CRestWorker::get_my_peers_finished_sl);
  connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
}

void CRestWorker::update_p2p_status() {
  QUrl url_env(p2p_rest_url().arg("status"));
  QNetworkRequest req(url_env);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  QNetworkReply* reply = get_reply(m_network_manager, req);
  reply->ignoreSslErrors();
  connect(reply, &QNetworkReply::finished, this,
          &CRestWorker::get_p2p_status_finished_sl);
  connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
}

////////////////////////////////////////////////////////////////////////////

void CRestWorker::update_environments() {
  qDebug() << "Getting the environments data from hub";
  QUrl url_env(hub_get_url().arg("environments"));
  QNetworkRequest req(url_env);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  QNetworkReply* reply = get_reply(m_network_manager, req);
  reply->ignoreSslErrors();

  QTimer *timer = new QTimer(this);
  timer->setInterval(30000);
  timer->setSingleShot(true);

  connect(timer, &QTimer::timeout, [reply](){
    CNotificationObserver::Instance()->
        Info("Connection timeout, can't connect to bazaar", DlgNotification::N_NO_ACTION);
    if (reply)
      reply->close();
  });

  timer->start();

  connect(reply, &QNetworkReply::finished,
          timer, &QTimer::stop);
  connect(reply, &QNetworkReply::finished,
          timer, &QTimer::deleteLater);
  connect(reply, &QNetworkReply::finished,
          this,  &CRestWorker::get_environments_finished_sl);
  connect(reply, &QNetworkReply::finished,
          reply, &QNetworkReply::deleteLater);
}
////////////////////////////////////////////////////////////////////////////

void CRestWorker::update_balance() {
  qDebug() << "Getting the balance data from hub";
  QUrl url_env(hub_get_url().arg("balance"));
  QNetworkRequest req(url_env);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  QNetworkReply* reply = get_reply(m_network_manager, req);
  reply->ignoreSslErrors();

  QTimer *timer = new QTimer(this);
  timer->setInterval(30000);
  timer->setSingleShot(true);

  connect(timer, &QTimer::timeout, [reply](){
    CNotificationObserver::Instance()->
        Info("Connection timeout, can't connect to bazaar", DlgNotification::N_NO_ACTION);
    if (reply)
      reply->close();
  });

  timer->start();

  connect(reply, &QNetworkReply::finished,
          timer, &QTimer::stop);
  connect(reply, &QNetworkReply::finished,
          timer, &QTimer::deleteLater);
  connect(reply, &QNetworkReply::finished, this,
          &CRestWorker::get_balance_finished_sl);
  connect(reply, &QNetworkReply::finished,
          reply, &QNetworkReply::deleteLater);
}

////////////////////////////////////////////////////////////////////////////

std::vector<CGorjunFileInfo> CRestWorker::get_gorjun_file_info(
    const QString& file_name) {
  static const QString str_fi("raw/info");
  int http_code, err_code, network_error;
  QUrl url_gorjun_fi(hub_gorjun_url().arg(str_fi));
  QUrlQuery query_gorjun_fi;
  query_gorjun_fi.addQueryItem("name", file_name);
  url_gorjun_fi.setQuery(query_gorjun_fi);
  QNetworkRequest request(url_gorjun_fi);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  QByteArray arr = send_request(m_network_manager, request, true, http_code,
                                err_code, network_error, QByteArray(), true);
  QJsonDocument doc = QJsonDocument::fromJson(arr);
  qDebug()
      << "Requested filename: " << file_name
      << "Json file: " << doc;

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

QString CRestWorker::get_vagrant_plugin_cloud_version(const QString &plugin_name){
  int http_code, err_code, network_error;
  QUrl url_gorjun_fi(QString("https://rubygems.org/api/v1/versions/%1/latest.json").arg(plugin_name));
  QNetworkRequest request(url_gorjun_fi);
  QByteArray arr = send_request(m_network_manager, request, true, http_code,
                                err_code, network_error, QByteArray(), false);
  QJsonDocument doc = QJsonDocument::fromJson(arr);

  qDebug()
      << "Requested plugin version: " << plugin_name
      << "Json file: " << doc;

  if (doc.isNull()) {
    err_code = RE_NOT_JSON_DOC;
    return QString("undefined");
  }

  if (doc.isArray()) {
    QJsonArray json_arr = doc.array();
    return json_arr.first().toString();
  } else if (doc.isObject()) {
      return doc.object()["version"].toString();
  }
  return QString("undefined");
}
////////////////////////////////////////////////////////////////////////////

void CRestWorker::check_if_ss_console_is_ready(const QString& url) {
  QUrl req_url(url);
  QNetworkRequest req(req_url);
  QNetworkReply* reply = get_reply(m_network_manager, req);
  connect(reply, &QNetworkReply::finished, this,
          &CRestWorker::check_if_ss_console_is_ready_finished_sl);
}
////////////////////////////////////////////////////////////////////////////

void CRestWorker::send_health_request(const QString& p2p_version,
                                      const QString& p2p_status) {
  CHealthReportData report_data(
      p2p_status, p2p_version,
      QSysInfo::kernelType() + " " + QSysInfo::kernelVersion());

  CTrayReport<CHealthReportData> report(report_data);
  QJsonDocument doc(report.to_json_object());

  QUrl url(hub_health_url());
  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  QNetworkReply* reply = post_reply(m_network_manager, doc.toJson(), req);
  connect(reply, &QNetworkReply::finished, [this]() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (reply == nullptr) {
      return;
    }

    int http_code, err_code, network_error;
    pre_handle_reply(reply, http_code, err_code, network_error);
    if (err_code != RE_SUCCESS) {
      qCritical(
          "send_health_request failed. http_code : %d, err_code : %d, "
          "network_err : %d",
          http_code, err_code, network_error);
    }
    reply->deleteLater();
  });
}
////////////////////////////////////////////////////////////////////////////

QNetworkReply* CRestWorker::download_gorjun_file(const QString& file_id) {
  static const QString str_file_url("raw/download");
  QUrl url(hub_gorjun_url().arg(str_file_url));
  QUrlQuery query;
  query.addQueryItem("id", file_id);
  url.setQuery(query);
  return download_file(url);
}
////////////////////////////////////////////////////////////////////////////

QNetworkReply* CRestWorker::download_file(const QUrl& url) {
  QNetworkRequest req(url);
  req.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                   QNetworkRequest::AlwaysNetwork);
  QNetworkReply* reply = m_network_manager->get(req);
  reply->ignoreSslErrors();
  return m_network_manager->get(req);
}
////////////////////////////////////////////////////////////////////////////

const QString& CRestWorker::rest_err_to_str(rest_error_t err) {
  static QString login_err_str[] = {
      "SUCCESS",       "HTTP_ERROR",         "LOGIN_OR_EMAIL_ERROR",
      "TIMEOUT_ERROR", "NOT_JSON_DOC_ERROR", "NOT_JSON_OBJECT_ERROR",
      "NETWORK_ERROR"};
  return login_err_str[err];
}
////////////////////////////////////////////////////////////////////////////

std::vector<uint8_t> CRestWorker::is_sshkeys_in_environment(
    const QStringList& keys, const QString& env) {
  qDebug() << "checking keys in " << env;
  static const QString str_url(hub_post_url().arg("environments/check-key"));
  std::vector<uint8_t> lst_res;
  QJsonObject obj;
  QJsonArray json_keys;
  for (auto i = keys.begin(); i != keys.end(); ++i)
    json_keys.push_back(QJsonValue(*i));

  obj["sshKeys"] = json_keys;
  obj["envId"] = QJsonValue(env);

  QJsonDocument doc(obj);
  QByteArray doc_serialized = doc.toJson();
  QUrl url(str_url);
  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  int http_code, err_code, network_err;
  QByteArray res_arr =
      send_request(m_network_manager, req, false, http_code, err_code,
                   network_err, doc_serialized, false);

  QJsonDocument res_doc = QJsonDocument::fromJson(res_arr);
  if (res_doc.isEmpty()) return lst_res;
  if (!res_doc.isArray()) return lst_res;
  QJsonArray json_arr = res_doc.array();
  for (auto i = json_arr.begin(); i != json_arr.end(); ++i)
    lst_res.push_back(i->toBool() ? 1 : 0);
  qDebug() << "checking keys in" << env << "finished";
  return lst_res;
}
////////////////////////////////////////////////////////////////////////////

void CRestWorker::add_sshkey_to_environments(
    const QString& key_name, const QString& key,
    const std::vector<QString>& lst_environments) {
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
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  int http_status_code, err_code, network_error;
  send_request(m_network_manager, req, false,
               http_status_code, err_code, network_error, doc_serialized, true);
}
////////////////////////////////////////////////////////////////////////////

void CRestWorker::remove_sshkey_from_environments(
    const QString& key_name, const QString& key,
    const std::vector<QString>& lst_environments) {
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
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  int http_status_code, err_code, network_error;
  send_request(m_network_manager, req, false,
               http_status_code, err_code, network_error, doc_serialized, true);
}

QNetworkAccessManager* CRestWorker::create_network_manager() {
  return new QNetworkAccessManager;
}
////////////////////////////////////////////////////////////////////////////

int CRestWorker::free_network_manager(QNetworkAccessManager* nam) {
  if (nam) delete nam;
  return 0;
}

void CRestWorker::pre_handle_reply(const QNetworkReply* reply, int& http_code,
                                   int& err_code, int& network_error) {
  bool parsed = false;
  http_code =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&parsed);
  err_code = RE_SUCCESS;
  network_error = 0;
  if (reply->error() != QNetworkReply::NoError) {
    network_error = reply->error();
    err_code = RE_NETWORK_ERROR;
    qCritical(
        "Send request network error : %s",
        reply->errorString().toStdString().c_str());
  }
}
////////////////////////////////////////////////////////////////////////////

QJsonDocument CRestWorker::qjson_doc_from_arr(const QByteArray& arr,
                                              int& err_code) {
  QJsonDocument doc = QJsonDocument::fromJson(arr);
  if (doc.isNull()) {
    if (err_code != RE_NETWORK_ERROR && err_code != RE_TIMEOUT &&
        QString(arr) != "[]") {
      err_code = RE_NOT_JSON_DOC;
    }
    return QJsonDocument();
  }
  return doc;
}

////////////////////////////////////////////////////////////////////////////

QNetworkReply* CRestWorker::get_reply(QNetworkAccessManager* nam,
                                      QNetworkRequest& req) {
  req.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                   QNetworkRequest::AlwaysNetwork);
  if (nam->networkAccessible() != QNetworkAccessManager::Accessible) {
    qCritical("Network isn't accessible : %d",
                                          (int)nam->networkAccessible());
    nam->setNetworkAccessible(QNetworkAccessManager::Accessible);
  }
  QNetworkReply* reply = nam->get(req);
  reply->ignoreSslErrors();
  return reply;
}

QNetworkReply* CRestWorker::post_reply(QNetworkAccessManager* nam,
                                       const QByteArray& data,
                                       QNetworkRequest& req) {
  req.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                   QNetworkRequest::AlwaysNetwork);
  if (nam->networkAccessible() != QNetworkAccessManager::Accessible) {
    qCritical("Network isn't accessible : %d",
                                          (int)nam->networkAccessible());
    nam->setNetworkAccessible(QNetworkAccessManager::Accessible);
  }
  QNetworkReply* reply = nam->post(req, data);
  reply->ignoreSslErrors();
  return reply;
}

QNetworkReply* CRestWorker::delete_reply(QNetworkAccessManager *nam,
                                         QNetworkRequest &req) {
  req.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                   QNetworkRequest::AlwaysNetwork);
  if (nam->networkAccessible() != QNetworkAccessManager::Accessible) {
    qCritical("Network isn't accessible : %d",
                                          (int)nam->networkAccessible());
    nam->setNetworkAccessible(QNetworkAccessManager::Accessible);
  }
  QNetworkReply* reply = nam->deleteResource(req);
  reply->ignoreSslErrors();
  return reply;
}
////////////////////////////////////////////////////////////////////////////

QByteArray CRestWorker::send_request(QNetworkAccessManager* nam,
                                     QNetworkRequest& req, int get,
                                     int& http_status_code, int& err_code,
                                     int& network_error, QByteArray data,
                                     bool show_network_err_msg, uint timeout_time) {
  req.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                   QNetworkRequest::AlwaysNetwork);
  if (nam->networkAccessible() != QNetworkAccessManager::Accessible) {
    qCritical("Network isn't accessible : %d",
                                          (int)nam->networkAccessible());
    nam->setNetworkAccessible(QNetworkAccessManager::Accessible);
  }

  err_code = RE_SUCCESS;
  network_error = 0;
  http_status_code = -1;

  QEventLoop* loop = new QEventLoop;
  QTimer* timer = new QTimer;
  timer->setSingleShot(true);
  if(timeout_time == 0 || timeout_time == 30000){
    timer->start(30000);
  }
  else {
      timer->start(timeout_time);
  }

  QNetworkReply* reply;
  switch (get) {
    case 1:
        reply = get_reply(nam, req);
        break;
    case 0:
        reply = post_reply(nam, data, req);
        break;
    case 3:
        reply = delete_reply(nam, req);
        break;
  }

  QObject::connect(timer, &QTimer::timeout, loop, &QEventLoop::quit);
  QObject::connect(reply, &QNetworkReply::finished, loop, &QEventLoop::quit);
  QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, loop, &QEventLoop::quit);

  reply->ignoreSslErrors();

  loop->exec();
  loop->deleteLater();

  // timer is active if timeout didn't fire
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
    qCritical(
        "Send request network error : %s",
        reply->errorString().toStdString().c_str());
    if (show_network_err_msg)
      CNotificationObserver::Error(tr(reply->errorString().toStdString().c_str()), DlgNotification::N_NO_ACTION);
    err_code = RE_NETWORK_ERROR;
    bool parsed = false;
    http_status_code =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute)
            .toInt(&parsed);
    reply->deleteLater();
    return QByteArray();
  }

  bool parsed = false;
  http_status_code =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&parsed);
  QByteArray res = reply->readAll();
  reply->deleteLater();
  return res;
}
////////////////////////////////////////////////////////////////////////////
