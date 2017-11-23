#ifndef NOTIFICATIONLOGGER_H
#define NOTIFICATIONLOGGER_H

#include <QObject>
#include <QDateTime>
#include <QTimer>
#include <QMutex>
#include <stdint.h>
#include "NotificationObserver.h"
#include "SettingsManager.h"

/**
 * @brief The CNotification class is Notification abstraction.
 */
class CNotification {
private:
  static QString LEVEL_STR[];

  QDateTime m_date_time;
  CNotificationObserver::notification_level_t m_level;
  QString m_level_str;
  QString m_msg;

public:
  CNotification(){}
  CNotification(const CNotification& copy) : m_date_time(copy.m_date_time),
                                             m_level(copy.m_level),
                                             m_level_str(copy.m_level_str),
                                             m_msg(copy.m_msg){}
  CNotification(const QDateTime& datetime,
                 CNotificationObserver::notification_level_t level,
                 const QString& msg) : m_date_time(datetime), m_level(level), m_msg(msg) {
    m_level_str = LEVEL_STR[m_level];
  }
  ~CNotification(){}

  QString toString() const;

  const QDateTime& date_time() const {return m_date_time;}
  CNotificationObserver::notification_level_t level() const {return m_level;}
  const QString& level_str() const {return m_level_str;}
  const QString& message() const {return m_msg;}

  static CNotification fromString(const QString& str, bool &converted);
};
////////////////////////////////////////////////////////////////////////////

/**
 * @brief The CNotificationUnion is union of several notifications with same message
 */
class CNotificationUnion {
private:
  CNotificationObserver::notification_level_t m_level;
  QString m_level_str;
  QString m_msg;
  uint32_t m_count;
  bool m_is_ignored;
public:
  explicit CNotificationUnion(const CNotification& notification, uint32_t count);
  ~CNotificationUnion();

  CNotificationObserver::notification_level_t level() const {return m_level;}
  const QString& level_str() const {return m_level_str;}
  const QString& message() const {return m_msg;}
  uint32_t count() const {return m_count;}
  bool is_ignored() const {return m_is_ignored;}
  void increment_count() {++m_count;}
  void decrement_count() {--m_count;}
  void set_ignored(bool val) {m_is_ignored = val;
                             if (m_is_ignored) CSettingsManager::Instance().ignore_notification(m_msg);
                             else CSettingsManager::Instance().not_ignore_notification(m_msg);}
};
////////////////////////////////////////////////////////////////////////////

/**
 * @brief The DatePredicate class should be replaced with lambda. It's 2k17!!!
 */
class DatePredicate {
private:
  QDateTime dt;
public:
  explicit DatePredicate(const QDateTime& _dt) : dt(_dt){}
  bool operator()(const CNotification& item) {return item.date_time() < dt;}
};
////////////////////////////////////////////////////////////////////////////

/**
 * @brief The CNotificationLogger class serializes notifications to file and
 * deserializes from file.
 */
class CNotificationLogger : public QObject {
  Q_OBJECT

private:
  CNotificationLogger(QObject* parent = nullptr);
  virtual ~CNotificationLogger();

  std::vector<CNotification> m_lst_notifications;
  QMutex m_clear_mutex;
  QTimer m_clear_timer;
  std::vector<CNotificationUnion> m_lst_notification_unions;

  void init_records(const QDateTime& from);
  void add_notification(const CNotification& notification);

public:
  static CNotificationLogger* Instance() {
    static CNotificationLogger inst;
    return &inst;
  }

  void init() const{;}
  void clear_old_records();

  std::vector<CNotification>& notifications() {
    return m_lst_notifications;
  }

  std::vector<CNotificationUnion>& notification_unions() {
    return m_lst_notification_unions;
  }

private slots:
  void notification_received(CNotificationObserver::notification_level_t level,
                             QString str);
  void clear_timer_timeout();

signals:
  void notifications_updated();
};

#endif // NOTIFICATIONLOGGER_H
