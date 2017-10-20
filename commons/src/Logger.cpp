#include "Logger.h"
#include "SettingsManager.h"
#include <QTextStream>
#include <QFile>
#include <QTime>
#include <QDirIterator>
#include <QTimer>
#include <QDebug>


/////////////////////////////////////////////////////////////////////////////////////////////////////

void Logger::init (){
  converter[(size_t)QtDebugMsg] = LOG_DEBUG;
  converter[(size_t)QtInfoMsg] = LOG_INFO;
  converter[(size_t)QtWarningMsg] = LOG_WARNING;
  converter[(size_t)QtCriticalMsg] = LOG_CRITICAL;
  converter[(size_t)QtFatalMsg] = LOG_FATAL;

  QTimer *timer = new QTimer(this);
  connect(timer , SIGNAL(timeout()), this , SLOT(deleteOldFiles()));
  timer->start(60 * 60 * 1000);
  deleteOldFiles();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

Logger::LOG_LEVEL Logger::typeToLevel(QtMsgType type){
  return converter[(size_t)type];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

Logger* Logger::Instance() {
  static Logger m_instance;
  return &m_instance;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

const QString& Logger::LogLevelToStr(LOG_LEVEL lt) {
  static QString ll_str[] = {"Debug", "Info", "Warning", "Critical", "Fatal", "Disabled"};
  return ll_str[lt];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void Logger::LoggerMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  if (Logger::Instance()->typeToLevel(type) < (Logger::LOG_LEVEL)CSettingsManager::Instance().logs_level()) // comparing level of msg with currentLogLevel
    return;

  static QString embedToMessage[] = {"Debug", "Info", "Warning", "Critical", "Fatal", "Disabled"};
  QString output_message = QString("[%1] [%2:%3] %4: %5 (in function %6)\n")
                              .arg(QTime::currentTime().toString("HH:mm:ss"))
                              .arg(context.file)
                              .arg(context.line)
                              .arg(embedToMessage[Logger::Instance()->typeToLevel(type)])
                              .arg(msg)
                              .arg(context.function);

  // log output to stdout
  QTextStream stdstream(stdout);
  stdstream << output_message;

  // log output to file
  QFile file(QString("%1/logs_%2.txt").arg(CSettingsManager::Instance().logs_storage()).arg(QDate::currentDate().toString("yyyy.MM.dd")));
  if (!file.open(QIODevice::Append))
    return;
  QTextStream filestream(&file);
  filestream << output_message;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void Logger::deleteOldFiles () {
  try {
    static QDate currentDate = QDate::currentDate();
    static QDirIterator it(CSettingsManager::Instance().logs_storage(), QStringList() << "logs_*.*.*.txt", QDir::Files);
    static QFile file;

    while (it.hasNext())
    {
      QString curFile = it.next();
      QFileInfo fileInformation(curFile);
      if (fileInformation.created().date().daysTo(currentDate) > 7) {
        file.setFileName(curFile);
        file.remove();
      }
    }
  } catch(...){}
}
