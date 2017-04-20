#include <string.h>
#include <stdarg.h>

#include <QDir>
#include <QDebug>

#include "FileWrapper.h"
#include "ApplicationLog.h"

#ifndef RT_OS_WINDOWS
#include "commons/include/MRE_Pthread.h"
#else
#include "commons/include/MRE_Windows.h"
#endif

const char* CApplicationLog::LOG_FILE_DELIMITER =
    "|-----------------------------------------------------------------------------";

CApplicationLog::CApplicationLog( void ) :
  m_log_level(LT_INFO)
{
#ifndef RT_OS_WINDOWS
  m_logEventLoop = new CEventLoop<SynchroPrimitives::CPthreadMRE>(NULL, NULL, NULL, 5000, false);
#else
  m_logEventLoop = new CEventLoop<SynchroPrimitives::CWindowsManualResetEvent>(NULL, NULL, NULL, 5000, false);
#endif  
  m_logEventLoop->Run();
}

CApplicationLog::~CApplicationLog( void ) {
  delete m_logEventLoop;
}
//////////////////////////////////////////////////////////////////////////


void
CApplicationLog::UpdateLogFilesNames( void ) {
  static const char* lst_files_by_log_type_prefix[] =
  { "trace_", "info_", "error_", "disabled_" };

  for (int i = 0; i < LT_DISABLED; ++i) {
    m_lst_files_by_log_type[i] = m_directory + QDir::separator().toLatin1() +
                                 std::string(lst_files_by_log_type_prefix[i]) +
                                 CCommons::CurrentDateFileNameString();
  }
}
//////////////////////////////////////////////////////////////////////////

int
CApplicationLog::AppendLog(const char* str,
                           std::string& logFileName ) {
  try {
    CFileWrapper logFile(logFileName.c_str(), "a+");
    if (!logFile.IsValid())
      return -2;
    logFile.FPrintf("%s\r\n%s. %s\r\n\r\n", CApplicationLog::LOG_FILE_DELIMITER, CCommons::CurrentDateTimeString(), str);
  }
  catch(...) {
    return -1;
  }
  return 0;
}
//////////////////////////////////////////////////////////////////////////

void
CApplicationLog::SetDirectory(const std::string &directory ) {
  m_directory = directory;
  UpdateLogFilesNames();
}

void
CApplicationLog::SetLogLevel(CApplicationLog::LOG_TYPE lt) {
  m_log_level = lt;
}
//////////////////////////////////////////////////////////////////////////

void
CApplicationLog::Log(CApplicationLog::LOG_TYPE log_type,
                     std::string msg) {
  if (log_type < m_log_level) return;
  IFunctor* functor =
      new FunctorWithResult<int, const char*, std::string&>(AppendLog,
                                                            msg.c_str(),
                                                            m_lst_files_by_log_type[log_type],
                                                            "AppendLog");
#ifndef RT_OS_WINDOWS
  if (CEventLoop<SynchroPrimitives::CPthreadMRE>::GetSyncResult<int>(m_logEventLoop, functor, true) == -1)
    qCritical() << "AppendLog failed";
#else
  if (CEventLoop<SynchroPrimitives::CWindowsManualResetEvent>::GetSyncResult<int>(m_logEventLoop, functor, true) == -1)
    qCritical() << "AppendLog failed";
#endif
}
//////////////////////////////////////////////////////////////////////////

void
CApplicationLog::LogTrace(const char *format, ...) {
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

void
CApplicationLog::LogInfo( const char* format, ... ) {
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

void
CApplicationLog::LogError( const char* format, ... ) {
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
