#ifndef APPLICATION_LOG_H
#define APPLICATION_LOG_H

#include <stdio.h>
#include "EventLoop.h"
#include "FunctorWithResult.h"
#ifndef RT_OS_WINDOWS
#include "commons/include/MRE_Linux.h"
#else
#include "commons/include/MRE_Windows.h"
#endif

/*!
 * \brief The CApplicationLog class. Uses for logging.
 * It generates trace_($current_date).txt, info_($current_date).txt and error_($current_date).txt
 * You can specify log_level via application argument --l
 */
class CApplicationLog
{
public:
  /*!
   * \brief The LOG_TYPE enum - log level enum
   */
  enum LOG_TYPE {
    LT_TRACE = 0,
    LT_INFO,
    LT_ERROR,
    LT_LAST
  };

  /*!
   * \brief Set directory where log files will be generated
   * \param directory - path to directory.
   */
  void SetDirectory(const std::string& directory);
  /*!
   * \brief Set log level.
   * \param lt - log level. See LOG_TYPE enum.
   */
  void SetLogLevel(LOG_TYPE lt) {m_log_level = lt;}

  /*!
   * \brief LogTrace - write log message to trace_$(current_date).txt file
   * \param format like in printf, fprintf functions.
   */
  void LogTrace( const char* format, ... );

  /*!
   * \brief LogInfo - write log message to info_$(current_date).txt file
   * \param format like in printf, fprintf functions.
   */
  void LogInfo ( const char* format, ... );

  /*!
   * \brief LogError - write log message to error_$(current_date).txt file
   * \param format like in printf, fprintf functions.
   */
  void LogError( const char* format, ... );

  /*!
   * \brief  CApplicationLog is singleton.
   * \return CApplicationLog instance.
   */
  static CApplicationLog* Instance(){
    static CApplicationLog m_instance;
    return &m_instance;
  }

private:
  //buffer size for log messages.
  static const int BUFFER_SIZE = 2048;

  //some string between log messages
  static const char* LOG_FILE_DELIMITER;

  //buffer for log messages.
  char m_messageBuffer[BUFFER_SIZE];

  //directory where log files will be located.
  std::string m_directory;

  //for fast access to file name by LOG_TYPE
  std::string m_lst_files_by_log_type[LT_LAST];

  //event loop for logging.
#ifndef RT_OS_WINDOWS
  CEventLoop<SynchroPrimitives::CLinuxManualResetEvent> *m_logEventLoop;
#else
  CEventLoop<SynchroPrimitives::CWindowsManualResetEvent> *m_logEventLoop;
#endif

  //current log level
  LOG_TYPE m_log_level;

  CApplicationLog(void);
  ~CApplicationLog(void);

  //copy constructor and asssignment are prohibited
  CApplicationLog(const CApplicationLog&);
  void operator=(const CApplicationLog&);

  //internal function for appending message to logFileName
  static int AppendLog(const char *str, std::string &logFileName);

  //constructing file names based on current date
  void UpdateLogFilesNames(void);  

  //Internal log function.
  void Log(CApplicationLog::LOG_TYPE log_type, std::string msg);

public:  
};

#endif //APPLICATION_LOG_H
