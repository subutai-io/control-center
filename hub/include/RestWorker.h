#ifndef RESTWORKER_H
#define RESTWORKER_H

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

/*!
 * \brief This class contains methods and functions for working with HUB REST
 */
class CRestWorker : public QObject {
  Q_OBJECT
public:
  /*!
   * \brief Instance of this singleton class
   */
  static CRestWorker* Instance() {
    static CRestWorker instance;
    return &instance;
  }

  void login(const QString& login,
             const QString& password,
             int &http_code,
             int &err_code,
             int &network_error);

  std::vector<CEnvironment> get_environments(int &http_code,
                                               int& err_code,
                                               int &network_error);

  CHubBalance get_balance(int &http_code,
                         int& err_code,
                         int &network_error);

  std::vector<CRHInfo> get_ssh_containers(int &http_code,
                                          int& err_code,
                                          int &network_error);

  std::vector<CGorjunFileInfo> get_gorjun_file_info(const QString& file_name);

  int is_ss_console_ready(const QString& url,
                          int &err_code,
                          int &network_err);

  void send_health_request(int &http_code,
                           int &err_code,
                           int &network_err,
                           const QString &p2p_version,
                           const QString &p2p_status);

  QNetworkReply* download_gorjun_file(const QString& file_id);
  QNetworkReply* download_file(const QUrl& url);

  static const QString& rest_err_to_str(rest_error_t err);
  void send_ssh_key(const QString &key,
                    int &http_code,
                    int &err_code,
                    int &network_err);

private:
  QNetworkAccessManager *m_network_manager;
  QByteArray send_request(QNetworkRequest &req,
                          bool get,
                          int& http_status_code,
                          int& err_code,
                          int &network_error,
                          QByteArray data,
                          bool ignore_ssl_errors,
                          bool show_network_err_msg);

  QJsonDocument get_request_json_document(const QString& link,
                                          int& http_code,
                                          int &err_code,
                                          int &network_error);

  CRestWorker();
  CRestWorker(const QString& login,
              const QString& password);
  CRestWorker(const CRestWorker& worker);
  ~CRestWorker(void);
 private slots:
  void ssl_errors_appeared(QList<QSslError> lst_errors);
};

#endif // CRESTWORKER_H
