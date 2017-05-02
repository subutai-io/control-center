#ifndef NOTIFICATIONLOGGER_H
#define NOTIFICATIONLOGGER_H

#include <QObject>
#include <QDateTime>
#include <QTimer>
#include <QMutex>
#include "NotificationObserver.h"

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

class CNotificationUnion {
private:
  CNotificationObserver::notification_level_t m_level;
  QString m_level_str;
  QString m_msg;
  uint32_t m_count;
public:
  explicit CNotificationUnion(const CNotification& notification, uint32_t count) :
    m_level(notification.level()), m_level_str(notification.level_str()),
    m_msg(notification.message()), m_count(count){}
  ~CNotificationUnion(){}

  CNotificationObserver::notification_level_t level() const {return m_level;}
  const QString& level_str() const {return m_level_str;}
  const QString& message() const {return m_msg;}
  uint32_t count() const {return m_count;}
  void increment_count() {++m_count;}
  void decrement_count() {--m_count;}
};
////////////////////////////////////////////////////////////////////////////

class DatePredicate {
private:
  QDateTime dt;
public:
  explicit DatePredicate(const QDateTime& _dt) : dt(_dt){}
  bool operator()(const CNotification& item) {return item.date_time() < dt;}
};
////////////////////////////////////////////////////////////////////////////

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
