#include <QFile>
#include <QDir>

#include "NotificationLogger.h"
#include "SettingsManager.h"
#include "ApplicationLog.h"

static const QString notifications_file = "notifications.log";
static QString level_str[] = {"info", "warning",
                              "error", "critical"};

CNotificationLogger::CNotificationLogger(QObject *parent) : QObject(parent) {
  connect(CNotificationObserver::Instance(), SIGNAL(notify(notification_level_t,QString)),
          this, SLOT(notification_received(notification_level_t,QString)));
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
    CApplicationLog::Instance()->LogError("Couldn't open notifications storage file for read only : %s",
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
      m_clear_mutex.lock();
      m_lst_notifications.push_back(notification);
      m_clear_mutex.unlock();
    }
    emit notifications_updated();
  }
}
////////////////////////////////////////////////////////////////////////////

void
CNotificationLogger::clear_old_records() {
  QDateTime old = QDateTime::currentDateTime().addDays(-7);
  m_clear_mutex.lock();
  std::remove_if(m_lst_notifications.begin(), m_lst_notifications.end(), DatePredicate(old));
  emit notifications_updated();
  m_clear_mutex.unlock();
  QFile st(CSettingsManager::Instance().logs_storage() +
           QDir::separator() + notifications_file);
  if (!st.exists()) return ;
  if (!st.open(QFile::Truncate | QFile::ReadWrite)) {
    CApplicationLog::Instance()->LogError("Couldn't open notifications storage file with truncate : %s",
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
CNotificationLogger::notification_received(notification_level_t level,
                                           QString str) {
  QFile file(CSettingsManager::Instance().logs_storage() +
             QDir::separator() + notifications_file);
  if (!file.open(QFile::ReadWrite)) {
    CApplicationLog::Instance()->LogError("Couldn't open notifications storage file : %s",
                                          file.errorString().toStdString().c_str());
    return;
  }
  file.seek(file.size());
  CNotification notification(QDateTime::currentDateTime(), level_str[level], str);
  m_clear_mutex.lock();
  m_lst_notifications.push_back(notification);
  emit notifications_updated();
  m_clear_mutex.unlock();
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
  QString res = QString("%1###%2###%3###\n").arg(date_str).arg(m_level).arg(m_msg);
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
  res.m_level = parts[1];
  res.m_msg = parts[2];
  converted = true;
  return res;
}
////////////////////////////////////////////////////////////////////////////
