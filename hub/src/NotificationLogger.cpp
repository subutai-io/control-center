#include <QFile>
#include <QDir>

#include "NotificationLogger.h"
#include "SettingsManager.h"
#include "ApplicationLog.h"

static const QString notifications_file = "notifications.log";
QString CNotification::LEVEL_STR[] = {"info", "warning", "error", "critical"};

CNotificationLogger::CNotificationLogger(QObject *parent) : QObject(parent) {
  connect(CNotificationObserver::Instance(), &CNotificationObserver::notify,
          this, &CNotificationLogger::notification_received);
  init_records(QDateTime::currentDateTime().addDays(-7));
  clear_old_records();
  m_clear_timer.setInterval(60*1000*10); //10min
  m_clear_timer.start();
}

CNotificationLogger::~CNotificationLogger() {

}
////////////////////////////////////////////////////////////////////////////

void
CNotificationLogger::init_records(const QDateTime &from) {
  QFile st(CSettingsManager::Instance().logs_storage() +
           QDir::separator() + notifications_file);
  if (!st.exists()) return ;
  if (!st.open(QFile::ReadOnly)) {
    //CApplicationLog::Instance()->LogError("Couldn't open notifications storage file for read only : %s",
    //                                      st.errorString().toStdString().c_str());
    qFatal("Couldn't open notifications storage file for read only : %s",
           st.errorString().toStdString().c_str());
    return ;
  }

  while (!st.atEnd()) {
    QByteArray line_arr = st.readAll();
    if (line_arr.isEmpty()) break;
    QString line_str(line_arr);
    QStringList lst = line_str.split("\n");
    bool converted;
    for (auto i : lst) {
      CNotification notification = CNotification::fromString(i, converted);
      if (!converted) continue;
      if (notification.date_time() < from) continue;
      add_notification(notification);
    }
    emit notifications_updated();
  }
}
////////////////////////////////////////////////////////////////////////////

void
CNotificationLogger::add_notification(const CNotification &notification) {
  m_clear_mutex.lock();
  m_lst_notifications.push_back(notification);
  auto found_union = std::find_if(m_lst_notification_unions.begin(),
                                  m_lst_notification_unions.end(),
                                  [&notification](const CNotificationUnion& item) {
                                    return item.message() == notification.message();
                                  });
  if (found_union == m_lst_notification_unions.end()) {
    m_lst_notification_unions.push_back(CNotificationUnion(notification, 1));
  } else {
    found_union->increment_count();
  }

  emit notifications_updated();
  m_clear_mutex.unlock();
}
////////////////////////////////////////////////////////////////////////////

void
CNotificationLogger::clear_old_records() {
  QDateTime old = QDateTime::currentDateTime().addDays(-7);
  m_clear_mutex.lock();
  auto first_to_remove = std::remove_if(m_lst_notifications.begin(),
                                        m_lst_notifications.end(),
                                        DatePredicate(old));

  for (auto i = first_to_remove; i != m_lst_notifications.end(); ++i) {
    auto found_union = std::find_if(m_lst_notification_unions.begin(),
                                    m_lst_notification_unions.end(),
                                    [i](const CNotificationUnion& item) {
                                      return item.message() == i->message();
                                    });
    if (found_union != m_lst_notification_unions.end()) {
      found_union->decrement_count();
    }
  }

  m_lst_notifications.erase(first_to_remove, m_lst_notifications.end());
  emit notifications_updated();
  m_clear_mutex.unlock();
  QFile st(CSettingsManager::Instance().logs_storage() +
           QDir::separator() + notifications_file);
  if (!st.exists()) return ;
  if (!st.open(QFile::Truncate | QFile::ReadWrite)) {
    //CApplicationLog::Instance()->LogError("Couldn't open notifications storage file with truncate : %s",
    //                                      st.errorString().toStdString().c_str());
    qFatal("Couldn't open notifications storage file with truncate : %s",
           st.errorString().toStdString().c_str());
    return;
  }

  for (auto i : m_lst_notifications) {
    st.write(i.toString().toUtf8());
  }
  st.close();
}
////////////////////////////////////////////////////////////////////////////

void
CNotificationLogger::notification_received(CNotificationObserver::notification_level_t level,
                                           QString str) {
  QFile file(CSettingsManager::Instance().logs_storage() +
             QDir::separator() + notifications_file);
  if (!file.open(QFile::ReadWrite)) {
    //CApplicationLog::Instance()->LogError("Couldn't open notifications storage file : %s",
    //                                      file.errorString().toStdString().c_str());
    qFatal("Couldn't open notifications storage file : %s",
           file.errorString().toStdString().c_str());
    return;
  }
  file.seek(file.size());
  CNotification notification(QDateTime::currentDateTime(), level, str);
  add_notification(notification);
  QString not_str = notification.toString();
  file.write(not_str.toUtf8());
  file.close();
}
////////////////////////////////////////////////////////////////////////////

void
CNotificationLogger::clear_timer_timeout() {
  clear_old_records();
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

QString
CNotification::toString() const {
  QString date_str = m_date_time.toString(Qt::ISODate);
  QString res = QString("%1###%2###%3###\n").arg(date_str).arg(m_level_str).arg(m_msg);
  return res;
}
////////////////////////////////////////////////////////////////////////////

CNotification
CNotification::fromString(const QString &str, bool& converted) {
  CNotification res;
  converted = false;
  QStringList parts = str.split("###", QString::SkipEmptyParts);
  if (parts.count() != 3) return res;
  QDateTime date = QDateTime::fromString(parts[0], Qt::ISODate);
  if (!date.isValid() || date.isNull()) return res;
  res.m_date_time = date;
  res.m_level_str = parts[1];
  for (int i = 0; i < 4; ++i) {
    if (LEVEL_STR[i] != res.m_level_str) continue;
    res.m_level = (CNotificationObserver::notification_level_t)i;
  }
  res.m_msg = parts[2];
  converted = true;
  return res;
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

CNotificationUnion::CNotificationUnion(const CNotification &notification,
                                       uint32_t count) :
  m_level(notification.level()), m_level_str(notification.level_str()),
  m_msg(notification.message()), m_count(count), m_is_ignored(false) {
  m_is_ignored = CSettingsManager::Instance().is_notification_ignored(m_msg);
}

CNotificationUnion::~CNotificationUnion() {
}
////////////////////////////////////////////////////////////////////////////
