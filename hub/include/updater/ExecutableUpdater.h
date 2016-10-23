#ifndef EXECUTABLEUPDATER_H
#define EXECUTABLEUPDATER_H

#include <QObject>
#include <QString>

namespace update_system {
  /*!
   * \brief Replaces one executable with another. Then generates `finished` signal.
   */
  class CExecutableUpdater : public QObject {
    Q_OBJECT
  private:
    QString m_file_id;
    QString m_src_file_str;
    QString m_dst_file_str;

    CExecutableUpdater();

  public:

    /*!
     * \brief Constructor
     * \param src - source file
     * \param dst - destination file that will be replaced by source file
     */
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
