#ifndef APPLICATION_LOG_H
#define APPLICATION_LOG_H

#include <stdio.h>
#include "EventLoop.h"
#include "FunctorWithResult.h"
#ifndef RT_OS_WINDOWS
#include "commons/include/MRE_Pthread.h"
#else
#include "commons/include/MRE_Windows.h"
#endif

class CApplicationLog {
 public:
  enum LOG_TYPE { LT_TRACE = 0, LT_INFO, LT_ERROR, LT_DISABLED };
  static const QString& LogLevelToStr(LOG_TYPE lt);

  void SetDirectory(const std::string& directory);
  void SetLogLevel(LOG_TYPE lt);
  LOG_TYPE LogLevel() const { return m_log_level; }
  void LogTrace(const char* format, ...);
  void LogInfo(const char* format, ...);
  void LogError(const char* format, ...);
  static CApplicationLog* Instance() {
    static CApplicationLog m_instance;
    return &m_instance;
  }

 private:
  static const int BUFFER_SIZE = 1024 * 4;

  static const char* LOG_FILE_DELIMITER;

  char m_messageBuffer[BUFFER_SIZE];

  std::string m_directory;

  std::string m_lst_files_by_log_type[LT_DISABLED];

#ifndef RT_OS_WINDOWS
  CEventLoop<SynchroPrimitives::CPthreadMRE>* m_logEventLoop;
#else
  CEventLoop<SynchroPrimitives::CWindowsManualResetEvent>* m_logEventLoop;
#endif

  LOG_TYPE m_log_level;

  CApplicationLog(void);
  ~CApplicationLog(void);

  CApplicationLog(const CApplicationLog&);
  void operator=(const CApplicationLog&);

  static int AppendLog(const char* str, std::string& logFileName);

  // constructing file names based on current date
  void UpdateLogFilesNames(void);

  void Log(CApplicationLog::LOG_TYPE log_type, std::string msg);

 public:
};

#endif  // APPLICATION_LOG_H
