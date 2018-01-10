#ifndef RHCONTROLLER_H
#define RHCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <map>

class CRhController : public QObject {
  Q_OBJECT

private:
  CRhController(QObject* parent = nullptr);
  virtual ~CRhController();

  std::map<QString, QString> m_dct_resource_hosts;
  bool m_has_changes;

  QTimer m_refresh_timer;
  QTimer m_delay_timer;
  bool m_refresh_in_progress;

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


  void ssh_to_rh(const QString &peer_fingerprint, void* action);

private slots:
  void found_device_slot(QString uid, QString location);
  void refresh_timer_timeout();
  void delay_timer_timeout();

signals:
  void resource_host_list_updated(bool);
};

#endif // RHCONTROLLER_H
