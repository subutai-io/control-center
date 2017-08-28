#ifndef EXECUTABLEUPDATER_H
#define EXECUTABLEUPDATER_H

#include <QObject>
#include <QString>

namespace update_system {
  class CExecutableUpdater : public QObject {
    Q_OBJECT
  private:
    QString m_file_id;
    QString m_src_file_str;
    QString m_dst_file_str;

    CExecutableUpdater();

  public:

    CExecutableUpdater(const QString& src,
                       const QString& dst);
    ~CExecutableUpdater();

  public slots:
    void replace_executables(bool was_successful_downloaded);

  signals:
    void finished(bool);
  };
}

#endif // EXECUTABLEUPDATER_H
