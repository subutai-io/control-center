#ifndef VIRTUALBOXHDR_H
#define VIRTUALBOXHDR_H

#if defined(RT_OS_LINUX) || defined(RT_OS_DARWIN)
#include <VirtualBox_XPCOM.h>
#include <nsXPCOM.h>
#include <nsIMemory.h>
#include <nsIServiceManager.h>
#include <nsIEventQueue.h>
#include <nsEventQueueUtils.h>
#elif RT_OS_WINDOWS
#include <VirtualBox.h>
typedef HRESULT nsresult;
#endif


#endif // VIRTUALBOXHDR_H

