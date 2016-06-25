#ifndef RESTCONTAINERS_H
#define RESTCONTAINERS_H

#include <QString>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

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
    return m_id == arg.m_id && m_ip == arg.m_ip && m_port == arg.m_port;
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
    for (auto i = arr.begin(); i != arr.end(); ++i) {
      if (i->isNull() || !i->isObject()) continue;
      m_lst_containers.push_back(CHubContainer(i->toObject()));
    }
  }

  ~CSSEnvironment(){}

  //todo use id
  bool operator==(const CSSEnvironment& arg) const {
    return m_id == m_id &&
        m_name == arg.m_name &&
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
    m_id = obj["id"].toString();
    m_ch_ip = obj["ip"].toString();
  }

  const QString& id() const {return m_id;}
  const QString& ch_ip() const {return m_ch_ip;}

  bool operator==(const CCHInfo& arg) const {
    return m_id==arg.m_id && m_ch_ip == arg.m_ch_ip;
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
    m_rh_ip = obj["rh_ip"].toString();

    QJsonArray arr= obj["containers"].toArray();
    for (auto i = arr.begin(); i != arr.end(); ++i) {
      if (i->isNull() || !i->isObject()) continue;
      m_lst_ch.push_back(CCHInfo(i->toObject()));
    }
  }

  const QString& id() const {return m_id;}
  const QString& rh_ip() const {return m_rh_ip;}
  const std::vector<CCHInfo>& lst_containers() const {return m_lst_ch;}

  bool operator==(const CRHInfo& arg) const {
    return m_id==arg.m_id && m_rh_ip == arg.m_rh_ip && m_lst_ch == arg.m_lst_ch;
  }

  bool operator!=(const CRHInfo& arg) const {
    return !(this->operator ==(arg));
  }
};
////////////////////////////////////////////////////////////////////////////

class CGorjunFileInfo {
private:
  QString m_md5_sum;
  QString m_name;
  int m_size;
  QString m_id;

public:
  explicit CGorjunFileInfo(const QJsonObject& obj) {
    m_md5_sum = obj["md5Sum"].toString();
    m_name = obj["name"].toString();
    m_size = obj["size"].toInt();
    m_id = obj["id"].toString();
  }
  ~CGorjunFileInfo() {}

  const QString& md5_sum() const {return m_md5_sum;}
  const QString& name() const {return m_name;}
  int size() const {return m_size;}
  const QString& id() const {return m_id;}

  bool operator==(const CGorjunFileInfo& gfi) const {
    return m_id == gfi.m_id;
  }

  bool operator!=(const CGorjunFileInfo& gfi) const {
    return !(this->operator ==(gfi));
  }
};

#endif // RESTCONTAINERS_H
