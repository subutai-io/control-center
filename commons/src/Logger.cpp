#include "Logger.h"
#include "SettingsManager.h"
#include <QTextStream>
#include <QFile>
#include <QTime>
#include <QDirIterator>
#include <QTimer>
#include <QDebug>

/////////////////////////////////////////////////////////////////////////////////////////////////////

void Logger::Init() {
  QTimer *timer = new QTimer(this);
  connect(timer , &QTimer::timeout, this, &Logger::deleteOldFiles);
  timer->start(60 * 60 * 1000);
  deleteOldFiles();
}
////////////////////////////////////////////////////////////////////////////

Logger::~Logger() {
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

Logger::LOG_LEVEL Logger::typeToLevel(QtMsgType type) {
  static LOG_LEVEL converter[] = {LOG_DEBUG, LOG_WARNING,
                                 LOG_CRITICAL, LOG_FATAL, LOG_INFO};
  return converter[type];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

Logger* Logger::Instance() {
  static Logger instance;
  return &instance;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

const QString& Logger::LogLevelToStr(LOG_LEVEL lt) {
  static QString ll_str[] = {tr("Debug"), tr("Info"), tr("Warning"), tr("Critical"), tr("Fatal"), tr("Disabled")};
  return ll_str[lt];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void Logger::LoggerMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
  if (typeToLevel(type) < (Logger::LOG_LEVEL)CSettingsManager::Instance().logs_level()) // comparing level of msg with currentLogLevel
    return;

  QString output_message = QString("[%1] [%2:%3] %4: %5 (in function %6)\n")
                              .arg(QTime::currentTime().toString("HH:mm:ss"))
                              .arg(context.file)
                              .arg(context.line)
                              .arg(LogLevelToStr(typeToLevel(type)))
                              .arg(msg)
                              .arg(context.function);

  // log output to stdout
  QTextStream stdstream(stdout);
  stdstream << output_message;

  // log output to file
  QFile file(QString("%1/logs_%2.txt").arg(CSettingsManager::Instance().logs_storage()).
             arg(QDate::currentDate().toString("yyyy.MM.dd")));

  if (!file.open(QIODevice::Append))
    return;
  QTextStream filestream(&file);
  filestream << output_message;
  file.close(); //
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

void Logger::deleteOldFiles() {
  try {
    QDate currentDate = QDate::currentDate();
    QDirIterator it(CSettingsManager::Instance().logs_storage(),
                           QStringList() << "logs_*.*.*.txt", QDir::Files);
    QFile file;

    while (it.hasNext()) {
      QString curFile = it.next();
      QFileInfo fileInformation(curFile);
      if (fileInformation.created().date().daysTo(currentDate) <= 7)
        continue;

      file.setFileName(curFile);
      file.remove();
    }
  } catch(...){
    /*do nothing here*/
  }
}
