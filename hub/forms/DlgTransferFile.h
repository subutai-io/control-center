#ifndef DLGTRANSFERFILE_H
#define DLGTRANSFERFILE_H

#include <QDialog>
#include <QDragEnterEvent>
#include <QDebug>
#include <QListWidget>
#include <QListWidget>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDebug>
#include "P2PController.h"
#include <QFileSystemModel>

namespace Ui {
  class DlgTransferFile;
}

enum FILE_UPLOAD_STATUS
{
  UPLOAD_SUCCESS = 0,
  UPLOAD_FAIL,
  UPLOAD_WAIT,
  UPLOAD_TIMEOUT,
  UPLOAD_NOT_STARTED
};

struct file_to_upload
{
public:
  file_to_upload(const QFileInfo &fi) {
    file_name = fi.fileName();
    file_path = fi.filePath();
    file_size = fi.size();
    file_status = UPLOAD_NOT_STARTED;
    file_created = fi.created();
  }


  QString file_name;
  QString file_path;
  QDateTime file_created;
  qint64 file_size;
  FILE_UPLOAD_STATUS file_status;
};

enum FILE_TYPE {
  FILE_SIMPLE = 0,
  FILE_DIR,
  FILE_BACK
};

enum HOST_MACHINE_TYPE {
  HOST_MACHINE_REMOTE = 0,
  HOST_MACHINE_LOCAL
};

class File {
  QString m_fileName;
  QString m_filePath;
  QDateTime m_created;

  quint64 m_fileSize;
  FILE_TYPE m_fileType;
  HOST_MACHINE_TYPE m_hostMachine;

  const QString &fileName() const {
    return m_fileName;
  }
  const QString &filePath() const {
    return m_filePath;
  }
  FILE_TYPE fileType() const {
    return m_fileType;
  }

  HOST_MACHINE_TYPE hostMachine() const {
    return m_hostMachine;
  }

  quint64 fileSize() const {
    return m_fileSize;
  }
};

class CustomListWidget : public QListWidget
{
  Q_OBJECT
public:
  explicit CustomListWidget(QWidget *parent = Q_NULLPTR) :
    QListWidget(parent) {
    setAcceptDrops(true);
  }
private:
  void dragEnterEvent(QDragEnterEvent *event) override {
    setBackgroundRole(QPalette::Highlight);
    if (event->mimeData()->hasUrls()) {
      event->acceptProposedAction();
    }
  }

  void dragMoveEvent(QDragMoveEvent *event) override {
    event->acceptProposedAction();
  }

  void dropEvent(QDropEvent *event) override {
    foreach (const QUrl &url, event->mimeData()->urls()) {
      QString fileName = url.toLocalFile();
      this->addItem(fileName);
    }
  }
};

class DlgTransferFile : public QDialog
{
  Q_OBJECT

public:
  explicit DlgTransferFile(QWidget *parent = 0);
  ~DlgTransferFile();
  void addSSHKey(const QString &key);
  void addIPPort(const QString &ip, const QString &port);
  void addUser(const QString &user);
  void upload_finished(system_call_wrapper_error_t res, int file_id);

private:
  Ui::DlgTransferFile *ui;
  std::vector <file_to_upload> files_to_upload;


  bool is_uploading;
  int cnt_upload_files;

  void local_file_system_directory_selected(const QModelIndex &index);
  QFileSystemModel *file_system_model;

  void set_upload_status (bool uploading);
  void add_file(const QString &filename);
  void clear_all_files();
  void upload_files();
  void select_file();
  void upload_start(int file_id);


private slots:
  void file_dropped(const QString &filename);
};

#endif // DLGTRANSFERFILE_H