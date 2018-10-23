#include "Logger.h"
#include <QTextStream>
#include <QFile>
#include <QTime>
#include <QDirIterator>
#include <QTimer>
#include <QDebug>

/////////////////////////////////////////////////////////////////////////////////////////////////////

void Logger::Init() {
  if (!QDir(LogStorage()).exists())
  {
    if(!QDir().mkdir(LogStorage())) {
      qCritical() << "Can't create log folder " << LogStorage();
    }
  }

  if (m_file) {
    m_file->close();
    delete m_file;
  }

  QString file_path = QString("%1/logs_%2.txt").arg(LogStorage())
                                               .arg(QTime::currentTime().toString("HH"));
  m_file = new QFile(file_path);

  if (!m_file->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
    qCritical() << "Can't Open file: "
                << file_path;
    delete m_file;
    m_file = nullptr;
  }

  deleteOldFiles();
}
////////////////////////////////////////////////////////////////////////////

Logger::~Logger() {
  if (!m_file) return;
  m_file->close();

  delete m_file;
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
  static QMutex mutex;
  QMutexLocker lock(&mutex);

  if (typeToLevel(type) < (Logger::LOG_LEVEL)CSettingsManager::Instance().logs_level()) // comparing level of msg with currentLogLevel
     return;

  QString output_message = QString("[%1] [%2(%3)] %4: %5 (in function %6)\n")
                              .arg(QTime::currentTime().toString("HH:mm:ss"))
                              .arg(context.file)
                              .arg(context.line)
                              .arg(LogLevelToStr(typeToLevel(type)))
                              .arg(msg)
                              .arg(context.function);

  // log output to stdout
  QTextStream stdstream(stdout);
  stdstream << output_message;

  static QTextStream filestream(Logger::Instance()->file());
  filestream << output_message;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

void Logger::deleteOldFiles() {
  try {

    QDate currentDate = QDate::currentDate();
    QDirIterator it(CSettingsManager::Instance().logs_storage(),
                           QStringList() << "logs_directory_*", QDir::Dirs);


    while (it.hasNext()) {
      QString curDir = it.next();
      QFileInfo fileInformation(curDir);

      if (fileInformation.created().date().daysTo(currentDate) <= 4)
        continue;

      QDir dir(curDir);
      dir.removeRecursively();
    }

  } catch(...){
    /*do nothing here*/
  }
}

const QString& Logger::LogStorage() {
  static QString logs_storage = QString("%1%2logs_directory_%3%4").arg(
        CSettingsManager::Instance().logs_storage(),
          QDir::separator(),
          QDate::currentDate().toString("yyyy.MM.dd"),
          branch_name_str()
        );

  return logs_storage;
}

QFile* Logger::file() {
  return m_file;
}
