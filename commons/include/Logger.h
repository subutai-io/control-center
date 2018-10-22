#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QTime>
#include <QMutex>
#include <QMutexLocker>
#include <QString>
#include "SettingsManager.h"
#include "OsBranchConsts.h"

////////////////////////////////////////////////////////////////////////////

/**
 * @brief The Logger class wrapps qDebug() , qWarning() etc. functions and write it to log files.
 */
class Logger : QObject
{
  Q_OBJECT

public:
  enum LOG_LEVEL {LOG_DEBUG = 0, LOG_INFO, LOG_WARNING, LOG_CRITICAL, LOG_FATAL , LOG_DISABLED};
  void Init();
  static Logger* Instance();
  static const QString& LogLevelToStr(LOG_LEVEL lt);
  static const QString& LogStorage();
  QFile* file();

  /**
   * @brief Write message to file.
   * @param type - qDebug(), qWarning() etc.
   * @param context - file, line, date etc.
   * @param msg - message text
   */
  static void LoggerMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private:
  static LOG_LEVEL typeToLevel(QtMsgType type);
  virtual ~Logger();
  QFile *m_file;

private slots:
  static void deleteOldFiles();
};

#endif // LOGGER_H
