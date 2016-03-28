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

typedef enum error_login {
  EL_SUCCESS = 0,
  EL_HTTP = 1,
  EL_LOGIN_OR_EMAIL = 2,
  EL_TIMEOUT = 3,
  EL_NOT_JSON_DOC = 4,
  EL_NOT_JSON_OBJECT = 5,
  EL_NETWORK_ERROR = 6
} error_login_t;

class CHubContainer {
private:
  QString m_name;
  QString m_ip;
  QString m_id;
  QString m_port;

public:
  explicit CHubContainer(QJsonObject obj) :
    m_name(obj["container_name"].toString()),
    m_ip(obj["container_ip"].toString()),
    m_id(obj["container_id"].toString())
  {
    QHostAddress addr(m_ip.split("/")[0]);
    if (!addr.isNull()) {
      quint32 ip = addr.toIPv4Address();
      quint8 port = ip & 0x000000ff; //last octet
      m_port = QString("%1").arg(10000+port);
    }
  }

  ~CHubContainer(){}

  const QString& ip() const { return m_ip;}
  const QString& name() const { return m_name;}
  const QString& id() const {return m_id;}
  const QString& port() const {return m_port;}

  bool operator==(const CHubContainer& arg) const {
    return m_id == arg.m_id;
  }
  bool operator!=(const CHubContainer& arg) const {
    return !(this->operator ==(arg));
  }
};
////////////////////////////////////////////////////////////////////////////

class CSSEnvironment {
private:
  QString m_name;
  QString m_hash;
  QString m_aes_key;
  QString m_ttl;
  QString m_id;
  std::vector<CHubContainer> m_lst_containers;
public:
  CSSEnvironment() : m_name(""){}
  explicit CSSEnvironment(const QJsonObject& obj) {
    m_name = obj["environment_name"].toString();
    m_hash = obj["environment_hash"].toString();
    m_aes_key = obj["environment_key"].toString();
    m_ttl = obj["environment_ttl"].toString();
    m_id = obj["environment_id"].toString();

    QJsonArray arr = obj["environment_containers"].toArray();
    for (int i = 0; i < arr.size(); ++i) {
      QJsonObject item = arr.at(i).toObject();
      m_lst_containers.push_back(CHubContainer(item));
    }
  }

  ~CSSEnvironment(){}

  //todo use id
  bool operator==(const CSSEnvironment& arg) const {
    return m_name == arg.m_name &&
        m_hash == arg.m_hash &&
        m_aes_key == arg.m_aes_key &&
        m_ttl == arg.m_ttl &&
        m_lst_containers == arg.m_lst_containers;
  }

  bool operator!=(const CSSEnvironment& arg) const {
    return !(this->operator ==(arg));
  }

  const QString& name() const {return m_name;}
  const QString& hash() const {return m_hash;}
  const QString& key() const {return m_aes_key;}
  const QString& id() const {return m_id;}
  const std::vector<CHubContainer>& containers() const {return m_lst_containers;}
};
////////////////////////////////////////////////////////////////////////////

class CSSBalance {
private:
  QString m_balance;
public:
  CSSBalance() : m_balance("") {}
  explicit CSSBalance(const QString& balance) : m_balance(balance) {
  }
  ~CSSBalance(){}

  const QString& value() const {return m_balance;}
};
////////////////////////////////////////////////////////////////////////////

class CCHInfo {
private:
  QString m_id;
  QString m_ch_ip;
public:
  explicit CCHInfo(const QJsonObject& obj) {
    m_id = obj["container_id"].toString();
    m_ch_ip = obj["container_ip"].toString();
  }

  const QString& id() const {return m_id;}
  const QString& ch_ip() const {return m_ch_ip;}

  bool operator==(const CCHInfo& arg) const {
    return m_id==arg.m_id;
  }

  bool operator!=(const CCHInfo& arg) const {
    return !(this->operator ==(arg));
  }
};

class CRHInfo {
private:
  QString m_id;
  QString m_rh_ip;
  std::vector<CCHInfo> m_lst_ch;
public:
  explicit CRHInfo(const QJsonObject& obj) {
    m_id = obj["peer_id"].toString();
    m_rh_ip = obj["peer_ip"].toString();

    QJsonArray arr_containers = obj["peer_containers"].toArray();
    for (int i = 0; i < arr_containers.size(); ++i) {
      QJsonValue val = arr_containers.at(i);
      CCHInfo ch(val.toObject());
      m_lst_ch.push_back(ch);
    }
  }

  const QString& id() const {return m_id;}
  const QString& rh_ip() const {return m_rh_ip;}
  const std::vector<CCHInfo>& lst_containers() const {return m_lst_ch;}

  bool operator==(const CRHInfo& arg) const {
    return m_id==arg.m_id;
  }

  bool operator!=(const CRHInfo& arg) const {
    return !(this->operator ==(arg));
  }
};
////////////////////////////////////////////////////////////////////////////

class CRestWorker {
private:

  static QByteArray send_request(const QNetworkRequest& req, bool get,
                                 int &http_status_code, int &err_code, int& network_error);
  static QByteArray send_get_request(const QNetworkRequest& req, int &http_status_code,
                                     int &err_code, int& network_error);
  static QByteArray send_post_request(const QNetworkRequest& req, int &http_status_code,
                                      int &err_code, int& network_error);
  static QJsonDocument get_request_json_document(const QString& link, int& http_code, int &err_code, int &network_error);

  CRestWorker();
  CRestWorker(const QString& login,
              const QString& password);
  CRestWorker(const CRestWorker& worker);
  ~CRestWorker(void);

public:

  static void login(const QString& login,
                   const QString& password,
                   int &http_code,
                   int &err_code,
                   int &network_error);
  static std::vector<CSSEnvironment> get_environments(int &http_code, int& err_code, int &network_error);
  static CSSBalance get_balance(int &http_code, int& err_code, int &network_error);
  static std::vector<CRHInfo> get_ssh_containers(int &http_code, int& err_code, int &network_error);
};

#endif // CRESTWORKER_H
