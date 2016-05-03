#ifndef EXECUTABLEUPDATER_H
#define EXECUTABLEUPDATER_H

#include <QObject>
#include <QString>

class CExecutableUpdater : public QObject {
  Q_OBJECT
private:
  QString m_src_file_str;
  QString m_dst_file_str;

  CExecutableUpdater();

public:
  CExecutableUpdater(const QString& src,
                     const QString& dst);
  ~CExecutableUpdater();

public slots:
  void replace_executables();

signals:
  void finished();
};

#endif // EXECUTABLEUPDATER_H
