#include "IFunctor.h"

IFunctor::IFunctor( const char* methodName ) :
  m_isSynchronized(true), m_exceptionOccured(false),
  m_timeoutOccured(false), m_methodName(methodName) {
  //init IFunctor :)))))
}
//////////////////////////////////////////////////////////////////////////
