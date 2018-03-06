#ifndef RHCONTROLLER_H
#define RHCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <map>
#include "SystemCallWrapper.h"
#include <QFileSystemModel>
#include "RestContainers.h"

class CRhController : public QObject {
  Q_OBJECT

private:
  CRhController(QObject* parent = nullptr);
  virtual ~CRhController();

  std::map<QString, QString> m_dct_resource_hosts;
  std::vector<CLocalPeer> m_local_peers;
  bool m_has_changes;

  QTimer m_refresh_timer;
  QTimer m_delay_timer;
  bool m_refresh_in_progress;

  void search_local();
  void get_peer_info(const QFileInfo &fi, QDir dir);
  QString parse_name(const QString &name);

public:

  static const int REFRESH_DELAY_SEC = 8;
  static CRhController* Instance() {
    static CRhController inst;
    return &inst;
  }

  void init();
  void refresh();

  const std::map<QString, QString>& dct_resource_hosts() const {
    return m_dct_resource_hosts;
  }

  const std::vector<CLocalPeer>& local_peers() const {
      return m_local_peers;
  }

  void ssh_to_rh(const QString &peer_fingerprint);

private slots:
  void found_device_slot(QString uid, QString location);
  void refresh_timer_timeout();
  void delay_timer_timeout();

signals:
  void resource_host_list_updated(bool);
  void ssh_to_rh_finished(const QString &peer_fingerprint, system_call_wrapper_error_t res, int exit_code);
};

#endif // RHCONTROLLER_H
