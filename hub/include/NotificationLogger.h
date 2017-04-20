#ifndef NOTIFICATIONLOGGER_H
#define NOTIFICATIONLOGGER_H

#include <QObject>
#include <QDateTime>
#include <QTimer>
#include <QMutex>
#include "NotifiactionObserver.h"

class CNotification {
private:
  QDateTime m_date_time;
  notification_level_t m_level;
  QString m_level_str;
  QString m_msg;

  static QString LEVEL_STR[];
public:
  CNotification(){}
  CNotification(const CNotification& copy) :
    m_date_time(copy.m_date_time), m_level(copy.m_level), m_msg(copy.m_msg){
    m_level_str = LEVEL_STR[m_level];
  }
  CNotification(const QDateTime& datetime,
                 notification_level_t level,
                 const QString& msg) : m_date_time(datetime), m_level(level), m_msg(msg){}
  ~CNotification(){}

  QString toString() const;

  const QDateTime& date_time() const {return m_date_time;}
  notification_level_t level() const {return m_level;}
  const QString& level_str() const {return m_level_str;}
  const QString& message() const {return m_msg;}

  static CNotification fromString(const QString& str, bool &converted);
};

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
  void init_records(const QDateTime& from); //todo move from to settings

  std::vector<CNotification> m_lst_notifications;
  QMutex m_clear_mutex;
  QTimer m_clear_timer;

public:
  static CNotificationLogger* Instance() {
    static CNotificationLogger inst;
    return &inst;
  }

  void init() const{;}
  void clear_old_records();

  const std::vector<CNotification>& notifications() const {
    return m_lst_notifications;
  }

private slots:
  void notification_received(notification_level_t level,
                             QString str);
  void clear_timer_timeout();

signals:
  void notifications_updated();
};

#endif // NOTIFICATIONLOGGER_H
