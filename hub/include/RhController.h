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
  static CRhController* Instance()  {
    static CRhController inst;
    return &inst;
  }
  void Init() const{;}

  const std::map<QString, QString>& dctresource_hosts() const {
    return m_dct_resource_hosts;
  }

private slots:
  void found_device_slot(QString uid, QString location);
  void refresh_timer_timeout();
  void delay_timer_timeout();

signals:
  void resource_host_list_updated();
};

#endif // RHCONTROLLER_H
