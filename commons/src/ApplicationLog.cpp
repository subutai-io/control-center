#include <string.h>
#include <stdarg.h>
#include "ApplicationLog.h"
#include "FileWrapper.h"

#include <QDir>
#include <QDebug>

const char* CApplicationLog::LOG_FILE_DELIMITER =
    "|-----------------------------------------------------------------------------";

CApplicationLog::CApplicationLog( void ) :
  m_log_level(LT_ERROR)
{
  m_logEventLoop = new CEventLoop(NULL, NULL, NULL, 5000, false);
  m_logEventLoop->Run();
}

CApplicationLog::~CApplicationLog( void ) {
  delete m_logEventLoop;
}
//////////////////////////////////////////////////////////////////////////


void CApplicationLog::UpdateLogFilesNames( void ) {
  static const char* lst_files_by_log_type_prefix[] =
  { "trace_", "info_", "error_", "undefined_" };

  for (int i = 0; i < LT_LAST; ++i) {
    m_lst_files_by_log_type[i] = m_directory + QDir::separator().toLatin1() +
                                 std::string(lst_files_by_log_type_prefix[i]) +
                                 CCommons::CurrentDateFileNameString();
  }
}
//////////////////////////////////////////////////////////////////////////

int CApplicationLog::AppendLog(const char* str, std::string& logFileName ) {
  try {
    CFileWrapper logFile(logFileName.c_str(), "a+");
    logFile.FPrintf("%s\r\n%s. %s\r\n\r\n", CApplicationLog::LOG_FILE_DELIMITER, CCommons::CurrentDateTimeString(), str);
  }
  catch(...) {
    return -1;
  }
  return 0;
}
//////////////////////////////////////////////////////////////////////////

void CApplicationLog::SetDirectory( const char* directory ) {
  m_directory = std::string(directory);
  UpdateLogFilesNames();
}
//////////////////////////////////////////////////////////////////////////

void CApplicationLog::Log(CApplicationLog::LOG_TYPE log_type, std::string msg) {
#ifndef RT_OS_WINDOWS
  if (log_type < m_log_level) return;
  IFunctor* functor =
      new FunctorWithResult<int, const char*, std::string&>(AppendLog,
                                                            msg.c_str(),
                                                            m_lst_files_by_log_type[log_type],
                                                            "AppendLog");
  if (CEventLoop::GetSyncResult<int>(m_logEventLoop, functor, true) == -1)
    qCritical() << "AppendLog failed";
#endif
}
//////////////////////////////////////////////////////////////////////////

void CApplicationLog::LogTrace(const char *format, ...) {
  va_list args;
  int n = 0;
  va_start(args, format);
  n = vsnprintf(m_messageBuffer, BUFFER_SIZE, format, args);
  va_end(args);
  qDebug() << m_messageBuffer;
  if (n == -1) return;
  Log(LT_TRACE, m_messageBuffer);
}
////////////////////////////////////////////////////////////////////////////

void CApplicationLog::LogInfo( const char* format, ... ) {
  va_list args;
  int n = 0;
  va_start(args, format);
  n = vsnprintf(m_messageBuffer, BUFFER_SIZE, format, args);
  va_end(args);
  qDebug() << m_messageBuffer;
  if (n == -1) return;
  Log(LT_INFO, m_messageBuffer);
}
//////////////////////////////////////////////////////////////////////////

void CApplicationLog::LogError( const char* format, ... ) {
  va_list args;
  int n = 0;
  va_start(args, format);
  n = vsnprintf(m_messageBuffer, BUFFER_SIZE, format, args);
  va_end(args);
  qCritical() << m_messageBuffer;
  if (n == -1) return;
  Log(LT_ERROR, m_messageBuffer);
}
//////////////////////////////////////////////////////////////////////////
