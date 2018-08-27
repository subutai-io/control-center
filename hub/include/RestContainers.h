#ifndef RESTCONTAINERS_H
#define RESTCONTAINERS_H

#include <stdint.h>
#include <QString>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "SettingsManager.h"
#include "OsBranchConsts.h"

class CHubContainer {
private:
  QString m_name;
  QString m_ip;
  QString m_id;
  bool m_is_desktop;
  QString m_port;
  QString m_rh_ip;
  QString m_rh_id;
  QString m_peer_id;
  QString m_desk_env;

public:
  explicit CHubContainer(QJsonObject obj) :
    m_name(obj["container_name"].toString()),
    m_ip(obj["container_ip"].toString()),
    m_id(obj["container_id"].toString()),
    m_is_desktop(str_to_bool(obj["container_is_desktop"].toString())),
    m_rh_ip(obj["rh_ip"].toString()),
    m_rh_id(obj["rh_id"].toString()),
    m_peer_id(obj["peer_id"].toString()),
    m_desk_env(obj["container_desk_env"].isObject() ? obj["container_desk_env"].toString() : "MATE")
  {
    QHostAddress cont_ip_addr(m_ip.split("/")[0]);
    if (!cont_ip_addr.isNull()) {
      quint32 ip = cont_ip_addr.toIPv4Address();
      quint8 port = ip & 0x000000ff; //last octet
      m_port = QString("%1").arg(10000+port);
    }
  }

  ~CHubContainer(){}

  const QString& ip() const { return m_ip;}

  const QString& name() const { return m_name;}

  const QString& id() const {return m_id;}

  const QString& port() const {return m_port;}

  const QString& rh_ip() const {return m_rh_ip;}

  const bool& is_desktop() const {return m_is_desktop;}

  const QString& desk_env() const {return m_desk_env;}

  const QString& peer_id() const {return m_peer_id;}

  bool str_to_bool(QString bol) {
    return bol == QString("true") ? true:false;
  }

  bool operator==(const CHubContainer& arg) const {
    bool res =
        m_name == arg.m_name &&
        m_ip == arg.m_ip &&
        m_id == arg.m_id &&
        m_port == arg.m_port &&
        m_rh_ip == arg.m_rh_ip &&
        m_is_desktop == arg.m_is_desktop;
    return res;
  }
  bool operator!=(const CHubContainer& arg) const {
    return !(this->operator ==(arg));
  }
};
////////////////////////////////////////////////////////////////////////////

template<class T>
bool VectorEq(const std::vector<T>& arg1,
              const std::vector<T>& arg2) {
  if (arg1.size() != arg2.size())
    return false;

  for (size_t i = 0; i < arg1.size(); ++i) {
    bool found = false;
    for (size_t j = 0; j < arg2.size(); ++j) {
      if (arg1[i] != arg2[j]) continue;
      found = true;
      break;
    }

    if (!found)
      return false;
  }
  return true;
}

class CEnvironment {
private:
  QString m_name;
  QString m_hash;
  QString m_aes_key;
  QString m_ttl;
  QString m_id;
  int m_hub_id;
  QString m_status;
  QString m_status_descr;
  int m_base_interface_id;
  std::vector<CHubContainer> m_lst_containers;
public:
  CEnvironment() : m_name(""){}
  explicit CEnvironment(const QJsonObject& obj) {
    m_name = obj["environment_name"].toString();
    m_hash = obj["environment_hash"].toString();
    m_aes_key = obj["environment_key"].toString();
    m_ttl = obj["environment_ttl"].toString();
    m_id = obj["environment_id"].toString();
    m_hub_id = obj["environment_hub_id"].toInt();
    m_status = obj["environment_status"].toString();
    m_status_descr = obj["environment_status_desc"].toString();
    m_base_interface_id = -1;
    QJsonArray arr = obj["environment_containers"].toArray();
    for (auto i = arr.begin(); i != arr.end(); ++i) {
      if (i->isNull() || !i->isObject()) continue;
      CHubContainer hc = CHubContainer(i->toObject());
      if (hc.name().isEmpty() || hc.name() == "") continue;
      m_lst_containers.push_back(hc);
    }
  }

  ~CEnvironment(){}

  bool operator==(const CEnvironment& arg) const {
    bool res =
        m_id == arg.m_id &&
        m_name == arg.m_name &&
        m_hash == arg.m_hash &&
        m_aes_key == arg.m_aes_key &&
        m_ttl == arg.m_ttl &&
        m_status == arg.m_status &&
        m_status_descr == arg.m_status_descr &&
        VectorEq<CHubContainer>(m_lst_containers, arg.m_lst_containers);
    return res;
  }

  bool operator!=(const CEnvironment& arg) const {
    return !(this->operator ==(arg));
  }

  const QString& name() const {return m_name;}
  const QString& hash() const {return m_hash;}
  const QString& key() const {return m_aes_key;}
  const QString& id() const {return m_id;}
  const int& hub_id() const {return m_hub_id;}
  const QString& ttl() const {return m_ttl;}
  const std::vector<CHubContainer>& containers() const {return m_lst_containers;}
  const QString& status() const {return m_status;}
  const QString& status_description() const {return m_status_descr;}
  const int& base_interface_id() const {return m_base_interface_id;}
  bool healthy() const {return m_status == QString("HEALTHY");}
  bool under_modification() const {return m_status == QString("UNDER_MODIFICATION");}

  void set_base_interface_id(int base_interface_id) {
    m_base_interface_id = base_interface_id;
  }

  void set_status(const QString& val){m_status = val;}
};
////////////////////////////////////////////////////////////////////////////

class CHubBalance {
private:
  QString m_balance;
public:
  CHubBalance() : m_balance("") {}
  explicit CHubBalance(const QString& balance) : m_balance(balance) {
  }
  ~CHubBalance(){}
  const QString& value() const {return m_balance;}
};
////////////////////////////////////////////////////////////////////////////

class CGorjunFileInfo {
private:
  QString m_md5_sum;
  QString m_name;
  int m_size;
  QString m_id;
  QString m_version;

public:
  explicit CGorjunFileInfo(const QJsonObject& obj) {
    m_md5_sum = obj["md5"].toString();
    m_name = obj["filename"].toString();
    m_size = obj["size"].toInt();
    m_id = obj["id"].toString();
    m_version = obj["version"].toString();
  }
  ~CGorjunFileInfo() {}

  const QString& md5_sum() const {return m_md5_sum;}
  const QString& name() const {return m_name;}
  int size() const {return m_size;}
  const QString& id() const {return m_id;}
  const QString& version() const {return m_version;}

  bool operator==(const CGorjunFileInfo& gfi) const {
    return m_id == gfi.m_id;
  }

  bool operator!=(const CGorjunFileInfo& gfi) const {
    return !(this->operator ==(gfi));
  }
};
////////////////////////////////////////////////////////////////////////////

typedef enum report_type {
  RT_HEALTH = 0, RT_ERROR
} report_type_t;

class CReportHeader {
private:
  static const int32_t MAGIC = 0x0caa1020;
  int32_t m_magic;
  QString m_tray_guid;
  report_type_t m_type;

public:
  CReportHeader(report_type_t type) :
    m_magic(MAGIC),
    m_tray_guid(CSettingsManager::Instance().tray_guid()),
    m_type(type) {}

  ~CReportHeader() {}

  QJsonObject to_json_object() const {
    QJsonObject obj;
    obj["magic"] = m_magic;
    obj["tray_guid"] = m_tray_guid;
    obj["type"] = (int32_t)m_type;
    return obj;
  }
};
////////////////////////////////////////////////////////////////////////////

template<class DT> class CTrayReport {
private:
  CReportHeader m_hdr;
  DT m_data;
public:
  CTrayReport(const DT& data) :
    m_hdr(DT::TYPE),
    m_data(data) { }
  ~CTrayReport() {}

  QJsonObject to_json_object() const {
    QJsonObject obj;
    obj["hdr"] = m_hdr.to_json_object();
    obj["data"] = m_data.to_json_object();
    return obj;
  }
};
////////////////////////////////////////////////////////////////////////////

class CHealthReportData {
private:
  QString m_p2p_status;
  QString m_p2p_version;
  QString m_os_info;

public:
  static const report_type_t TYPE = RT_HEALTH;
  CHealthReportData(const QString& p2p_status,
                const QString& p2p_version,
                const QString& os_info) :
    m_p2p_status(p2p_status),
    m_p2p_version(p2p_version),
    m_os_info(os_info) {}

  ~CHealthReportData(){}

  QJsonObject to_json_object() const {
    QJsonObject obj;
    obj["p2p_status"] = m_p2p_status;
    obj["p2p_version"] = m_p2p_version;
    obj["os_info"] = m_os_info;
    return obj;
  }
};
////////////////////////////////////////////////////////////////////////////

class CMyPeerInfo {
public:
  struct env_info
  {
    QString mypeerid;
    QString ownerName, envName, status;
    int ownerId, envId;

    env_info(const QJsonObject& obj, QString& peer_id) {
      ownerName = obj["envOwnerName"].toString();
      ownerId = obj["envOwnerId"].toInt();
      envName = obj["envName"].toString();
      envId = obj["envId"].toInt();
      status = obj["envStatus"].toString();
      mypeerid = peer_id;
    }
  };
  CMyPeerInfo(){}

private:
  QString m_country;
  QString m_countryCode;
  QString m_fingerprint;
  QString m_id;
  QString m_ip_address;
  QString m_isp;
  QString m_name;
  int m_rh_count;
  QString m_scope;
  QString m_status;
  bool m_updated;
  std::vector <env_info> m_lst_environments;

public:

  explicit CMyPeerInfo(const QJsonObject& obj) {
    m_country = obj["country"].toString();
    m_countryCode = obj["countryCode"].toString();
    m_fingerprint = obj["fingerprint"].toString();
    m_id = obj["id"].toString();
    m_ip_address = obj["ipAddress"].toString();
    m_isp = obj["isp"].toString();
    m_name = obj["name"].toString();
    m_rh_count = obj["rhCount"].toInt();
    m_scope = obj["scope"].toString();
    m_status = obj["status"].toString();

    QJsonArray arr = obj["environments"].toArray();
    for (auto i = arr.begin(); i != arr.end(); ++i) {
      if (i->isNull() || !i->isObject()) continue;
      env_info ei(i->toObject(), m_id);
      m_lst_environments.push_back(ei);
    }
  }
  ~CMyPeerInfo(){}

  std::vector<env_info> &peer_environments()  { return m_lst_environments; }
  const QString &country() const { return m_country; }
  const QString &countryCode() const { return m_countryCode; }
  const QString &fingerprint() const { return m_fingerprint; }
  const QString &id() const { return m_id; }
  const QString &ip_address() const { return m_ip_address; }
  const QString &isp() const { return m_isp; }
  const QString &name() const { return m_name; }
  int rh_count() const { return m_rh_count; }
  const QString &scope() const { return m_scope; }
  const QString &status() const { return m_status; }
  const bool &updated() const { return m_updated; }
  void set_updated(bool val){ m_updated = val; }
};

class CP2Ppeer
{
private:
  QString m_id;
  QString m_ip;
  QString m_state;
  QString m_last_error;
public:

  explicit CP2Ppeer(const QJsonObject& obj) {
    m_id = obj["id"].toString();
    m_ip = obj["ip"].toString();
    m_state = obj["state"].toString();
    m_last_error = obj["lastError"].toString();
  }
  const QString &id() const { return m_id; }
  const QString &ip() const { return m_ip; }
  const QString &state() const { return m_state; }
  const QString &last_error() const { return m_last_error; }
};

class CP2PInstance {

private:
  QString m_id;
  QString m_ip;
  std::vector <CP2Ppeer> m_peers;
public:
  explicit CP2PInstance(const QJsonObject& obj) {
    m_id = obj["id"].toString();
    m_ip = obj["ip"].toString();

    QJsonArray arr = obj["peers"].toArray();
    for (auto i = arr.begin(); i != arr.end(); ++i) {
      if (i->isNull() || !i->isObject()) continue;
      CP2Ppeer p2p_peer(i->toObject());
      m_peers.push_back(p2p_peer);
    }
  }

  const QString &id() const { return m_id; }
  const QString &ip() const { return m_ip; }
  const std::vector<CP2Ppeer>& peers() const {return m_peers;}

};

#endif // RESTCONTAINERS_H
