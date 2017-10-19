#ifndef LOGGER_H
#define LOGGER_H

#include <QTextStream>
#include <QFile>
#include <QTime>
#include <QDirIterator>
#include "SettingsManager.h"

////////////////////////////////////////////////////////////////////////////

class Logger
{
public:
  enum LOG_LEVEL {LOG_DEBUG = 0, LOG_INFO, LOG_WARNING, LOG_CRITICAL, LOG_FATAL , LOG_DISABLED};
  LOG_LEVEL currentLogLevel;
  QFile file;
  LOG_LEVEL converter[LOG_DISABLED];

  Logger(){
    converter[(size_t)QtDebugMsg] = LOG_DEBUG;
    converter[(size_t)QtInfoMsg] = LOG_INFO;
    converter[(size_t)QtWarningMsg] = LOG_WARNING;
    converter[(size_t)QtCriticalMsg] = LOG_CRITICAL;
    converter[(size_t)QtFatalMsg] = LOG_FATAL;
    currentLogLevel = (LOG_LEVEL)CSettingsManager::Instance().logs_level();
  }
  void init (){
    deleteOldFiles();
  }

  int logLevel(){
    return (int)currentLogLevel;
  }

  LOG_LEVEL typeToLevel(QtMsgType type){
    return converter[(size_t)type];
  }

  void setLogLevel(LOG_LEVEL lt) {
    currentLogLevel = lt;
  }

  static Logger* Instance() {
    static Logger m_instance;
    return &m_instance;
  }

  static const QString& LogLevelToStr(LOG_LEVEL lt) {
    static QString ll_str[] = {"Debug", "Info", "Warning", "Critical", "Fatal", "Disabled"};
    return ll_str[lt];
  }

  static void LoggerMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
  {
    if (Logger::Instance()->typeToLevel(type) < Logger::Instance()->logLevel()) // comparing level of msg with currentLogLevel
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
    Logger::Instance()->file.setFileName(QString("%1/logs_%2.txt").arg(CSettingsManager::Instance().logs_storage()).arg(QDate::currentDate().toString("yyyy.MM.dd")));
    if (!Logger::Instance()->file.open(QIODevice::Append)) // if directory doesn't exist
      return;
    QTextStream filestream(&Logger::Instance()->file);

    filestream << output_message;
  }


	static void deleteOldFiles () {
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


};



#endif // LOGGER_H
