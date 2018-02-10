#include "DlgTransferFile.h"
#include "ui_DlgTransferFile.h"
#include "QFileDialog"
#include "NotificationObserver.h"
#include <QDateTime>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <QFileSystemModel>

#include <QTableWidgetItem>

static QStringList headers;

DlgTransferFile::DlgTransferFile(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgTransferFile){

  qInfo() <<"Open new upload dialog";

  headers << "File Name"
          << "Size"
          << "Date"
          << "Path"
          << "Status";

  ui->setupUi(this);
  this->ui->groupBox->setVisible(false);
  ui->tableWidget->setColumnCount(5);
  ui->tableWidget->setHorizontalHeaderLabels(headers);
  ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  ui->tableWidget->verticalHeader()->setVisible(true);
  ui->tableWidget->setShowGrid(false);
  this->setMinimumWidth(this->width());

  connect(ui->more_info, &QCheckBox::toggled, [this](bool checked){
    this->ui->groupBox->setVisible(checked);
    this->adjustSize();
  });
  connect(ui->btn_upload_files, &QPushButton::clicked,
          this, &DlgTransferFile::upload_files);
  connect(ui->btn_choose_files, &QPushButton::clicked,
          this, &DlgTransferFile::select_file);
  connect(ui->btn_clear, &QPushButton::clicked,
          this, &DlgTransferFile::clear_all_files);

  file_system_model = new QFileSystemModel(this);

  ui->local_file_system->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  ui->local_file_system->setModel(file_system_model);

  ui->local_file_system->setRootIndex(file_system_model->index(QDir::currentPath()));
  file_system_model->setRootPath("/");

  ui->local_file_system->verticalHeader()->setVisible(true);
  ui->local_file_system->setShowGrid(false);

  connect(ui->local_file_system, &QTableView::doubleClicked,
          this, &DlgTransferFile::local_file_system_directory_selected);
}

void DlgTransferFile::local_file_system_directory_selected(const QModelIndex &index) {
  QVariant data = ui->local_file_system->model()->data(
                    ui->local_file_system->model()->index(index.row(), 0)
                  );

  CNotificationObserver::Instance()->Info(data.toString(), DlgNotification::N_ABOUT);

 // ui->local_file_system->setRootIndex(
  //file_system_model->setRootPath(data.toString()));

}

////////////////////////////////////////////////////////////////////////

void DlgTransferFile::addSSHKey(const QString &key) {
  ui->remote_ssh_key_path->setText(key);
}
void DlgTransferFile::addIPPort(const QString &ip, const QString &port) {
  ui->remote_ip->setText(ip);
  ui->remote_port->setText(port);
}
void DlgTransferFile::addUser(const QString &user) {
  ui->remote_user->setText(user);
}

////////////////////////////////////////////////////////////////////////

void DlgTransferFile::clear_all_files(){
  qInfo()<<"Clear files list";

  files_to_upload.clear();
  ui->tableWidget->clear();
  ui->progressBar->setValue(0);
  ui->tableWidget->setRowCount(0);
  ui->tableWidget->setHorizontalHeaderLabels(headers);
}

////////////////////////////////////////////////////////////////////////

void DlgTransferFile::add_file(const QString &filename) {
  qInfo() << "Add file " << filename;

  // status icons
  static QIcon icon_not_upload(":/hub/OK.png");

  QFileInfo fi(filename);
  file_to_upload new_file(fi);

  int last_index = ui->tableWidget->rowCount();

  QTableWidgetItem *file_name = new QTableWidgetItem(new_file.file_name);
  QTableWidgetItem *file_size = new QTableWidgetItem(QString::number(new_file.file_size/1024) + "KB");
  QTableWidgetItem *file_date = new QTableWidgetItem(new_file.file_created.date().toString());
  QTableWidgetItem *file_path = new QTableWidgetItem(new_file.file_path);
  QTableWidgetItem *upload_status = new QTableWidgetItem(icon_not_upload, "Not yet uploaded");

  ui->tableWidget->insertRow(last_index);

  ui->tableWidget->setItem(last_index, 0, file_name);
  ui->tableWidget->setItem(last_index, 1, file_size);
  ui->tableWidget->setItem(last_index, 2, file_date);
  ui->tableWidget->setItem(last_index, 3, file_path);
  ui->tableWidget->setItem(last_index, 4, upload_status);

  files_to_upload.push_back(new_file);
}

////////////////////////////////////////////////////////////////////////

void DlgTransferFile::select_file() {
  QString fn = QFileDialog::getOpenFileName(this, tr("File to upload"));
  if (fn == "")
    return;
  add_file(fn);
}

////////////////////////////////////////////////////////////////////////

void DlgTransferFile::file_dropped(const QString &filename) {
  if (is_uploading)
    return;
  add_file(filename);
}

////////////////////////////////////////////////////////////////////////

void DlgTransferFile::set_upload_status (bool uploading) {
  ui->btn_choose_files->setEnabled(!uploading);
  ui->btn_upload_files->setEnabled(!uploading);
  ui->btn_clear->setEnabled(!uploading);
  is_uploading = uploading;
  if (is_uploading) {
    cnt_upload_files = 0;
    ui->progressBar->setMaximum(files_to_upload.size());
  }
}

////////////////////////////////////////////////////////////////////////

void DlgTransferFile::upload_start(int file_id) {
  static QIcon uploading(":/hub/uploading.png");

  QString ip = ui->remote_ip->text();
  QString user = ui->remote_user->text();
  QString port = ui->remote_port->text();
  QString destination = ui->remote_destination->text();
  QString file = files_to_upload[file_id].file_path;

  QFutureWatcher<system_call_wrapper_error_t> *future_watcher =
          new QFutureWatcher<system_call_wrapper_error_t>(this);
  QFuture<system_call_wrapper_error_t> res =
      QtConcurrent::run(CSystemCallWrapper::copy_paste, user, ip, port, destination, file);
  future_watcher->setFuture(res);
  connect(future_watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished, [this, res, file_id] () {
      this->upload_finished(res.result(), file_id);
  });

  QTableWidgetItem *upload_status_item = ui->tableWidget->item(file_id, 4);

  qDebug() << "Is status null: " << (upload_status_item == nullptr);

  if (upload_status_item == nullptr)
    return;

  upload_status_item->setIcon(uploading);
  upload_status_item->setText(tr("Uploading"));

  // changing status
  files_to_upload[file_id].file_status = UPLOAD_WAIT;
}

////////////////////////////////////////////////////////////////////////

void DlgTransferFile::upload_finished(system_call_wrapper_error_t res, int file_id) {
  // status icons
  static QIcon uploaded(":/hub/GOOD.png");
  static QIcon failed_upload(":/hub/BAD.png");

  ui->progressBar->setValue(++ cnt_upload_files);
  if (ui->progressBar->maximum() == cnt_upload_files) {
    set_upload_status(false);
  }

  QTableWidgetItem *upload_status_item = ui->tableWidget->item(file_id, 4);

  qDebug() << "Is status null: " << (upload_status_item == nullptr);

  if (upload_status_item == nullptr)
    return;

  if (res == SCWE_SUCCESS) {
    upload_status_item->setIcon(uploaded);
    upload_status_item->setText(tr("Uploaded"));
    files_to_upload[file_id].file_status = UPLOAD_SUCCESS;
  }
  else {
    upload_status_item->setIcon(failed_upload);
    upload_status_item->setText(tr("Failed to upload"));
    files_to_upload[file_id].file_status = UPLOAD_FAIL;
  }
}

////////////////////////////////////////////////////////////////////////

void DlgTransferFile::upload_files() {
  qInfo() << "Uploading files ";

  if (files_to_upload.empty())
  {
    CNotificationObserver::Error(tr("Drag and Drop files to upload."), DlgNotification::N_NO_ACTION);
    return;
  }

  if (ui->remote_ssh_key_path->text().isEmpty())
  {
    CNotificationObserver::Error(tr("Please, add ssh-key to current environment.") , DlgNotification::N_NO_ACTION);
    return;
  }

  set_upload_status(true);

  for (int file_id = 0 ; file_id < (int) files_to_upload.size() ; file_id++) {
    if (files_to_upload[file_id].file_status != UPLOAD_SUCCESS)
      upload_start(file_id);
  }
}


DlgTransferFile::~DlgTransferFile()
{
  delete ui;
}
