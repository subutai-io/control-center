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

class CDownloadFileManager : public QObject {
  Q_OBJECT
private:
  QString m_file_id;
  QString m_dst_file_path;
  int m_expected_size;
  QNetworkReply* m_network_reply;
  QFile* m_dst_file;
  download_file_manager_errors_t m_last_error;

public:
  CDownloadFileManager(const QString& file_id,
                       const QString& dst_file,
                       int expected_size);
  ~CDownloadFileManager();

  download_file_manager_errors_t last_error() const {return m_last_error;}

private slots:

  void download_progress(qint64 read_bytes,
                         qint64 total_bytes);
  void network_reply_ready_read();
  void reply_finished();

public slots:
  void start_download();
  void interrupt_download();

signals:
  void finished();
  void download_progress_sig(qint64, qint64);
};

#endif // DOWNLOADFILEMANAGER_H
