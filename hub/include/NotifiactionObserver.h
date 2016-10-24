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

/*!
 * \brief This class is used for notifications. Many instances could be notified.
 * All subscribers will receive notification message and notification level. Level
 * could be "error" or "info". In our case we use it for pop up notification messages.
 */
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
  /*!
   * \brief Instance of this singleton class
   */
  static CNotificationObserver* Instance() {
    static CNotificationObserver obs;
    return &obs;
  }

  /*!
   * \brief Notify all subscribers about something with "error" level
   */
  static void NotifyAboutError(const QString& msg) {
    Instance()->notify_all_int(NL_ERROR, msg);
  }

  /*!
   * \brief Notify all subscribers about something with "info" level
   */
  static void NotifyAboutInfo(const QString& msg) {
    Instance()->notify_all_int(NL_INFO, msg);
  }

};

#endif // NOTIFIACTIONOBSERVER_H

