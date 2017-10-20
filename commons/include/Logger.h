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

  void Init() {;} //need for call constructor
  static Logger* Instance();
  static const QString& LogLevelToStr(LOG_LEVEL lt);
  static void LoggerMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private:
  Logger();
  virtual ~Logger();
  static LOG_LEVEL typeToLevel(QtMsgType type);

private slots:
  static void deleteOldFiles();

};



#endif // LOGGER_H
