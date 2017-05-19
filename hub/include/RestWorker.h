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
#include "EventLoop.h"

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

private:
  CEventLoop<SynchroPrimitives::CPthreadMRE> *m_el;
  QNetworkAccessManager *m_network_manager;

  static QNetworkAccessManager* create_network_manager();
  static int free_network_manager(QNetworkAccessManager*nam);

  static void pre_handle_reply(const QNetworkReply *reply,
                               int &http_code,
                               int &err_code,
                               int &network_error);

  static QJsonDocument qjson_doc_from_arr(const QByteArray& arr,
                                          int &err_code);

  static QNetworkReply* get_reply(QNetworkAccessManager* nam,
                                  QNetworkRequest &req);

  static QNetworkReply* post_reply(QNetworkAccessManager* nam,
                                   const QByteArray &data,
                                   QNetworkRequest &req);

  QByteArray send_request(QNetworkAccessManager *nam,
      QNetworkRequest &req,
      bool get,
      int& http_status_code,
      int& err_code,
      int &network_error,
      QByteArray data,
      bool show_network_err_msg);

  CRestWorker();
  ~CRestWorker(void);

private slots:
  void login_finished_sl();
  void get_environments_finished_sl();
  void get_balance_finished_sl();
  void check_if_ss_console_is_ready_finished_sl();

signals:

  void on_login_finished(int http_code,
                         int err_code,
                         int network_error);

  void on_get_environments_finished(std::vector<CEnvironment>,
                                    int http_code,
                                    int err_code,
                                    int network_error);

  void on_get_balance_finished(CHubBalance,
                               int http_code,
                               int err_code,
                               int network_error);  

  void on_got_ss_console_readiness(bool is_ready,
                                   QString err);

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

  void update_environments();
  void update_balance();

  std::vector<CGorjunFileInfo> get_gorjun_file_info(const QString& file_name);

  void check_if_ss_console_is_ready(const QString& url);

  void send_health_request(const QString &p2p_version,
                           const QString &p2p_status);

  QNetworkReply* download_gorjun_file(const QString& file_id);
  QNetworkReply* download_file(const QUrl& url);

  static const QString& rest_err_to_str(rest_error_t err);

  std::vector<bool> is_sshkeys_in_environment(const QStringList &keys,
                                              const QString& env);

  void add_sshkey_to_environments(const QString &key_name,
                                  const QString& key,
                                  const std::vector<QString>& lst_environments);
};

#endif // CRESTWORKER_H
