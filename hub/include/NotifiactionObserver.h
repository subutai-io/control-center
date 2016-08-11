#ifndef NOTIFIACTIONOBSERVER_H
#define NOTIFIACTIONOBSERVER_H

#include <QObject>
#include <QString>

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

  void notify_all_int(notification_level_t level,
                      const QString& msg) {
    emit notify(level, msg);
  }


public:
  static CNotificationObserver* Instance() {
    static CNotificationObserver obs;
    return &obs;
  }

  static void NotifyAboutError(const QString& msg) {
    Instance()->notify_all_int(NL_ERROR, msg);
  }

  static void NotifyAboutInfo(const QString& msg) {
    Instance()->notify_all_int(NL_INFO, msg);
  }

};

#endif // NOTIFIACTIONOBSERVER_H

