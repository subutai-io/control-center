#ifndef CRESTWORKER_H
#define CRESTWORKER_H

#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QSsl>
#include <QtNetwork/QSslConfiguration>
#include <QUrl>
#include <QUrlQuery>
#include <QString>
#include <vector>
#include "ApplicationLog.h"
#include "RestContainers.h"

typedef enum error_login {
  EL_SUCCESS = 0,
  EL_HTTP,
  EL_LOGIN_OR_EMAIL,
  EL_TIMEOUT,
  EL_NOT_JSON_DOC,
  EL_NOT_JSON_OBJECT,
  EL_NETWORK_ERROR
} error_login_t;
////////////////////////////////////////////////////////////////////////////

class CRestWorker : public QObject {
  Q_OBJECT

private:
  QNetworkAccessManager m_network_manager;

  QByteArray send_request(const QNetworkRequest& req, bool get,
                          int &http_status_code, int &err_code, int& network_error);
  QByteArray send_get_request(const QNetworkRequest& req, int &http_status_code,
                              int &err_code, int& network_error);
  QByteArray send_post_request(const QNetworkRequest& req, int &http_status_code,
                               int &err_code, int& network_error);
  QJsonDocument get_request_json_document(const QString& link, int& http_code,
                                          int &err_code, int &network_error);

  CRestWorker();
  CRestWorker(const QString& login,
              const QString& password);
  CRestWorker(const CRestWorker& worker);
  ~CRestWorker(void);

public:
  static CRestWorker* Instance() {
    static CRestWorker instance;
    return &instance;
  }

  void login(const QString& login,
             const QString& password,
             int &http_code,
             int &err_code,
             int &network_error);
  std::vector<CSSEnvironment> get_environments(int &http_code, int& err_code, int &network_error);
  CSSBalance get_balance(int &http_code, int& err_code, int &network_error);
  std::vector<CRHInfo> get_ssh_containers(int &http_code, int& err_code, int &network_error);
  std::vector<CGorjunFileInfo> get_gorjun_file_info(const QString& file_name);

  QNetworkReply* download_gorjun_file(const QString& file_id);
  QNetworkReply* download_file(const QUrl& url);

  static const QString& login_err_to_str(error_login_t err);
private slots:
  void ssl_errors_appeared(QList<QSslError> lst_errors);
};

#endif // CRESTWORKER_H
