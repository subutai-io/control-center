#include <QFile>
#include "Commons.h"
#include "ApplicationLog.h"
#include "NotificationObserver.h"
#include "updater/ExecutableUpdater.h"

using namespace update_system;

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
CExecutableUpdater::replace_executables(bool was_successful_downloaded) {
  if (!was_successful_downloaded)
    return;

  QString tmp = m_dst_file_str + ".tmp";
  QFile src(m_src_file_str);
  QFile dst(m_dst_file_str);
  bool replaced = true;
  QFile::Permissions perm;
  if (dst.exists()) {
    perm = dst.permissions();
  } else {
    perm = QFile::ReadUser | QFile::WriteUser | QFile::ExeUser  |
           QFile::ReadGroup | QFile::ExeGroup |
           QFile::ReadOther | QFile::ExeOther ; // 0x0755 :)
  }

  CApplicationLog::Instance()->LogTrace("dst : %s", m_dst_file_str.toStdString().c_str());
  CApplicationLog::Instance()->LogTrace("tmp : %s", tmp.toStdString().c_str());
  CApplicationLog::Instance()->LogTrace("src : %s", m_src_file_str.toStdString().c_str());

  do {    
    if (dst.exists()) {      

      QFile ftmp(tmp);
      if (ftmp.exists() && !ftmp.remove()) {
        CApplicationLog::Instance()->LogError("remove tmp file %s failed. %s",
                                              tmp.toStdString().c_str(),
                                              ftmp.errorString().toStdString().c_str());
        break;
      }

      if (!(replaced &= dst.rename(tmp))) {
        CApplicationLog::Instance()->LogError("rename %s to %s failed. %s",
                                              m_dst_file_str.toStdString().c_str(),
                                              tmp.toStdString().c_str(),
                                              dst.errorString().toStdString().c_str());
        break;
      }
    }

    if (!(replaced &= src.copy(m_dst_file_str))) {
      CApplicationLog::Instance()->LogError("copy %s to %s failed. %s",
                                            m_src_file_str.toStdString().c_str(),
                                            m_dst_file_str.toStdString().c_str(),
                                            src.errorString().toStdString().c_str());
      break;
    }

#ifndef RT_OS_WINDOWS
    if (!(replaced &= dst.setPermissions(m_dst_file_str, perm))) {
      CApplicationLog::Instance()->LogError("set permission to file %s failed", m_dst_file_str.toStdString().c_str());
      break;
    }
#endif
  } while (0);

  emit finished(replaced);
}
////////////////////////////////////////////////////////////////////////////
