#include <QEventLoop>

#include "RestWorker.h"
#include <QDebug>
#include "SettingsManager.h"

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
  QByteArray arr = post_request(request);
  m_id = QString(arr).mid(1, arr.length()-2); //hack. should be error code and result

  if (m_id == QString("Wrong email or password."))
    return 1;
  return 0;
}
////////////////////////////////////////////////////////////////////////////

QJsonObject CRestWorker::get_request_object(int &err_code, const QString &link) {
  err_code = 0;
  QUrl url_env(CSettingsManager::Instance().get_url().arg(m_id).arg(link));
  QNetworkRequest req(url_env);
  req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
  QByteArray arr = get_request(req);
  QJsonDocument doc  = QJsonDocument::fromJson(arr);
  if (doc.isNull()) {err_code = 1; return QJsonObject();}
  if (!doc.isObject()) {err_code = 2; return QJsonObject();}
  return doc.object();
}

////////////////////////////////////////////////////////////////////////////

std::vector<CSSEnvironment> CRestWorker::get_environments(int& err_code)
{
  std::vector<CSSEnvironment> lst_res;
  QJsonObject obj = get_request_object(err_code, "environments");
  if (err_code != 0) return lst_res;
  QJsonArray env_arr = obj["environments"].toArray();
  for (int i = 0; i < env_arr.count(); ++i) {
    QJsonValue val = env_arr.at(i);
    lst_res.push_back(CSSEnvironment(val.toString()));
  }
  return lst_res;
}
////////////////////////////////////////////////////////////////////////////

CSSContainer CRestWorker::get_containers(int& err_code)
{
  QJsonObject container = get_request_object(err_code, "containers");
  if (err_code != 0)
    return CSSContainer();
  QJsonObject cont_val = container["containers"].toObject();
  return CSSContainer(cont_val);
}
////////////////////////////////////////////////////////////////////////////

CSSPeerUser CRestWorker::get_peer_users(int& err_code)
{
  QJsonObject peer_users = get_request_object(err_code, "peerusers");
  if (err_code != 0) return CSSPeerUser();
  QJsonObject cont_val = peer_users["peerusers"].toObject();
  return CSSPeerUser(cont_val);
}
////////////////////////////////////////////////////////////////////////////

CSSBalance CRestWorker::get_balance(int& err_code)
{
  QJsonObject balance = get_request_object(err_code, "balance");
  if (err_code != 0) return CSSBalance();
  return CSSBalance(balance["currentBalance"].toString());
}
////////////////////////////////////////////////////////////////////////////

QByteArray CRestWorker::send_request(const QNetworkRequest &req, bool get)
{
  static QNetworkAccessManager qnam;
  QEventLoop loop;
  QNetworkReply* reply =
      get ? qnam.get(req) : qnam.post(req, QByteArray());
  QObject::connect(reply, SIGNAL(readyRead()), &loop, SLOT(quit()));
  loop.exec();
  QObject::disconnect(reply, SIGNAL(readyRead()), &loop, SLOT(quit()));
  return reply->readAll();
}

QByteArray CRestWorker::get_request(const QNetworkRequest &req)
{
  return send_request(req, true);
}
////////////////////////////////////////////////////////////////////////////

QByteArray CRestWorker::post_request(const QNetworkRequest &req)
{
  return send_request(req, false);
}
////////////////////////////////////////////////////////////////////////////
