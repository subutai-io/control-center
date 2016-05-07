#include <QFile>
#include "ExecutableUpdater.h"
#include "Commons.h"

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

void
CExecutableUpdater::replace_executables() {
  rename(m_dst_file_str.toStdString().c_str(),
         CCommons::AppNameTmp().toStdString().c_str());
  if (rename(m_src_file_str.toStdString().c_str(),
         m_dst_file_str.toStdString().c_str()) == 0) {
    QFile f(m_dst_file_str);
    f.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                     QFile::ReadUser  | QFile::WriteUser  | QFile::ExeUser  |
                     QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup);
  }
  emit finished();
}
////////////////////////////////////////////////////////////////////////////
