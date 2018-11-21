#include <QApplication>
#include <QEventLoop>
#include <QNetworkProxy>
#include <QTimer>

#include "DlgNotification.h"
#include "SettingsManager.h"
#include "Locker.h"
#include "NotificationObserver.h"
#include "OsBranchConsts.h"
#include "RestWorker.h"

CRestWorker::CRestWorker() {
  m_network_manager = create_network_manager();
  m_network_manager->setProxy(QNetworkProxy::NoProxy);
}

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

  if (obj["code"].toInt() == 0) {
    QJsonArray doc_arr = obj["instances"].toArray();
    for (auto i : doc_arr) {
      if (i.isNull() || !i.isObject()) continue;
      lst_res.push_back(CP2PInstance(i.toObject()));
    }
  } else {
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
  qDebug() << "Is reply null " << (reply == nullptr);

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
  qDebug() << "Is reply null " << (reply == nullptr);
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
  qDebug() << "Is reply null " << (reply == nullptr);
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
      query_login.toString(QUrl::FullyEncoded).toUtf8(), false);
  qDebug() << "Http code " << http_code << "Error code " << err_code
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
void CRestWorker::peer_token(const QString& url_management, const QString& login,
                             const QString& password, QString& token,
                             int& err_code, int& http_code,
                             int& network_error) {
  qInfo() << tr("Getting token of %1").arg(url_management);

  const QString str_url(
      QString("https://%1/rest/v1/identity/gettoken").arg(url_management));

  QUrl url_login(str_url);
  QUrlQuery query_login;

  query_login.addQueryItem("username", login);
  query_login.addQueryItem("password", password);

  QNetworkRequest request(url_login);
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    "application/x-www-form-urlencoded");
  QByteArray arr = send_request(
      m_network_manager, request, false, http_code, err_code, network_error,
      query_login.toString(QUrl::FullyEncoded).toUtf8(), false, 60000);

  qDebug() << "Http code " << http_code << "Error code " << err_code
           << "Network Error " << network_error;

  token = QString(arr);
  if (err_code != RE_SUCCESS) {
    if (network_error == 204) err_code = RE_LOGIN_OR_EMAIL;
    return;
  }

  if (QString(arr) == "") {
    err_code = RE_NETWORK_ERROR;
    return;
  }
}

bool CRestWorker::peer_update_management(const QString &port){
  qInfo() << "Request to update management %1" << port
          << " provider: " << VagrantProvider::Instance()->CurrentVal();
  QString port_or_ip;

  if (VagrantProvider::Instance()->UseIp()) {
    port_or_ip = QString("%1:8443").arg(port);
  } else{
    port_or_ip = QString("localhost:%1").arg(port);
  }

  const QString str_url(
      QString("https://%1/rest/v1/system/update_management").arg(port_or_ip));

  QUrl url_login(str_url);
  QNetworkRequest request(url_login);

  int http_code, err_code, network_error;

  send_request(m_network_manager, request, false,
              http_code, err_code, network_error,
              QByteArray(), true, 24 * 3600000); // 24 hours for update

  qDebug () << "Update peer management is finished with "
            << "Error code: " << err_code
            << "Network error" << network_error
            << "Http code" << http_code
            << "ip " << port_or_ip;

  if (err_code != RE_SUCCESS) {
    if (http_code == 500) err_code = RE_FAILED_ERROR;
    return false;
  }
  return true;
}

void CRestWorker::peer_unregister(const QString& url_management, const QString& token,
                                  int& err_code, int& http_code,
                                  int& network_error, QString& body) {
  qInfo() << tr("Unregistering peer %1").arg(url_management);

  const QString str_url(
      QString("https://%1/rest/v1/hub/unregister?sptoken=%2")
          .arg(url_management, token));

  QUrl url_login(str_url);
  QNetworkRequest request(url_login);

  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QByteArray arr =
      send_request(m_network_manager, request, 3, http_code, err_code,
                   network_error, QByteArray(), false, 60000);
  body = QString(arr);
  qDebug() << "peer unregister body " << body
           << "Http code " << http_code << "Error code " << err_code
           << "Network Error " << network_error;
}

void CRestWorker::peer_register(const QString& url_management, const QString& token,
                                const QString& login, const QString& password,
                                const QString& peer_name,
                                const QString& peer_scope, int& err_code,
                                int& http_code, int& network_error, QString& body) {
  qInfo() << tr("Registering peer %1").arg(url_management);

  const QString str_url(
      QString("https://%1/rest/v1/hub/register?sptoken=%2")
          .arg(url_management, token));
  QUrl url_login(str_url);
  QUrlQuery query;

  query.addQueryItem("email", login);
  query.addQueryItem("password", password);
  query.addQueryItem("peerName", peer_name);
  query.addQueryItem("peerScope", peer_scope);

  QNetworkRequest request(url_login);
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    "application/x-www-form-urlencoded");
  QByteArray arr = send_request(
      m_network_manager, request, false, http_code, err_code, network_error,
      query.toString(QUrl::FullyEncoded).toUtf8(), false, 60000);
  body = QString(arr);

  qDebug() << "body " << body
           << "Http code " << http_code
           << "Error code " << err_code
           << "Network Error " << network_error
           << "URL " << str_url;
}

void CRestWorker::peer_finger(const QString& url_management,
                              CPeerController::peer_info_t type,
                              QString name,
                              QString dir) {
  qInfo() << tr("Getting finger from %1").arg(url_management);

  const QString str_url(QString("https://%1/rest/v1/security/keyman/"
                                "getpublickeyfingerprint")
                            .arg(url_management));
  QUrl url_finger(str_url);
  QNetworkRequest req(url_finger);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  QNetworkReply* reply = get_reply(m_network_manager, req);
  reply->ignoreSslErrors();

  QTimer* timer = new QTimer(this);
  timer->setInterval(10000);
  timer->setSingleShot(true);
  timer->start();

  connect(timer, &QTimer::timeout, [reply]() {
    if (reply) reply->close();
  });
  connect(reply, &QNetworkReply::finished, timer, &QTimer::stop);
  connect(reply, &QNetworkReply::finished, timer, &QTimer::deleteLater);
  connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
          reply, &QNetworkReply::close);
  connect(reply, &QNetworkReply::finished,
          reply, &QNetworkReply::deleteLater);

  connect(reply, &QNetworkReply::finished, [reply, type, name, dir](){
    qDebug() << "Is reply null " << (reply == nullptr);
    if (reply == nullptr) {
      return;
    }
    int http_code, err_code, network_error;
    pre_handle_reply(reply, http_code, err_code, network_error);
    QString finger = "undefined";
    QByteArray arr = reply->readAll();
    try {
      finger = QString(arr);
    } catch (...) {
      finger = "undefined";
    }
    CPeerController::Instance()->parse_peer_info(type, name, dir, finger);
  });
}

void CRestWorker::peer_set_pass(const QString& url_mangement, const QString& username,
                                const QString& old_pass,
                                const QString& new_pass) {
  qInfo() << tr("Setting password for %1").arg(url_mangement);

  const QString str_url(QString("https://%1/login").arg(url_mangement));
  QUrl url_finger(str_url);
  QUrlQuery query;
  query.addQueryItem("username", username);
  query.addQueryItem("password", old_pass);
  query.addQueryItem("newpassword", new_pass);
  QNetworkRequest request(url_finger);
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    "application/x-www-form-urlencoded");
  QNetworkReply *reply = post_reply(m_network_manager, query.toString(QUrl::FullyEncoded).toUtf8(), request);
  reply->ignoreSslErrors();

  QTimer* timer = new QTimer(this);
  timer->setInterval(10000);
  timer->setSingleShot(true);
  timer->start();

  connect(timer, &QTimer::timeout, [reply]() {
    if (reply) reply->close();
  });
  connect(reply, &QNetworkReply::finished, timer, &QTimer::stop);
  connect(reply, &QNetworkReply::finished, timer, &QTimer::deleteLater);
  connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
  connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
          reply, &QNetworkReply::close);
}

void CRestWorker::peer_get_info(const QString& url_management, QString peer_info_type,
                                CPeerController::peer_info_t type,
                                QString name,
                                QString dir) {
  // create request
  const QString str_url(QString("https://%1/rest/v1/system/management_updates").arg(url_management));
  QUrl url_login(str_url);
  QNetworkRequest request(url_login);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  QNetworkReply* reply = get_reply(m_network_manager, request);
  reply->ignoreSslErrors();

  QTimer* timer = new QTimer(this);
  timer->setInterval(10000);
  timer->setSingleShot(true);
  timer->start();

  connect(timer, &QTimer::timeout, [reply]() {
    if (reply) reply->close();
  });
  connect(reply, &QNetworkReply::finished, timer, &QTimer::stop);
  connect(reply, &QNetworkReply::finished, timer, &QTimer::deleteLater);
  connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
  connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
          reply, &QNetworkReply::close);

  connect(reply, &QNetworkReply::finished, [reply, url_management, type, name, dir, peer_info_type]() {
    qDebug() << "Is reply null " << (reply == nullptr);
    if (reply == nullptr) {
      return;
    }
    int http_code, err_code, network_error;
    pre_handle_reply(reply, http_code, err_code, network_error);
    QByteArray arr = reply->readAll();
    // get json file
    QJsonDocument doc = QJsonDocument::fromJson(arr);
    QString res = "false";
    if (doc.isNull() || doc.isEmpty() || !doc.isObject()) {
      qCritical() << "couldn't get json file from" << name;
      // if user is not authorized
      if (http_code == 401 || QString(arr) == "User is not authorized") {
        // set password if not changed
        QString old_pass = "secret", user_name = "admin";
        QString new_pass = CSettingsManager::Instance().peer_pass(name);
        CRestWorker::Instance()->peer_set_pass(url_management, user_name, old_pass, new_pass);
        // login
        CRestWorker::Instance()->peer_login(url_management, user_name, new_pass);
      }
    } else {
      // get value from json file
      QJsonObject obj = doc.object();
      if (obj.find(peer_info_type) != obj.end()) {
        if (obj[peer_info_type].toBool()) {
          res = "true";
        } else {
          res = "false";
        }
      }
    }
    CPeerController::Instance()->parse_peer_info(type, name, dir, res);
  });
}

void CRestWorker::peer_login(const QString &url_management,
                             const QString &username,
                             const QString &pass){
  const QString str_url(QString("https://%1/login").arg(url_management));
  QUrl url_finger(str_url);
  QUrlQuery query;
  query.addQueryItem("username", username);
  query.addQueryItem("password", pass);
  QNetworkRequest request(url_finger);
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    "application/x-www-form-urlencoded");
  QNetworkReply *reply = post_reply(m_network_manager, query.toString(QUrl::FullyEncoded).toUtf8(), request);
  reply->ignoreSslErrors();

  QTimer* timer = new QTimer(this);
  timer->setInterval(6000);
  timer->setSingleShot(true);
  connect(timer, &QTimer::timeout, [reply]() {
    if (reply) reply->close();
  });
  timer->start();

  connect(reply, &QNetworkReply::finished, timer, &QTimer::stop);
  connect(reply, &QNetworkReply::finished, timer, &QTimer::deleteLater);
  connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
  connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
          reply, &QNetworkReply::close);
}

////////////////////////////////////////////////////////////////////////////

bool CRestWorker::get_user_info(QString user_info_type,
                                QString& user_info_str) {
  int http_code, err_code, network_error;
  user_info_str = "";
  static const QString str_url(hub_get_url().arg("user-info"));
  QUrl url_login(str_url);
  QNetworkRequest request(url_login);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  QByteArray arr = send_request(m_network_manager, request, true, http_code,
                                err_code, network_error, QByteArray(), true);

  QJsonDocument doc = QJsonDocument::fromJson(arr);
  qDebug() << "Json file: " << doc;
  if (doc.isNull() || doc.isEmpty() || !doc.isObject()) {
    qCritical("Get user %s failed. URL : %s",
              user_info_type.toStdString().c_str(),
              str_url.toStdString().c_str());
    return false;
  }

  QJsonObject obj = doc.object();
  if (obj.find(user_info_type) != obj.end()){
    if (user_info_type == "id")
      user_info_str = QString("%1").arg(obj[user_info_type].toInt());
    else
      user_info_str = QString("%1").arg(obj[user_info_type].toString());
  }
  return true;
}
////////////////////////////////////////////////////////////////////////////

void CRestWorker::update_my_peers() {
  QUrl url_env(hub_get_url().arg("my-peers"));
  QNetworkRequest req(url_env);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  QNetworkReply* reply = get_reply(m_network_manager, req);
  reply->ignoreSslErrors();
  QTimer* timer = new QTimer(this);
  timer->setInterval(60000);
  timer->setSingleShot(true);

  connect(timer, &QTimer::timeout, [reply]() {
    CNotificationObserver::Instance()->Info(
        "Connection timeout, can't connect to bazaar",
        DlgNotification::N_NO_ACTION);
    if (reply) reply->close();
  });

  timer->start();

  connect(reply, &QNetworkReply::finished, timer, &QTimer::stop);
  connect(reply, &QNetworkReply::finished, timer, &QTimer::deleteLater);
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

  QTimer* timer = new QTimer(this);
  timer->setInterval(60000);
  timer->setSingleShot(true);

  connect(timer, &QTimer::timeout, [reply]() {
    CNotificationObserver::Instance()->Info(
        "Connection timeout, can't connect to bazaar",
        DlgNotification::N_NO_ACTION);
    if (reply) reply->close();
  });

  timer->start();

  connect(reply, &QNetworkReply::finished, timer, &QTimer::stop);
  connect(reply, &QNetworkReply::finished, timer, &QTimer::deleteLater);
  connect(reply, &QNetworkReply::finished, this,
          &CRestWorker::get_environments_finished_sl);
  connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
}
////////////////////////////////////////////////////////////////////////////

void CRestWorker::update_balance() {
  qDebug() << "Getting the balance data from hub";
  QUrl url_env(hub_get_url().arg("balance"));
  QNetworkRequest req(url_env);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  QNetworkReply* reply = get_reply(m_network_manager, req);
  reply->ignoreSslErrors();

  QTimer* timer = new QTimer(this);
  timer->setInterval(60000);
  timer->setSingleShot(true);

  connect(timer, &QTimer::timeout, [reply]() {
    CNotificationObserver::Instance()->Info(
        "Connection timeout, can't connect to bazaar",
        DlgNotification::N_NO_ACTION);
    if (reply) reply->close();
  });

  timer->start();

  connect(reply, &QNetworkReply::finished, timer, &QTimer::stop);
  connect(reply, &QNetworkReply::finished, timer, &QTimer::deleteLater);
  connect(reply, &QNetworkReply::finished, this,
          &CRestWorker::get_balance_finished_sl);
  connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
}

////////////////////////////////////////////////////////////////////////////

std::vector<CGorjunFileInfo> CRestWorker::get_gorjun_file_info(
    const QString& file_name, QString link) {
  int http_code, err_code, network_error;
  if (link.isEmpty()) {
    link = hub_gorjun_url();
  }
  link += QString("?name=%1&latest").arg(file_name);
  QUrl url_gorjun_fi(link);
  QNetworkRequest request(url_gorjun_fi);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  QByteArray arr = send_request(m_network_manager, request, true, http_code,
                                err_code, network_error, QByteArray(), true);
  QJsonDocument doc = QJsonDocument::fromJson(arr);
  qDebug() << "Requested filename: " << file_name << "Json file: " << doc;

  std::vector<CGorjunFileInfo> lst_res;
  if (doc.isNull()) {
    err_code = RE_NOT_JSON_DOC;
    return lst_res;
  }

  if (doc.isObject()) {
    QJsonObject json_obj = doc.object();
    QStringList dbg;
    QJsonObject obj = QJsonObject();

    obj.insert("id", json_obj["id"]);
    dbg << QString("id=%1").arg(json_obj["id"].toString());
    obj.insert("filename", json_obj["filename"]);
    dbg << QString("filename=%1").arg(json_obj["filename"].toString());

    QJsonObject tmp_obj = json_obj["raw_details"].toArray().begin()->toObject();

    obj.insert("md5", tmp_obj["md5"]);
    dbg << QString("md5=%1").arg(tmp_obj["md5"].toString());
    obj.insert("size", tmp_obj["size"]);
    dbg << QString("size=%1").arg(tmp_obj["size"].toInt());

    qDebug() << "Parsed json doc:" << dbg;
    lst_res.emplace_back(CGorjunFileInfo(obj));
  }

  return lst_res;
}
////////////////////////////////////////////////////////////////////////////

QString CRestWorker::get_vagrant_plugin_cloud_version(
    const QString& plugin_name) {
  int http_code, err_code, network_error;
  QUrl url_gorjun_fi(
      QString("https://rubygems.org/api/v1/versions/%1/latest.json")
          .arg(plugin_name));
  QNetworkRequest request(url_gorjun_fi);
  QByteArray arr = send_request(m_network_manager, request, true, http_code,
                                err_code, network_error, QByteArray(), false);
  QJsonDocument doc = QJsonDocument::fromJson(arr);

  qDebug() << "Requested plugin version: " << plugin_name
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

QString CRestWorker::get_vagrant_box_cloud_version(
    const QString& box_name, const QString& box_provider) {
  QStringList parsed_name = box_name.split("/");
  if (parsed_name.size() != 2) {
    return QString("undefined");
  }
  int http_code, err_code, network_error;
  QUrl url_gorjun_fi(QString("https://app.vagrantup.com/api/v1/box/%1/%2")
                         .arg(parsed_name[0], parsed_name[1]));
  QNetworkRequest request(url_gorjun_fi);
  QByteArray arr = send_request(m_network_manager, request, true, http_code,
                                err_code, network_error, QByteArray(), false);
  QJsonDocument doc = QJsonDocument::fromJson(arr);

  qDebug() << "Requested box version: " << box_name << "Json file: " << doc;

  if (doc.isNull()) {
    err_code = RE_NOT_JSON_DOC;
    return QString("undefined");
  }
  if (doc.isObject()) {
    QJsonArray all_versions = doc.object()["versions"].toArray();
    for (QJsonArray::iterator it1 = all_versions.begin();
         it1 != all_versions.end(); it1++) {
      QJsonObject version = it1->toObject();
      QJsonArray providers = version["providers"].toArray();
      for (QJsonArray::iterator it2 = providers.begin(); it2 != providers.end();
           it2++) {
        QJsonObject provider = it2->toObject();
        if (provider["name"].toString() == box_provider) {
          return version["version"].toString();
        }
      }
    }
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

QNetworkReply* CRestWorker::download_gorjun_file(const QString& file_name,
                                                 QString link) {
  UNUSED_ARG(file_name);
  if (link.isEmpty()) link = hub_gorjun_url();
  QString str_file_url =
      QString("%1").arg(link);
  QUrl url(str_file_url);
  return download_file(url);
}
////////////////////////////////////////////////////////////////////////////

QNetworkReply* CRestWorker::download_file(const QUrl& url) {
  QNetworkRequest req(url);
  //req.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
  //                 QNetworkRequest::AlwaysNetwork);
  QNetworkReply* reply = m_network_manager->get(req);
  reply->ignoreSslErrors();
  return reply;
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
  QString ssh_key_md5 = CCommons::FileMd5(CSettingsManager::Instance().ssh_keys_storage() + QDir::separator() + key_name);
  static const QString str_url(hub_post_url().arg("environments/ssh-keys"));
  QJsonObject obj;
  QJsonArray arr_environments;
  for (auto i = lst_environments.begin(); i != lst_environments.end(); ++i)
    arr_environments.push_back(QJsonValue(*i));

  obj["key_name"] = ssh_key_md5 + key_name;
  obj["sshKey"] = QJsonValue(key);
  obj["environments"] = arr_environments;

  QJsonDocument doc(obj);
  QByteArray doc_serialized = doc.toJson();
  QUrl url(str_url);
  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  int http_status_code, err_code, network_error;
  send_request(m_network_manager, req, false,
               http_status_code, err_code, network_error, doc_serialized, false);
  qDebug() << "finished to add ssh keys"
           << "json doc: " << doc
           << "http code" << http_status_code
           << "network code" << network_error
           << "error code" << err_code;
  if (err_code != RE_SUCCESS) {
    if (http_status_code == 500) {
      CNotificationObserver::Instance()->Error(tr("Failed to deploy SSH key %1 to the environments. "
                                                  "Mostly it's because some of the environments already has SSH key with "
                                                  "the same name or value. Try to change name of SSH key or generate a new one.").arg(key_name),
                                               DlgNotification::N_NO_ACTION);
    } else {
      CNotificationObserver::Instance()->Error(tr("Unable to deploy SSH key %2 to the environments."
                                                  "Network error message: %1").arg(CCommons::NetworkErrorToString(network_error), key_name),
                                               DlgNotification::N_NO_ACTION);
    }
  }
}
////////////////////////////////////////////////////////////////////////////

void CRestWorker::remove_sshkey_from_environments(
    const QString& key_name, const QString& key,
    const QStringList& lst_environments) {
  QString ssh_key_md5 = CCommons::FileMd5(CSettingsManager::Instance().ssh_keys_storage() + QDir::separator() + key_name);
  static const QString str_url(hub_post_url().arg("environments/remove-keys"));
  QJsonObject obj;
  QJsonArray arr_environments;
  for (auto i = lst_environments.begin(); i != lst_environments.end(); ++i)
    arr_environments.push_back(QJsonValue(*i));

  obj["key_name"] = ssh_key_md5 + key_name;
  obj["sshKey"] = QJsonValue(key);
  obj["environments"] = arr_environments;

  QJsonDocument doc(obj);
  QByteArray doc_serialized = doc.toJson();
  QUrl url(str_url);
  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  int http_status_code, err_code, network_error;
  send_request(m_network_manager, req, false, http_status_code, err_code,
               network_error, doc_serialized, true);
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
    qCritical("Send request network error : %s",
              reply->errorString().toStdString().c_str());
  }
}
////////////////////////////////////////////////////////////////////////////

QJsonDocument CRestWorker::qjson_doc_from_arr(const QByteArray& arr,
                                              int& err_code) {
  QJsonParseError error;
  QJsonDocument doc = QJsonDocument::fromJson(arr, &error);
  if (error.error != QJsonParseError::NoError) {
    qCritical() << QString("Failed to convert json document. Error message: %1")
                   .arg(error.errorString());
    err_code = RE_NOT_JSON_DOC;
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
    qCritical("Network isn't accessible : %d", (int)nam->networkAccessible());
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
    qCritical("Network isn't accessible : %d", (int)nam->networkAccessible());
    nam->setNetworkAccessible(QNetworkAccessManager::Accessible);
  }
  QNetworkReply* reply = nam->post(req, data);
  reply->ignoreSslErrors();
  return reply;
}

QNetworkReply* CRestWorker::delete_reply(QNetworkAccessManager* nam,
                                         QNetworkRequest& req) {
  req.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                   QNetworkRequest::AlwaysNetwork);
  if (nam->networkAccessible() != QNetworkAccessManager::Accessible) {
    qCritical("Network isn't accessible : %d", (int)nam->networkAccessible());
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
                                     bool show_network_err_msg,
                                     uint timeout_time) {
  req.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                   QNetworkRequest::AlwaysNetwork);
  if (nam->networkAccessible() != QNetworkAccessManager::Accessible) {
    qCritical("Network isn't accessible : %d", (int)nam->networkAccessible());
    nam->setNetworkAccessible(QNetworkAccessManager::Accessible);
  }

  err_code = RE_SUCCESS;
  network_error = 0;
  http_status_code = -1;

  QEventLoop* loop = new QEventLoop(this);
  QTimer* timer = new QTimer(this);
  timer->setSingleShot(true);
  if (timeout_time == 0 || timeout_time == 30000) {
    timer->start(30000);
  } else {
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
  QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
                   loop, &QEventLoop::quit);
  QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
                   reply, &QNetworkReply::close);

  reply->ignoreSslErrors();

  loop->exec();
  loop->deleteLater();

  // timer is active if timeout didn't fire
  if (!timer->isActive()) {
    reply->abort();
    err_code = RE_TIMEOUT;
    reply->deleteLater();
    timer->deleteLater();
    return reply->readAll();
  }

  timer->stop();
  timer->deleteLater();
  if (reply->error() != QNetworkReply::NoError) {
    network_error = reply->error();
    qCritical("Send request network error : %s",
              reply->errorString().toStdString().c_str());
    if (show_network_err_msg)
      CNotificationObserver::Error(
          tr(reply->errorString().toStdString().c_str()),
          DlgNotification::N_NO_ACTION);
    err_code = RE_NETWORK_ERROR;
    bool parsed = false;
    http_status_code =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute)
            .toInt(&parsed);
    reply->deleteLater();
    return reply->readAll();
  }

  bool parsed = false;
  http_status_code =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&parsed);
  QByteArray res = reply->readAll();
  reply->deleteLater();
  return res;
}
////////////////////////////////////////////////////////////////////////////
