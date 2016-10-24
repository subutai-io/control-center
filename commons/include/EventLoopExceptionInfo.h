#ifndef EVENTLOOPEXCEPTIONINFO_H
#define EVENTLOOPEXCEPTIONINFO_H

#include <string>
  /*! \brief CEventLoopExceptionInfo provides low level information about SEH exception.
  */
typedef struct CEventLoopExceptionInfo
{
  void* ExceptionAddress;
  unsigned int ExceptionCode;
  std::string MethodName;
}CEventLoopExceptionInfo, *LPCEventLoopExceptionInfo;

#endif // EVENTLOOPEXCEPTIONINFO_H
