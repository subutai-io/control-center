#ifndef APPLICATION_LOG_H
#define APPLICATION_LOG_H

#include <stdio.h>
#include "EventLoop.h"
#include "FunctorWithResult.h"

class CApplicationLog
{
private:
  static const int BUFFER_SIZE = 2048;
  static const char* LOG_FILE_DELIMITER;

  char m_messageBuffer[BUFFER_SIZE];
  std::string m_directory;

  typedef enum LOG_TYPE {
    TRACE = 0, INFO, ERROR, LOG_TYPE_LAST
  } LOG_TYPE;
  std::string m_lst_files_by_log_type[LOG_TYPE_LAST];

  CEventLoop* m_logEventLoop;
  /*todo change to min level and dictionary of handlers*/
  bool m_enabled;

  CApplicationLog(void);
  ~CApplicationLog(void);

  CApplicationLog(const CApplicationLog&);
  void operator=(const CApplicationLog&);

  static int AppendLog(const char *str, std::string &logFileName);
  void UpdateLogFilesNames(void);


  void Log(CApplicationLog::LOG_TYPE log_type, std::string msg);

public:

  void SetDirectory(const char *directory);
  void LogTrace( const char* format, ...);
  void LogInfo( const char* format, ... );
  void LogError( const char* format, ... );
  void SetEnabled(bool enabled) {m_enabled = enabled;}

  static CApplicationLog* Instance(){
    static CApplicationLog m_instance;
    return &m_instance;
  }
};

#endif //APPLICATION_LOG_H
