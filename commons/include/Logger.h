#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>

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

private slots:
  static void deleteOldFiles();



};



#endif // LOGGER_H
