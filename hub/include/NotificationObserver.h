#ifndef NOTIFIACTIONOBSERVER_H
#define NOTIFIACTIONOBSERVER_H

#include <QObject>
#include <QString>
#include <QSystemTrayIcon>

typedef enum notification_level {
  NL_INFO = 0,
  NL_WARNING,
  NL_ERROR,
  NL_CRITICAL
} notification_level_t;

class CNotificationObserver : public QObject {
  Q_OBJECT

signals:
  void notify(notification_level_t, const QString&);

private:
  CNotificationObserver(){}
  virtual ~CNotificationObserver(){}
  CNotificationObserver(const CNotificationObserver&);
  CNotificationObserver& operator=(const CNotificationObserver&);

  void notify_all_internal(notification_level_t level,
                      const QString& msg);

  QSystemTrayIcon* m_sys_tray_icon;

public:
  static CNotificationObserver* Instance() {
    static CNotificationObserver obs;
    return &obs;
  }

  void
  set_sys_tray_icon(QSystemTrayIcon* sti) {m_sys_tray_icon = sti;}

  static void Error(const QString& msg) {
    Instance()->notify_all_internal(NL_ERROR, msg);
  }

  static void Info(const QString& msg) {
    Instance()->notify_all_internal(NL_INFO, msg);
  }

  static void Warning(const QString& msg) {
    Instance()->notify_all_internal(NL_WARNING, msg);
  }

  static void Critical(const QString& msg) {
    Instance()->notify_all_internal(NL_CRITICAL, msg);
  }
};

#endif // NOTIFIACTIONOBSERVER_H
