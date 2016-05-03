#include "DownloadFileManager.h"
#include "RestWorker.h"
#include <QDebug>

CDownloadFileManager::CDownloadFileManager(const QString &file_id,
                                           const QString &dst_file,
                                           int expected_size) :
  m_file_id(file_id),
  m_dst_file_path(dst_file),
  m_expected_size(expected_size),
  m_network_reply(NULL),
  m_dst_file(NULL),
  m_last_error(DFME_SUCCESS)
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
  qDebug() << "download_progress rb = " << read_bytes << ", tb = " << total_bytes;
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
  m_last_error = m_network_reply->error() != QNetworkReply::NoError ?
                                               DFME_NETWORK_ERROR : DFME_SUCCESS;
  emit finished();
}
////////////////////////////////////////////////////////////////////////////

void
CDownloadFileManager::start_download() {
  if (m_network_reply != NULL) return;
  m_network_reply = CRestWorker::Instance()->download_gorjun_file(m_file_id);
  connect(m_network_reply, SIGNAL(downloadProgress(qint64,qint64)),
          this, SLOT(download_progress(qint64,qint64)));
  connect(m_network_reply, SIGNAL(readyRead()),
          this, SLOT(network_reply_ready_read()));
  connect(m_network_reply, SIGNAL(finished()),
          this, SLOT(reply_finished()));
}
////////////////////////////////////////////////////////////////////////////

void
CDownloadFileManager::interrupt_download() {
  m_network_reply->abort();
  reply_finished();
}
////////////////////////////////////////////////////////////////////////////
