#ifndef RESTCONTAINERS_H
#define RESTCONTAINERS_H

#include <stdint.h>
#include <QString>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "SettingsManager.h"

/*!
 * \brief This class represents information about container
 */
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

  /*!
   * \brief Container's IP
   */
  const QString& ip() const { return m_ip;}

  /*!
   * \brief Container's name
   */
  const QString& name() const { return m_name;}

  /*!
   * \brief Container's ID
   */
  const QString& id() const {return m_id;}

  /*!
   * \brief Container's port. 10000 + last octet of container's IP.
   */
  const QString& port() const {return m_port;}

  bool operator==(const CHubContainer& arg) const {
    return m_id == arg.m_id && m_ip == arg.m_ip && m_port == arg.m_port;
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
      if (arg1[i] == arg2[j]) {
        found = true;
        break;
      }
    }
    if (!found) return false;
  }
  return true;
}

/*!
 * \brief This class represents environment
 */
class CSSEnvironment {
private:
  QString m_name;
  QString m_hash;
  QString m_aes_key;
  QString m_ttl;
  QString m_id;
  QString m_status;
  QString m_status_descr;
  std::vector<CHubContainer> m_lst_containers;
public:
  CSSEnvironment() : m_name(""){}
  explicit CSSEnvironment(const QJsonObject& obj) {
    m_name = obj["environment_name"].toString();
    m_hash = obj["environment_hash"].toString();
    m_aes_key = obj["environment_key"].toString();
    m_ttl = obj["environment_ttl"].toString();
    m_id = obj["environment_id"].toString();
    m_status = obj["environment_status"].toString();
    m_status_descr = obj["environment_status_desc"].toString();

    QJsonArray arr = obj["environment_containers"].toArray();
    for (auto i = arr.begin(); i != arr.end(); ++i) {
      if (i->isNull() || !i->isObject()) continue;
      m_lst_containers.push_back(CHubContainer(i->toObject()));
    }
  }

  ~CSSEnvironment(){}

  //todo use id
  bool operator==(const CSSEnvironment& arg) const {
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

  bool operator!=(const CSSEnvironment& arg) const {
    return !(this->operator ==(arg));
  }

  /*!
   * \brief Environment's name
   */
  const QString& name() const {return m_name;}
  /*!
   * \brief Environment's hash. Used for p2p join
   */
  const QString& hash() const {return m_hash;}
  /*!
   * \brief Environment's aes key. Used by p2p for traffic encryption
   */
  const QString& key() const {return m_aes_key;}
  /*!
   * \brief Environment's id
   */
  const QString& id() const {return m_id;}
  /*!
   * \brief Environment's list of containers.
   */
  const std::vector<CHubContainer>& containers() const {return m_lst_containers;}
  /*!
   * \brief Environment's status (HEALTHY, UNDER_MODIFICATION etc.)
   */
  const QString& status() const {return m_status;}
  /*!
   * \brief If status != HEALTHY we can receive reason in this field represented by string
   */
  const QString& status_description() const {return m_status_descr;}

  /*!
   * \brief Is environment in HEALTHY state.
   */
  bool healthy() const {return m_status == "HEALTHY";}
};
////////////////////////////////////////////////////////////////////////////

/*!
 * \brief This class represents user's balance
 */
class CSSBalance {
private:
  QString m_balance;
public:
  CSSBalance() : m_balance("") {}
  explicit CSSBalance(const QString& balance) : m_balance(balance) {
  }
  ~CSSBalance(){}

  /*!
   * \brief String representation of user's balance
   */
  const QString& value() const {return m_balance;}
};
////////////////////////////////////////////////////////////////////////////

/*!
 * \brief Represents information about control host
 */
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

/*!
 * \brief Represents information about resource host
 */
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
    return m_id==arg.m_id &&
        m_rh_ip == arg.m_rh_ip &&
        VectorEq<CCHInfo>(m_lst_ch, arg.m_lst_ch);
  }

  bool operator!=(const CRHInfo& arg) const {
    return !(this->operator ==(arg));
  }
};
////////////////////////////////////////////////////////////////////////////

/*!
 * \brief Represents gorjun file information like size and md5.
 */
class CGorjunFileInfo {
private:
  QString m_md5_sum;
  QString m_name;
  int m_size;
  QString m_id;

public:
  explicit CGorjunFileInfo(const QJsonObject& obj) {
    m_md5_sum = obj["id"].toString(); //I like hub team
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
////////////////////////////////////////////////////////////////////////////

typedef enum report_type {
  RT_HEALTH = 0, RT_ERROR
} report_type_t;

/*!
 * \brief Report header contains MAGIC number and tray unique identificator. And report type.
 */
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

/*!
 * \brief Represents report information. Contains header and data. Data should be represented as class
 * with to_json_object method and copy constructor
 */
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

/*!
 * \brief This class is used for wrapping health report data.
 */
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

#endif // RESTCONTAINERS_H
