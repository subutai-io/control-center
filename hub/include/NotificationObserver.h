#ifndef NOTIFIACTIONOBSERVER_H
#define NOTIFIACTIONOBSERVER_H

#include <QObject>

class QWidget;

class CNotificationObserver : public QObject {
  Q_OBJECT

public:
  enum notification_level_t {
    NL_INFO = 0,
    NL_WARNING,
    NL_ERROR,
    NL_CRITICAL
  };

  enum notification_preffered_place_t {
    NPP_RIGHT_UP = 0,
    NPP_RIGHT_DOWN = 1,
    NPP_LEFT_DOWN = 2,
    NPP_LEFT_UP = 3,
    NPP_LAST
  };

  static CNotificationObserver* Instance() {
    static CNotificationObserver obs;
    return &obs;
  }

  static const QString& notification_level_to_str(notification_level_t nt);
  static const QString& notifications_preffered_place_to_str(notification_preffered_place_t pl);
  static void Error(const QString& msg);
  static void Info(const QString& msg);
  static void Warning(const QString& msg);
  static void Critical(const QString& msg);

private:
  CNotificationObserver(){}
  virtual ~CNotificationObserver(){}
  CNotificationObserver(const CNotificationObserver&);
  CNotificationObserver& operator=(const CNotificationObserver&);

  void notify_all_internal(notification_level_t level,
                      const QString& msg);
signals:
  void notify(CNotificationObserver::notification_level_t, const QString&);
};

#endif // NOTIFIACTIONOBSERVER_H
