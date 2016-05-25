#ifndef EVENTLOOPEXCEPTIONINFO_H
#define EVENTLOOPEXCEPTIONINFO_H

#include <string>

typedef struct CEventLoopExceptionInfo
{
  void* ExceptionAddress;
  unsigned int ExceptionCode;
  std::string MethodName;
}CEventLoopExceptionInfo, *LPCEventLoopExceptionInfo;

#endif // EVENTLOOPEXCEPTIONINFO_H
