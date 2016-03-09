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

class CNotifiactionObserver : public QObject {
  Q_OBJECT

signals:
  void notify(notification_level_t, const QString&);

private:
  CNotifiactionObserver(){}
  virtual ~CNotifiactionObserver(){}
  CNotifiactionObserver(const CNotifiactionObserver&);
  CNotifiactionObserver& operator=(const CNotifiactionObserver&);

  void notify_all_int(notification_level_t level,
                      const QString& msg) {
    emit notify(level, msg);
  }


public:
  static CNotifiactionObserver* Instance() {
    static CNotifiactionObserver obs;
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

