#include "ExecutableUpdater.h"

CExecutableUpdater::CExecutableUpdater(const QString &src,
                                       const QString &dst) :
  m_src_file_str(src),
  m_dst_file_str(dst)
{
}
////////////////////////////////////////////////////////////////////////////

CExecutableUpdater::~CExecutableUpdater() {
}
////////////////////////////////////////////////////////////////////////////

void CExecutableUpdater::replace_executables() {
#ifdef RT_OS_LINUX
  rename(m_src_file_str.toStdString().c_str(),
         m_dst_file_str.toStdString().c_str());
#endif
#ifdef RT_OS_DARWIN
  rename(m_src_file_str.toStdString().c_str(),
         m_dst_file_str.toStdString().c_str());
#endif
  emit finished();
}
////////////////////////////////////////////////////////////////////////////
