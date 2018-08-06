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
#include "RestContainers.h"
#include "PeerController.h"

typedef enum rest_error {
  RE_SUCCESS = 0,
  RE_HTTP,
  RE_LOGIN_OR_EMAIL,
  RE_TIMEOUT,
  RE_NOT_JSON_DOC,
  RE_NOT_JSON_OBJECT,
  RE_NETWORK_ERROR,
  RE_FAILED_ERROR,
  RE_NO_UPDATES
} rest_error_t;
////////////////////////////////////////////////////////////////////////////

class CRestWorker : public QObject {
  Q_OBJECT

private:
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

  static QNetworkReply* delete_reply(QNetworkAccessManager* nam,
                                    QNetworkRequest &req);

  QByteArray send_request(QNetworkAccessManager *nam,
      QNetworkRequest &req,
      int get,
      int& http_status_code,
      int& err_code,
      int &network_error,
      QByteArray data,
      bool show_network_err_msg,
      uint timeout_time = 0);

  CRestWorker();
  ~CRestWorker(void);

private slots:
  void get_my_peers_finished_sl();
  void get_environments_finished_sl();
  void get_balance_finished_sl();
  void check_if_ss_console_is_ready_finished_sl();
  void get_p2p_status_finished_sl();

signals:

  void on_get_my_peers_finished(std::vector<CMyPeerInfo>,
                                int http_code,
                                int err_code,
                                int network_error);

  void on_get_p2p_status_finished(std::vector<CP2PInstance>,
                                int http_code,
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
  static CRestWorker* Instance() {
    static CRestWorker instance;
    return &instance;
  }

  void login(const QString& login,
             const QString& password,
             int &http_code,
             int &err_code,
             int &network_error);

  void peer_register(const QString& port,
                     const QString& token,
                     const QString& login,
                     const QString& password,
                     const QString& peer_name,
                     const QString& peer_scope,
                     int &err_code,
                     int &http_code,
                     int &network_error);

  void peer_unregister(const QString& port,
                       const QString& token,
                       int &err_code,
                       int &http_code,
                       int &network_error);


  void peer_token(const QString& port,
                  const QString& login,
                  const QString& password,
                  QString& token,
                  int &err_code,
                  int &http_code,
                  int &network_error);

  void peer_finger(const QString& port,
                   CPeerController::peer_info_t type,
                   QString name,
                   QString dir);

  bool peer_set_pass(const QString& port,
                     const QString& username,
                     const QString& old_pass,
                     const QString& new_pass);

  void peer_get_info(const QString& port,
                     QString peer_info_type,
                     CPeerController::peer_info_t type,
                     QString name,
                     QString dir);

  bool peer_login(const QString& port,
                  const QString& username,
                  const QString& pass);

  bool peer_update_management(const QString& port);

  bool get_user_info(QString user_info_type, QString& user_info_str);

  void update_my_peers();
  void update_p2p_status();

  void update_environments();
  void update_balance();

  std::vector<CGorjunFileInfo> get_gorjun_file_info(const QString& file_name, QString link = "");


  QString get_vagrant_plugin_cloud_version(const QString& plugin_name);

  QString get_vagrant_box_cloud_version(const QString& box_name,
                                        const QString& box_provider);

  void check_if_ss_console_is_ready(const QString& url);

  void send_health_request(const QString &p2p_version,
                           const QString &p2p_status);

  QNetworkReply* download_gorjun_file(const QString& file_id, QString link = "");
  QNetworkReply* download_file(const QUrl& url);

  static const QString& rest_err_to_str(rest_error_t err);

  std::vector<uint8_t> is_sshkeys_in_environment(const QStringList &keys,
                                              const QString& env);

  void add_sshkey_to_environments(const QString &key_name,
                                  const QString& key,
                                  const std::vector<QString>& lst_environments);

  void remove_sshkey_from_environments(const QString &key_name,
                                       const QString &key,
                                       const std::vector<QString> &lst_environments);
};

#endif // CRESTWORKER_H
