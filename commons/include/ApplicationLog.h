#ifndef APPLICATION_LOG_H
#define APPLICATION_LOG_H

#include <stdio.h>
#include "EventLoop.h"
#include "FunctorWithResult.h"

class CApplicationLog
{
public:
  enum LOG_TYPE {
    LT_TRACE = 0,
    LT_INFO,
    LT_ERROR,
    LT_LAST
  };

  void SetDirectory(const char *directory);
  void SetLogLevel(LOG_TYPE lt) {m_log_level = lt;}

  void LogTrace( const char* format, ... );
  void LogInfo ( const char* format, ... );
  void LogError( const char* format, ... );

  static CApplicationLog* Instance(){
    static CApplicationLog m_instance;
    return &m_instance;
  }

private:
  static const int BUFFER_SIZE = 2048;
  static const char* LOG_FILE_DELIMITER;

  char m_messageBuffer[BUFFER_SIZE];
  std::string m_directory;


  std::string m_lst_files_by_log_type[LT_LAST];
  CEventLoop* m_logEventLoop;
  LOG_TYPE m_log_level;

  CApplicationLog(void);
  ~CApplicationLog(void);

  CApplicationLog(const CApplicationLog&);
  void operator=(const CApplicationLog&);

  static int AppendLog(const char *str, std::string &logFileName);
  void UpdateLogFilesNames(void);  
  void Log(CApplicationLog::LOG_TYPE log_type, std::string msg);

public:  
};

#endif //APPLICATION_LOG_H
