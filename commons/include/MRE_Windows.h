#ifndef MRE_WINDOWS_H
#define MRE_WINDOWS_H

#include <Windows.h>
#include "InternalCriticalSection.h"

#include <string>

class CGuidCreator
{
public:
#ifdef UNICODE
  static std::wstring CreateGuidString(void) {
    GUID guid;
    CoCreateGuid(&guid);
    RPC_WSTR rpcStr;
    UuidToStringW( &guid, &rpcStr );
    wchar_t* pwStr = reinterpret_cast<wchar_t*>( rpcStr );
    std::wstring result(pwStr);
    RpcStringFreeW( &rpcStr );
    return result;
  }

#else
  static std::string CreateGuidString(void) {
    GUID guid;
    CoCreateGuid(&guid);
    RPC_CSTR rpcStr;
    UuidToStringA( &guid, &rpcStr );
    char* pStr = reinterpret_cast<char*>( rpcStr );
    std::string result(pwStr);
    RpcStringFreeA( &rpcStr );
    return result;
  }
#endif
};

namespace SynchroPrimitives {

  class CWindowsManualResetEvent {
  private:
    HANDLE m_mre;

    CWindowsManualResetEvent(const CWindowsManualResetEvent&);
    void operator=(const CWindowsManualResetEvent&);

  public:
    CWindowsManualResetEvent(void){}
    ~CWindowsManualResetEvent(void){}

    static int MRE_Init(CWindowsManualResetEvent* lpMre) {
      lpMre->m_mre = CreateEvent(NULL, TRUE, FALSE, CGuidCreator::CreateGuidString().c_str()) ;
      return 0;
    }
    static int MRE_Wait(CWindowsManualResetEvent* lpMre, int timeInMs) { return WaitForSingleObject (lpMre->m_mre, timeInMs) ;}
    static int MRE_Set(CWindowsManualResetEvent* lpMre) {return SetEvent(lpMre->m_mre);}
    static int MRE_Reset(CWindowsManualResetEvent* lpMre) {return ResetEvent(lpMre->m_mre);}
    static int MRE_Destroy(CWindowsManualResetEvent* lpMre) {return CloseHandle(lpMre->m_mre);}
  };
}

#endif // MRE_WINDOWS_H
