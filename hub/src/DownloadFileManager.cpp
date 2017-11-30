#include "DownloadFileManager.h"
#include "RestWorker.h"
#include "NotificationObserver.h"

CDownloadFileManager::CDownloadFileManager(const QString &kurjun_file_id,
                                           const QString &dst_file,
                                           int expected_size) :
  m_kurjun_file_id(kurjun_file_id),
  m_dst_file_path(dst_file),
  m_expected_size(expected_size),
  m_network_reply(NULL),
  m_dst_file(NULL)
{
  m_dst_file = new QFile(m_dst_file_path);
  m_dst_file->open(QIODevice::WriteOnly);
}

CDownloadFileManager::~CDownloadFileManager() {
  if (m_network_reply != NULL)
    m_network_reply->deleteLater();

  if (m_dst_file != NULL) {
    m_dst_file->flush();
    m_dst_file->close();
    delete m_dst_file;
  }
}
////////////////////////////////////////////////////////////////////////////

void
CDownloadFileManager::download_progress(qint64 read_bytes,
                                        qint64 total_bytes) {
  emit download_progress_sig(read_bytes, total_bytes);
}
////////////////////////////////////////////////////////////////////////////

void
CDownloadFileManager::network_reply_ready_read() {
  if (m_dst_file == NULL) return;
  m_dst_file->write(m_network_reply->readAll());
}
////////////////////////////////////////////////////////////////////////////

void
CDownloadFileManager::reply_finished() {
  //DON'T REMOVE THIS !!!!
  if (m_dst_file != NULL) {
    m_dst_file->flush();
    m_dst_file->close();
  }

  if (m_network_reply->error() != QNetworkReply::NoError) {
    CNotificationObserver::Instance()->Error(
          tr("Download file error. %1").arg(m_network_reply->errorString()), DlgNotification::N_NO_ACTION);
    qCritical("Download file error : %s",
                                          m_network_reply->errorString().toStdString().c_str());
  } else {
      qInfo("Download file %s finished", m_dst_file_path.toStdString().c_str());
  }

  emit finished(m_network_reply->error() == QNetworkReply::NoError);
}
////////////////////////////////////////////////////////////////////////////

void
CDownloadFileManager::start_download() {
  if (m_network_reply != NULL) return;
  m_network_reply = CRestWorker::Instance()->download_gorjun_file(m_kurjun_file_id);
  connect(m_network_reply, &QNetworkReply::downloadProgress,
          this, &CDownloadFileManager::download_progress);
  connect(m_network_reply, &QNetworkReply::readyRead,
          this, &CDownloadFileManager::network_reply_ready_read);
  connect(m_network_reply, &QNetworkReply::finished,
          this, &CDownloadFileManager::reply_finished);
}
////////////////////////////////////////////////////////////////////////////

void
CDownloadFileManager::interrupt_download() {
  m_network_reply->abort();
  reply_finished();
}
////////////////////////////////////////////////////////////////////////////
