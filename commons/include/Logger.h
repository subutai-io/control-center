#ifndef LOGGER_H
#define LOGGER_H

#include <QTextStream>
#include <QFile>
#include <QTime>
#include "SettingsManager.h"

#include <QDebug>


class Logger
{
public:
  QtMsgType currentLogLevel;
  QFile file;

  static Logger* Instance() {
      static Logger m_instance;
      return &m_instance;
  }

  int logLevel(){
    return (int)currentLogLevel;
  }

  void setLogLevel(QtMsgType lt) {
      currentLogLevel = lt;
  }

  static const QString& LogLevelToStr(QtMsgType lt) {
    static QString ll_str[] = {"Debug", "Warning", "Critical", "Fatal" , "Info" , "Disabled"};
    return ll_str[lt];
  }

  static void LoggerMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
  {
    if ((int)type < Logger::Instance()->logLevel()) // comparing level of msg with currentLogLevel
      return;

    // log output to stdout
    QTextStream stdstream(stdout);
    QString timestamp = (QTime::currentTime().toString("HH:mm:ss"));

    switch (type) {
      case QtDebugMsg: // 0 level
        stdstream << QString("[%1] [%2:%3] Debug: %4 (in function %5)\n").arg(timestamp).arg(context.file).arg(context.line).arg(msg).arg(context.function);
        break;
      case QtWarningMsg: // 1 level
        stdstream << QString("[%1] [%2:%3] Info: %4 (in function %5)\n").arg(timestamp).arg(context.file).arg(context.line).arg(msg).arg(context.function);
        break;
      case QtCriticalMsg: // 2 level
        stdstream << QString("[%1] [%2:%3] Info: %4 (in function %5)\n").arg(timestamp).arg(context.file).arg(context.line).arg(msg).arg(context.function);
        break;
      case QtFatalMsg: // 3 level
        stdstream << QString("[%1] [%2:%3] Info: %4 (in function %5)\n").arg(timestamp).arg(context.file).arg(context.line).arg(msg).arg(context.function);
        abort();
      case QtInfoMsg: // 4 level
        stdstream << QString("[%1] [%2:%3] Info: %4 (in function %5)\n").arg(timestamp).arg(context.file).arg(context.line).arg(msg).arg(context.function);
        break;
    }

    // log output to file
    Logger::Instance()->file.setFileName(QString("%1\\logs_%2.txt").arg(CSettingsManager::Instance().logs_storage()).arg(QDate::currentDate().toString("yyyy.MM.dd")));
    if (!Logger::Instance()->file.open(QIODevice::Append)) // if file doesn't exist
      return;
    QTextStream filestream(&Logger::Instance()->file);

    switch (type) {
      case QtDebugMsg: // 0 level
      filestream << QString("[%1] [%2:%3] Debug: %4 (in function %5)\n").arg(timestamp).arg(context.file).arg(context.line).arg(msg).arg(context.function);
        break;
      case QtWarningMsg: // 1 level
        filestream << QString("[%1] [%2:%3] Info: %4 (in function %5)\n").arg(timestamp).arg(context.file).arg(context.line).arg(msg).arg(context.function);
        break;
      case QtCriticalMsg: // 2 level
        filestream << QString("[%1] [%2:%3] Info: %4 (in function %5)\n").arg(timestamp).arg(context.file).arg(context.line).arg(msg).arg(context.function);
        break;
      case QtFatalMsg: // 3 level
        filestream << QString("[%1] [%2:%3] Info: %4 (in function %5)\n").arg(timestamp).arg(context.file).arg(context.line).arg(msg).arg(context.function);
        abort();
      case QtInfoMsg: // 4 level
        filestream << QString("[%1] [%2:%3] Info: %4 (in function %5)\n").arg(timestamp).arg(context.file).arg(context.line).arg(msg).arg(context.function);
        break;
    }
  }

};



#endif // LOGGER_H
