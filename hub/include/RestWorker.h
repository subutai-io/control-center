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

typedef enum rest_error {
  RE_SUCCESS = 0,
  RE_HTTP,
  RE_LOGIN_OR_EMAIL,
  RE_TIMEOUT,
  RE_NOT_JSON_DOC,
  RE_NOT_JSON_OBJECT,
  RE_NETWORK_ERROR
} rest_error_t;
////////////////////////////////////////////////////////////////////////////

class CRestWorker : public QObject {
  Q_OBJECT
public:
  static CRestWorker* Instance() {
    static CRestWorker instance;
    return &instance;
  }
  void create_network_manager();
  void free_network_manager();

  void login(const QString& login,
             const QString& password,
             int &http_code,
             int &err_code,
             int &network_error);
  std::vector<CSSEnvironment> get_environments(int &http_code, int& err_code, int &network_error);
  CSSBalance get_balance(int &http_code, int& err_code, int &network_error);
  std::vector<CRHInfo> get_ssh_containers(int &http_code, int& err_code, int &network_error);
  std::vector<CGorjunFileInfo> get_gorjun_file_info(const QString& file_name);
  int is_ss_console_ready(const QString& url, int &err_code, int &network_err);

  QNetworkReply* download_gorjun_file(const QString& file_id);
  QNetworkReply* download_file(const QUrl& url);

  static const QString& rest_err_to_str(rest_error_t err);

private:
  QNetworkAccessManager *m_network_manager;
  QByteArray send_request(const QNetworkRequest& req, bool get,
                          int &http_status_code, int &err_code, int& network_error, bool ignore_ssl_errors = false);
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

private slots:
  void ssl_errors_appeared(QList<QSslError> lst_errors);
  void ssl_errors_appeared_ignore_them(QList<QSslError> lst_errors);
};

#endif // CRESTWORKER_H
