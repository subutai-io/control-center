#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>

////////////////////////////////////////////////////////////////////////////

class Logger : QObject
{
  Q_OBJECT

public:
  enum LOG_LEVEL {LOG_DEBUG = 0, LOG_INFO, LOG_WARNING, LOG_CRITICAL, LOG_FATAL , LOG_DISABLED};

  LOG_LEVEL converter[LOG_DISABLED];

  void init ();

  LOG_LEVEL typeToLevel(QtMsgType type);

  static Logger* Instance();

  static const QString& LogLevelToStr(LOG_LEVEL lt);

  static void LoggerMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

public slots:
  static void deleteOldFiles ();

};



#endif // LOGGER_H
