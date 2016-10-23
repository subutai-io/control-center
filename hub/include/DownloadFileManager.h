#ifndef DOWNLOADFILEMANAGER_H
#define DOWNLOADFILEMANAGER_H

#include <QObject>
#include <QString>
#include <QNetworkReply>
#include <QFile>

typedef enum download_file_manager_errors {
  DFME_SUCCESS = 0,
  DFME_ABORTED,
  DFME_NETWORK_ERROR
} download_file_manager_errors_t;
////////////////////////////////////////////////////////////////////////////

/*!
 * \brief This class is used for downloading files from kurjun. Emits signals "finished" and "progress"
 */
class CDownloadFileManager : public QObject {
  Q_OBJECT
private:
  QString m_kurjun_file_id;
  QString m_dst_file_path;
  int m_expected_size;
  QNetworkReply* m_network_reply;
  QFile* m_dst_file;

public:
  CDownloadFileManager(const QString& kurjun_file_id,
                       const QString& dst_file,
                       int expected_size);
  ~CDownloadFileManager();

private slots:

  void download_progress(qint64 read_bytes,
                         qint64 total_bytes);
  void network_reply_ready_read();
  void reply_finished();

public slots:
  /*!
   * \brief Starts downloading from kurjun. It's slot because we use it in separated thread.
   */
  void start_download();

  /*!
   * \brief Interrupts current downloading and emits signal finished(false).
   */
  void interrupt_download();

signals:
  void finished(bool success);
  void download_progress_sig(qint64 rec, qint64 total);
};

#endif // DOWNLOADFILEMANAGER_H
