#ifndef LOGGER_H
#define LOGGER_H

#include <QTextStream>
#include <QFile>
#include <QTime>
#include "SettingsManager.h"


static QtMsgType currentLogLevel;

class Logger
{
public:
    static int logLevel(){
      return (int)currentLogLevel;
    }

    static void setLogLevel(QtMsgType lt) {
        currentLogLevel = lt;
    }

    static const QString& Logger::LogLevelToStr(QtMsgType lt) {
      static QString ll_str[] = {"Debug", "Warning", "Critical", "Fatal" , "Info" , "Disabled"};
      return ll_str[lt];
    }

    static void Logger::LoggerMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
      if ((int)type < logLevel()){
        return;
      }

      QFile file;
      file.setFileName(QString("%1\\logs_%2.txt").arg(CSettingsManager::Instance().logs_storage()).arg(QDate::currentDate().toString("yyyy.MM.dd")));
      file.open(QIODevice::Append);

      QTextStream stdstream(stdout);
      QTextStream filestream(&file);
      QString timestamp = (QTime::currentTime().toString("HH:mm:ss"));

      switch (type) {
        case QtDebugMsg: // 0 level
          stdstream << QString("[%1] [%2:%3] Debug: %4 (in function %5)\n").arg(timestamp).arg(context.file).arg(context.line).arg(msg).arg(context.function);
          filestream << QString("[%1] [%2:%3] Debug: %4 (in function %5)\n").arg(timestamp).arg(context.file).arg(context.line).arg(msg).arg(context.function);
          break;
        case QtWarningMsg: // 1 level
          stdstream << QString("[%1] [%2:%3] Info: %4 (in function %5)\n").arg(timestamp).arg(context.file).arg(context.line).arg(msg).arg(context.function);
          filestream << QString("[%1] [%2:%3] Info: %4 (in function %5)\n").arg(timestamp).arg(context.file).arg(context.line).arg(msg).arg(context.function);
          break;
        case QtCriticalMsg: // 2 level
          stdstream << QString("[%1] [%2:%3] Info: %4 (in function %5)\n").arg(timestamp).arg(context.file).arg(context.line).arg(msg).arg(context.function);
          filestream << QString("[%1] [%2:%3] Info: %4 (in function %5)\n").arg(timestamp).arg(context.file).arg(context.line).arg(msg).arg(context.function);
          break;
        case QtFatalMsg: // 3 level
          stdstream << QString("[%1] [%2:%3] Info: %4 (in function %5)\n").arg(timestamp).arg(context.file).arg(context.line).arg(msg).arg(context.function);
          filestream << QString("[%1] [%2:%3] Info: %4 (in function %5)\n").arg(timestamp).arg(context.file).arg(context.line).arg(msg).arg(context.function);
          abort();
        case QtInfoMsg: // 4 level
          stdstream << QString("[%1] [%2:%3] Info: %4 (in function %5)\n").arg(timestamp).arg(context.file).arg(context.line).arg(msg).arg(context.function);
          filestream << QString("[%1] [%2:%3] Info: %4 (in function %5)\n").arg(timestamp).arg(context.file).arg(context.line).arg(msg).arg(context.function);
          break;
       }
    }

};



#endif // LOGGER_H
