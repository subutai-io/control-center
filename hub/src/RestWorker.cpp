#include <QTimer>
#include <QEventLoop>
#include <QDebug>
#include "SettingsManager.h"
#include "RestWorker.h"

QString CRestWorker::m_id = "";

int CRestWorker::login(const QString& login, const QString& password)
{
  QUrl url_login(CSettingsManager::Instance().post_url().arg("login"));
  QUrlQuery query_login;
  query_login.addQueryItem("email", login);
  query_login.addQueryItem("password", password);
  url_login.setQuery(query_login);
  QNetworkRequest request(url_login);
  request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

  int http_code;
  int err_code;

  QByteArray arr = post_request(request, http_code, err_code);
  m_id = QString(arr).mid(1, arr.length()-2); //hack.
  return err_code;
}
////////////////////////////////////////////////////////////////////////////

QJsonObject CRestWorker::get_request_object(const QString &link,
                                            int &http_code,
                                            int &err_code) {
  QUrl url_env(CSettingsManager::Instance().get_url().arg(m_id).arg(link));
  QNetworkRequest req(url_env);
  req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
  QByteArray arr = get_request(req, http_code, err_code);
  QJsonDocument doc  = QJsonDocument::fromJson(arr);
  if (doc.isNull()) {err_code = EL_NOT_JSON_DOC; return QJsonObject();}
  if (!doc.isObject()) {err_code = EL_NOT_JSON_OBJECT; return QJsonObject();}
  return doc.object();
}

////////////////////////////////////////////////////////////////////////////

std::vector<CSSEnvironment> CRestWorker::get_environments(int& http_code, int& err_code)
{
  std::vector<CSSEnvironment> lst_res;
  QJsonObject obj = get_request_object("environments", http_code, err_code);
  if (err_code != 0) return lst_res;
  QJsonArray env_arr = obj["environments"].toArray();
  for (int i = 0; i < env_arr.count(); ++i) {
    QJsonValue val = env_arr.at(i);
    lst_res.push_back(CSSEnvironment(val.toString()));
  }
  return lst_res;
}
////////////////////////////////////////////////////////////////////////////

CSSContainer CRestWorker::get_containers(int& http_code, int& err_code)
{
  QJsonObject container = get_request_object("containers", http_code, err_code);
  if (err_code != 0)
    return CSSContainer();
  QJsonObject cont_val = container["containers"].toObject();
  return CSSContainer(cont_val);
}
////////////////////////////////////////////////////////////////////////////

CSSPeerUser CRestWorker::get_peer_users(int& http_code, int& err_code)
{
  QJsonObject peer_users = get_request_object("peerusers", http_code, err_code);
  if (err_code != 0) return CSSPeerUser();
  QJsonObject cont_val = peer_users["peerusers"].toObject();
  return CSSPeerUser(cont_val);
}
////////////////////////////////////////////////////////////////////////////

CSSBalance CRestWorker::get_balance(int& http_code, int& err_code)
{
  QJsonObject balance = get_request_object("balance", http_code, err_code);
  if (err_code != 0) return CSSBalance();
  return CSSBalance(balance["currentBalance"].toString());
}
////////////////////////////////////////////////////////////////////////////

QByteArray CRestWorker::send_request(const QNetworkRequest &req,
                                     bool get,
                                     int& http_status_code,
                                     int& err_code)
{
  err_code = EL_SUCCESS;
  static QNetworkAccessManager qnam;

  QEventLoop loop;
  QTimer timer(&loop);
  timer.setSingleShot(true);
  timer.start(15000);

  QNetworkReply* reply =
      get ? qnam.get(req) : qnam.post(req, QByteArray());

  QObject::connect(reply, SIGNAL(readyRead()), &loop, SLOT(quit()));
  QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
  QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                   &loop, SLOT(quit()));

  loop.exec();

  QObject::disconnect(reply, SIGNAL(readyRead()), &loop, SLOT(quit()));
  QObject::disconnect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
  QObject::disconnect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                      &loop, SLOT(quit()));

  //timer active is timout didn't fire
  if (timer.isActive()) {
    if (reply->error() != QNetworkReply::NoError) {
      err_code = reply->error();
      return QByteArray();
    }

    bool parsed = false;
    http_status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&parsed);
    return reply->readAll();
  } else {
    qDebug() << "timeout";
    reply->abort();
    err_code = EL_TIMEOUT;
    return QByteArray();
  }
}

QByteArray CRestWorker::get_request(const QNetworkRequest &req,
                                    int& http_status_code,
                                    int& err_code)
{
  return send_request(req, true, http_status_code, err_code);
}
////////////////////////////////////////////////////////////////////////////

QByteArray CRestWorker::post_request(const QNetworkRequest &req,
                                     int& http_status_code,
                                     int& err_code)
{
  return send_request(req, false, http_status_code, err_code);
}
////////////////////////////////////////////////////////////////////////////
