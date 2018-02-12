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
#include <QTableWidget>

namespace Ui {
  class DlgTransferFile;
}
enum FILE_OPERATION_TYPE{
  FILE_UPLOAD = 0,
  FILE_DOWNLOAD,
};

enum FILE_UPLOAD_STATUS
{
  UPLOAD_SUCCESS = 0,
  UPLOAD_FAIL,
  UPLOAD_WAIT,
  UPLOAD_TIMEOUT,
  UPLOAD_NOT_STARTED
};

enum FILE_TYPE
{
  FILE_SIMPLE = 0,
  FILE_DIRECTORY,
  FILE_UNKNOWN
};

class OneFile
{
  QString m_fileName;
  QString m_filePath;
  QDateTime m_created;

  quint64 m_fileSize;
  FILE_UPLOAD_STATUS m_fileStatus;
  FILE_TYPE m_fileType;

public:
  const QString &fileName() const {
    return m_fileName;
  }
  const QString &filePath() const {
    return m_filePath;
  }

  FILE_TYPE fileType() const {
    return m_fileType;
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
  std::vector <OneFile> remote_files;
  std::vector <OneFile> local_files;

private:
  void local_back();
  void remote_back();
  void path_remote_changed(const QString &new_path);
  void path_local_changed(const QString &new_path);

  void design_table_widget(QTableWidget *tw, const QStringList &headers);

  Ui::DlgTransferFile *ui;
  void add_file_local(const QFileInfo &fi);
  void fill_with_local_dir();

  bool is_uploading;
  int cnt_upload_files;
  void check_more_info(bool checked);
  void add_file_remote(const QString file_name, const QString file_size);
  void refresh_remote_file_system();
  void refresh_local_file_system();
  void Init();
  void file_to_download();
  void file_to_upload();
  void upload_selected();
  void download_selected();

  void add_file(QTableWidget *tw, int row, const std::vector<QString> &values) ;
  void refresh_button();

  void local_file_system_directory_selected(const QModelIndex &index);

  void file_local_selected(const QModelIndex &index);
  void file_remote_selected(const QModelIndex &index);


  void set_upload_status (bool uploading);
  void add_file(const QString &filename);
  void clear_all_files();
  void upload_files();
  void select_file();
  void upload_start(int file_id);
  QDir current_local_dir;
  QString current_remote_dir;

};

#endif // DLGTRANSFERFILE_H
