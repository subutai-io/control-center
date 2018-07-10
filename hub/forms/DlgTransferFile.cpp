#include "DlgTransferFile.h"
#include "ui_DlgTransferFile.h"
#include "QFileDialog"
#include "NotificationObserver.h"
#include <QDateTime>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <QFileSystemModel>
#include <QTableWidgetSelectionRange>
#include <QTableWidgetItem>
#include <QMovie>

////////////////////////////////////////////////////////////////////////////////

DlgTransferFile::DlgTransferFile(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgTransferFile) {
  qInfo () <<"Open new upload dialog";
  ui->setupUi(this);
  Init();
}

////////////////////////////////////////////////////////////////////////////////
/////////////////// COMMON FUNCTIONS ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void DlgTransferFile::Init() {
  local_movie = new QMovie(":/hub/refreshing.gif");
  remote_movie = new QMovie(":/hub/refreshing.gif");


  ui->btn_upload_file->setToolTip("Upload selected files");
  ui->btn_upload_file->setIcon(QIcon(":/hub/upload-0.png"));
  ui->btn_download_file->setToolTip("Download selected files");
  ui->btn_download_file->setIcon(QIcon(":/hub/download.png"));

  ui->btn_refresh_local->setToolTip("Refresh current local directory");
  ui->btn_refresh_remote->setToolTip("Refresh current remote directory");

  ui->btn_local_back->setToolTip("Go to the parent directory");
  ui->btn_remote_back->setToolTip("Go to the parent directory");

  ui->local_file_system->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui->remote_file_system->setEditTriggers(QAbstractItemView::NoEditTriggers);

  ui->local_file_system->setDragDropMode(QAbstractItemView::DragDrop);
  ui->remote_file_system->setDragDropMode(QAbstractItemView::DragDrop);

  ui->local_file_system->setDropIndicatorShown(true);
  ui->remote_file_system->setDropIndicatorShown(true);

  ui->local_file_system->viewport()->setAcceptDrops(true);
  ui->remote_file_system->viewport()->setAcceptDrops(true);

  ui->local_file_system->setDragEnabled(true);
  ui->remote_file_system->setDragEnabled(true);

  ui->tw_transfer_file->setEditTriggers(QAbstractItemView::NoEditTriggers);

  ui->groupBox->setVisible(false);

  ui->le_local->setReadOnly(true);
  ui->le_remote->setReadOnly(true);

  QStringList stdDirList = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
  QStringList::iterator stdDir = stdDirList.begin();
  if(stdDir == stdDirList.end())
    current_local_dir.setCurrent("/");
  else
    current_local_dir.setCurrent(*stdDir);
  current_remote_dir = "/";

  this->setMinimumWidth(this->width());
  this->setMinimumHeight(this->height());

  connect(ui->more_info, &QCheckBox::toggled,
          this, &DlgTransferFile::check_more_info);

  connect(ui->btn_refresh_local, &QPushButton::clicked,
          this, &DlgTransferFile::refresh_button_local);

  connect(ui->btn_refresh_remote, &QPushButton::clicked,
          this, &DlgTransferFile::refresh_button_remote);

  connect(ui->btn_local_back, &QPushButton::clicked,
          this, &DlgTransferFile::local_back);

  connect(ui->btn_remote_back, &QPushButton::clicked,
          this, &DlgTransferFile::remote_back);

  connect(ui->local_file_system, &QTableWidget::doubleClicked,
          this, &DlgTransferFile::file_local_selected);

  connect(ui->remote_file_system, &QTableWidget::doubleClicked,
          this, &DlgTransferFile::file_remote_selected);

  connect(ui->btn_upload_file, &QPushButton::clicked,
          this, &DlgTransferFile::upload_selected);

  connect(ui->btn_download_file, &QPushButton::clicked,
          this, &DlgTransferFile::download_selected);

  connect(ui->btn_start_transfer, &QPushButton::clicked,
          this, &DlgTransferFile::start_transfer_files);

  connect(ui->btn_clear_files, &QPushButton::clicked,
          this, &DlgTransferFile::clear_files);

  connect(ui->tw_transfer_file, &DropFileTableWidget::file_was_dropped,
          this, &DlgTransferFile::file_was_dropped);

  connect(ui->local_file_system, &FileSystemTableWidget::cellPressed,
          this, &DlgTransferFile::local_cell_pressed);

  connect(ui->remote_file_system, &FileSystemTableWidget::cellPressed,
          this, &DlgTransferFile::remote_cell_pressed);

  connect(ui->local_file_system, &FileSystemTableWidget::something_is_dropped,
          this, &DlgTransferFile::local_file_system_drop);

  connect(ui->remote_file_system, &FileSystemTableWidget::something_is_dropped,
          this, &DlgTransferFile::remote_file_system_drop);


  QStringList file_system_header {
    "Name", "Size", "Modified", "File Path"
  };

  QStringList transfer_file_field_header {
    "Source File", "File Path", "Destination", "Size", "Operation Status",
  };
  design_table_widget(ui->local_file_system, file_system_header);
  design_table_widget(ui->remote_file_system, file_system_header);
  design_table_widget(ui->tw_transfer_file, transfer_file_field_header);

  refresh_local_file_system();

  ui->local_file_system->setShowGrid(true);
  ui->remote_file_system->setShowGrid(true);
}

//////////////////////////////////////////////////////////////////////////////////////////

void DlgTransferFile::local_cell_pressed(int row, int column) {
  UNUSED_ARG(row);
  UNUSED_ARG(column);
}

void DlgTransferFile::remote_cell_pressed(int row, int column) {
  UNUSED_ARG(row);
  UNUSED_ARG(column);
}

void DlgTransferFile::local_file_system_drop() {
  for (QTableWidgetSelectionRange table_range : ui->remote_file_system->selectedRanges()) {
    for (int row = table_range.topRow() ; row <= table_range.bottomRow() ; row ++) {
      FileToTransfer file_to_transfer;
      file_to_transfer.setFileInfo(remote_files[row]);
      file_to_transfer.setDesinationPath(current_local_dir.absolutePath());
      file_to_transfer.setSourceMachineType(REMOTE_MACHINE);
      file_to_transfer.setTransferFileStatus(FILE_TO_DOWNLOAD);
      file_transfer_field_add_file(file_to_transfer, true);
    }
  }
}

void DlgTransferFile::remote_file_system_drop() {
  for (QTableWidgetSelectionRange table_range : ui->local_file_system->selectedRanges()) {
    for (int row = table_range.topRow() ; row <= table_range.bottomRow() ; row ++) {
      FileToTransfer file_to_transfer;
      file_to_transfer.setFileInfo(local_files[row]);
      file_to_transfer.setDesinationPath(current_remote_dir);
      file_to_transfer.setSourceMachineType(LOCAL_MACHINE);
      file_to_transfer.setTransferFileStatus(FILE_TO_UPLOAD);
      file_transfer_field_add_file(file_to_transfer, true);
    }
  }
}


void DlgTransferFile::file_was_dropped(const QString &file_path) {
  QFileInfo fi(file_path);
  OneFile local_file(fi.fileName(),
                     fi.absoluteFilePath(),
                     fi.created(),
                     fi.size(),
                     fi.isDir() ? FILE_TYPE_DIRECTORY : FILE_TYPE_SIMPLE);
  FileToTransfer file_to_transfer;
  file_to_transfer.setFileInfo(local_file);
  file_to_transfer.setDesinationPath(current_remote_dir);
  file_to_transfer.setSourceMachineType(LOCAL_MACHINE);
  file_to_transfer.setTransferFileStatus(FILE_TO_UPLOAD);
  file_transfer_field_add_file(file_to_transfer, true);
}

void DlgTransferFile::set_buttons_enabled(bool enabled) {
  ui->btn_clear_files->setEnabled(enabled);
  ui->btn_start_transfer->setEnabled(enabled);

  ui->btn_upload_file->setEnabled(enabled);
  ui->btn_download_file->setEnabled(enabled);

  ui->btn_refresh_local->setEnabled(enabled);
  ui->btn_refresh_remote->setEnabled(enabled);

  ui->btn_local_back->setEnabled(enabled);
  ui->btn_remote_back->setEnabled(enabled);
}

void  DlgTransferFile::set_remote_button_enabled(bool enabled){
    ui->btn_refresh_remote->setEnabled(enabled);
    ui->btn_remote_back->setEnabled(enabled);
    ui->btn_download_file->setEnabled(enabled);
}

void DlgTransferFile::transfer_finished(int tw_row, system_call_wrapper_error_t res, QStringList output) {
  static QIcon transfer_finished_icon(":/hub/GOOD");
  static QIcon transfer_failed_icon(":/hub/BAD");

  FileToTransfer &file_to_transfer = files_to_transfer[tw_row];
  QTableWidgetItem *twi_operation_status = ui->tw_transfer_file->item(tw_row, 4);

  if(twi_operation_status == nullptr)
      return;

  //refresh_local_file_system();
  //refresh_remote_file_system();

  if (file_to_transfer.currentFileStatus() == FILE_TO_UPLOAD || file_to_transfer.currentFileStatus() == FIlE_FAILED_TO_UPLOAD) {
    if (res == SCWE_SUCCESS) {
      file_to_transfer.setTransferFileStatus(FILE_FINISHED_UPLOAD);
      twi_operation_status->setText("Uploaded successfully");
      twi_operation_status->setIcon(transfer_finished_icon);
      twi_operation_status->setToolTip("");
    }
    else {
      file_to_transfer.setTransferFileStatus(FIlE_FAILED_TO_UPLOAD);
      if(res == SCWE_PERMISSION_DENIED)
        twi_operation_status->setText("Permision denied");
      else
          twi_operation_status->setText("Failed to upload");
      twi_operation_status->setIcon(transfer_failed_icon);
      if(file_to_transfer.fileInfo().fileName().contains("\\"))
        twi_operation_status->setText("Invalid file name");
      twi_operation_status->setToolTip(
            output.join(",") +
            " Error Code: " + CSystemCallWrapper::scwe_error_to_str(res));
    }
  }
  else if(file_to_transfer.currentFileStatus() == FILE_TO_DOWNLOAD || file_to_transfer.currentFileStatus() == FILE_FAILED_TO_DOWNLOAD){
    if (res == SCWE_SUCCESS) {
      file_to_transfer.setTransferFileStatus(FILE_FINISHED_DOWNLOAD);
      twi_operation_status->setText("Downloaded successfully");
      twi_operation_status->setIcon(transfer_finished_icon);
      twi_operation_status->setToolTip("");
    }
    else {
      file_to_transfer.setTransferFileStatus(FILE_FAILED_TO_DOWNLOAD);
      if(res == SCWE_PERMISSION_DENIED)
          twi_operation_status->setText("Permision denied");
      else
        twi_operation_status->setText("Failed to download");
      twi_operation_status->setIcon(transfer_failed_icon);
      if(file_to_transfer.fileInfo().fileName().contains("\\"))
        twi_operation_status->setText("Invalid file name");
      twi_operation_status->setToolTip(output.join(",") +
            " Error Code: " + CSystemCallWrapper::scwe_error_to_str(res));
    }
  }
}

void DlgTransferFile::transfer_file(int tw_row) {
  if (tw_row < 0 || tw_row >= (int)files_to_transfer.size())
    return;

  FileToTransfer file_to_transfer = files_to_transfer[tw_row];
  QTableWidgetItem *twi_operation_status = ui->tw_transfer_file->item(tw_row, 4);

  static QIcon waiting_icon(":/hub/uploading.png");

  QString remote_user = ui->remote_user->text();
  QString remote_ip = ui->remote_ip->text();
  QString remote_port = ui->remote_port->text();
  QString transfer_file_path = file_to_transfer.fileInfo().filePath();
  QString destination_file_path = file_to_transfer.destinationPath();
  QString key = ui->remote_ssh_key_path->text();

  if(transfer_file_path[1] == ":"){ //correct me for windows :D
    transfer_file_path.remove(0,2);
    transfer_file_path.insert(0,QString("\\."));
  }
  if(destination_file_path[1] == ":"){ //correct me for windows :D
      destination_file_path.remove(0,2);
      destination_file_path.insert(0,QString("\\."));
  }

  twi_operation_status->setIcon(waiting_icon);

  if (file_to_transfer.currentFileStatus() == FILE_TO_UPLOAD
      || file_to_transfer.currentFileStatus() == FIlE_FAILED_TO_UPLOAD) twi_operation_status->setText("Uploading");
  else twi_operation_status->setText("Downloading");

  QFutureWatcher<system_call_wrapper_error_t> *watcher = new QFutureWatcher<system_call_wrapper_error_t>(this);
  QFuture<system_call_wrapper_error_t> res;
  watcher->setFuture(res);

  if (file_to_transfer.currentFileStatus() == FILE_TO_UPLOAD ||
      file_to_transfer.currentFileStatus() == FIlE_FAILED_TO_UPLOAD) {
    FileThreadUploader *file_thread_uploader =
        new FileThreadUploader(this);
    file_thread_uploader->init(remote_user, remote_ip, remote_port, transfer_file_path,
                               destination_file_path, key);
    file_thread_uploader->startWork();
    connect(file_thread_uploader, &FileThreadUploader::outputReceived,
            [tw_row, this](system_call_wrapper_error_t res, QStringList output){
      this->transfer_finished(tw_row, res, output);
    });
  }
  else {
    FileThreadDownloader *file_thread_uploader =
        new FileThreadDownloader(this);
    file_thread_uploader->init(remote_user, remote_ip, remote_port,
                               transfer_file_path,
                               destination_file_path, key);
    file_thread_uploader->startWork();
    connect(file_thread_uploader, &FileThreadDownloader::outputReceived,
            [tw_row, this](system_call_wrapper_error_t res, QStringList output){
      this->transfer_finished(tw_row, res, output);
    });
  }
}

void DlgTransferFile::start_transfer_files() {
  set_buttons_enabled(false);
  if (ui->tw_transfer_file->selectedRanges().isEmpty()) {
    for (int row = 0 ; row < (int)files_to_transfer.size() ; row ++) {
      if (files_to_transfer[row].currentFileStatus() == FILE_FINISHED_DOWNLOAD
          || files_to_transfer[row].currentFileStatus() == FILE_FINISHED_UPLOAD)
        continue;
      transfer_file(row);
    }
  }
  else {
    for (QTableWidgetSelectionRange table_range : ui->tw_transfer_file->selectedRanges()) {
      for (int row = table_range.bottomRow() ; row >= table_range.topRow() ; row --) {
        if (files_to_transfer[row].currentFileStatus() == FILE_FINISHED_DOWNLOAD
            || files_to_transfer[row].currentFileStatus() == FILE_FINISHED_UPLOAD)
          continue;
        if (files_to_transfer[row].currentFileStatus() == FILE_FAILED_TO_DOWNLOAD)
          files_to_transfer[row].setTransferFileStatus(FILE_TO_DOWNLOAD);
        if (files_to_transfer[row].currentFileStatus() == FIlE_FAILED_TO_UPLOAD)
          files_to_transfer[row].setTransferFileStatus(FILE_TO_UPLOAD);
        transfer_file(row);

      }
    }
  }
  set_buttons_enabled(true);
}

void DlgTransferFile::clear_files() {
  set_buttons_enabled(false);
  if (ui->tw_transfer_file->selectedRanges().isEmpty())
  {
    ui->tw_transfer_file->setRowCount(0);
    files_to_transfer.clear();
  }
  else{
    for (QTableWidgetSelectionRange table_range : ui->tw_transfer_file->selectedRanges()) {
      for (int row = table_range.bottomRow() ; row >= table_range.topRow() ; row --) {
        ui->tw_transfer_file->removeRow(row);
        files_to_transfer.erase(files_to_transfer.begin() + row);
      }
    }
  }
  set_buttons_enabled(true);
}

QString DlgTransferFile::parseDate(const QString &month, const QString &day, const QString &year_or_time) {
  QString result;

  static std::map <QString, QString> month_converter {
    {"Jan", "01"}, {"Feb", "02"}, {"Mar", "03"}, {"Apr", "04"}, {"May", "05"}, {"Jun", "06"},
    {"Jul", "07"}, {"Aug", "08"}, {"Sep", "09"}, {"Oct", "10"}, {"Nov", "11"}, {"Dec", "12"},
  };
  if(month_converter.find(month) == month_converter.end()) result.append("01");
  else result.append(month_converter[month]);
  result.append("/");

  result.append(day);
  result.append("/");

  if (year_or_time.contains(":")) result.append(QDate::currentDate().toString("yyyy"));
  else result.append(year_or_time);
  return result;
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
  refresh_remote_file_system();
}

///////////////////////////////////////////////////////////////////////////////////////////

void DlgTransferFile::file_transfer_field_add_file(const FileToTransfer &file, bool instant_transfer) {
  static QIcon ok_icon(":/hub/OK.png");
  static QIcon download_icon(":/hub/download");
  static QIcon upload_icon(":/hub/upload");


  int current_row = ui->tw_transfer_file->rowCount();


  ui->tw_transfer_file->insertRow(current_row);
  QTableWidgetItem *wi_source_file;
  if (file.currentFileStatus() == FILE_TO_UPLOAD) wi_source_file = new QTableWidgetItem(upload_icon ,file.fileInfo().fileName());
  else wi_source_file = new QTableWidgetItem(download_icon ,file.fileInfo().fileName());
  QTableWidgetItem *wi_file_path = new QTableWidgetItem(file.fileInfo().filePath());
  QTableWidgetItem *wi_destination_directory = new QTableWidgetItem(file.destinationPath());
  QTableWidgetItem *wi_file_size = new QTableWidgetItem(QString::number(file.fileInfo().fileSize()));
  QTableWidgetItem *wi_file_status;
  if (file.currentFileStatus() == FILE_TO_UPLOAD) wi_file_status = new QTableWidgetItem(ok_icon,"Not yet Uploaded");
  else wi_file_status = new QTableWidgetItem(ok_icon, "Not yet downloaded");

  ui->tw_transfer_file->setItem(current_row, 0, wi_source_file);
  ui->tw_transfer_file->setItem(current_row, 1, wi_file_path);
  ui->tw_transfer_file->setItem(current_row, 2, wi_destination_directory);
  ui->tw_transfer_file->setItem(current_row, 3, wi_file_size);
  ui->tw_transfer_file->setItem(current_row, 4, wi_file_status);

  files_to_transfer.push_back(file);
  if (instant_transfer)
    transfer_file(current_row);
}

void DlgTransferFile::check_more_info(bool checked) {
  ui->groupBox->setVisible(checked);
  this->adjustSize();
}

void DlgTransferFile::design_table_widget(QTableWidget *tw,
                                          const QStringList &headers) {
  tw->setColumnCount(headers.size());
  tw->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  tw->verticalHeader()->setVisible(true);
  tw->setHorizontalHeaderLabels(headers);
  tw->setShowGrid(false);
  tw->setSelectionBehavior(QAbstractItemView::SelectRows);
}


void DlgTransferFile::add_file_to_file_system_tw(QTableWidget *file_system_tw, int row, const OneFile &file) {
  static QIcon file_icon(":/hub/file_icon.png");
  static QIcon directory_icon(":/hub/directory_icon.png");

  file_system_tw->insertRow(row);
  QTableWidgetItem *wi_file_name;
  QTableWidgetItem *wi_file_size = new QTableWidgetItem(QString::number(file.fileSize()));
  QTableWidgetItem *wi_file_modified = new QTableWidgetItem(file.created().toString("MM/dd/yyyy"));
  QTableWidgetItem *wi_file_path = new QTableWidgetItem(file.filePath());
  if (file.fileType() == FILE_TYPE_DIRECTORY) wi_file_name = new QTableWidgetItem(directory_icon, file.fileName());
  else wi_file_name = new QTableWidgetItem(file_icon, file.fileName());
  file_system_tw->setItem(row, 0, wi_file_name);
  file_system_tw->setItem(row, 1, wi_file_size);
  file_system_tw->setItem(row, 2, wi_file_modified);
  file_system_tw->setItem(row, 3, wi_file_path);
}


////////////////////////////////////////////////////////////////////////////////

void DlgTransferFile::upload_selected() {
  set_buttons_enabled(false);
  for (QTableWidgetSelectionRange table_range : ui->local_file_system->selectedRanges()) {
    for (int row = table_range.topRow() ; row <= table_range.bottomRow() ; row ++) {
      FileToTransfer file_to_transfer;
      file_to_transfer.setFileInfo(local_files[row]);
      file_to_transfer.setDesinationPath(current_remote_dir);
      file_to_transfer.setSourceMachineType(LOCAL_MACHINE);
      file_to_transfer.setTransferFileStatus(FILE_TO_UPLOAD);
      file_transfer_field_add_file(file_to_transfer, true);
    }
  }
  set_buttons_enabled(true);
}

void DlgTransferFile::download_selected() {
  set_buttons_enabled(false);
  for (QTableWidgetSelectionRange table_range : ui->remote_file_system->selectedRanges()) {
    for (int row = table_range.topRow() ; row <= table_range.bottomRow() ; row ++) {
      FileToTransfer file_to_transfer;
      file_to_transfer.setFileInfo(remote_files[row]);
      file_to_transfer.setDesinationPath(current_local_dir.absolutePath());
      file_to_transfer.setSourceMachineType(REMOTE_MACHINE);
      file_to_transfer.setTransferFileStatus(FILE_TO_DOWNLOAD);
      file_transfer_field_add_file(file_to_transfer, true);
    }
  }
  set_buttons_enabled(true);
}

////////////////////////////////////////////////////////////////////////////////

void DlgTransferFile::file_to_upload() {
  set_buttons_enabled(false);
  for (QTableWidgetSelectionRange table_range : ui->local_file_system->selectedRanges()) {
    for (int row = table_range.topRow() ; row <= table_range.bottomRow() ; row ++) {
      FileToTransfer file_to_transfer;
      file_to_transfer.setFileInfo(local_files[row]);
      file_to_transfer.setDesinationPath(current_remote_dir);
      file_to_transfer.setSourceMachineType(LOCAL_MACHINE);
      file_to_transfer.setTransferFileStatus(FILE_TO_UPLOAD);
      file_transfer_field_add_file(file_to_transfer, false);

    }
  }
  set_buttons_enabled(true);
}

void DlgTransferFile::file_to_download() {
  set_buttons_enabled(false);
  for (QTableWidgetSelectionRange table_range : ui->remote_file_system->selectedRanges()) {
    for (int row = table_range.topRow() ; row <= table_range.bottomRow() ; row ++) {
      FileToTransfer file_to_transfer;
      file_to_transfer.setFileInfo(remote_files[row]);
      file_to_transfer.setDesinationPath(current_local_dir.absolutePath());
      file_to_transfer.setSourceMachineType(REMOTE_MACHINE);
      file_to_transfer.setTransferFileStatus(FILE_TO_DOWNLOAD);
      file_transfer_field_add_file(file_to_transfer, false);
    }
  }
  set_buttons_enabled(true);
}

////////////////////////////////////////////////////////////////////////////////

void DlgTransferFile::path_remote_changed(const QString &new_path) {
  current_remote_dir = new_path;
  refresh_remote_file_system();
}

void DlgTransferFile::path_local_changed(const QString &new_path) {
  current_local_dir.setCurrent(new_path);
  refresh_local_file_system();
}

////////////////////////////////////////////////////////////////////////////////

void DlgTransferFile::file_local_selected(const QModelIndex &index) {
  if (index.row() < 0 || index.row() >= (int)local_files.size())
    return;
  OneFile file_selected = local_files[index.row()];
  if(file_selected.fileType() == FILE_TYPE_DIRECTORY) {
    current_local_dir.cd(file_selected.fileName());
    refresh_local_file_system();
  }
  else {
    FileToTransfer file_to_transfer;
    file_to_transfer.setFileInfo(file_selected);
    file_to_transfer.setDesinationPath(current_remote_dir);
    file_to_transfer.setSourceMachineType(LOCAL_MACHINE);
    file_to_transfer.setTransferFileStatus(FILE_TO_UPLOAD);
    file_transfer_field_add_file(file_to_transfer, false);
  }
}

void DlgTransferFile::file_remote_selected(const QModelIndex &index) {
  if (index.row() < 0 || index.row() >= (int)remote_files.size())
    return;

  OneFile file_selected = remote_files[index.row()];
  if(file_selected.fileType() == FILE_TYPE_DIRECTORY) {
    current_remote_dir += file_selected.fileName();
    refresh_remote_file_system();
  }
  else {
    FileToTransfer file_to_transfer;
    file_to_transfer.setFileInfo(file_selected);
    file_to_transfer.setDesinationPath(current_local_dir.absolutePath());
    file_to_transfer.setSourceMachineType(REMOTE_MACHINE);
    file_to_transfer.setTransferFileStatus(FILE_TO_DOWNLOAD);
    file_transfer_field_add_file(file_to_transfer, false);
  }
}

////////////////////////////////////////////////////////////////////////////////

void DlgTransferFile::local_back() {
  if (current_local_dir.absolutePath() == "/")
    return;
  current_local_dir.cdUp();
  refresh_local_file_system();
}

void DlgTransferFile::remote_back() {
  set_remote_button_enabled(false);
  QStringList pwd = current_remote_dir.split("/");
  QString new_dir = "";
  // EXAMPLE: current_remote_dir= '/usr/bin/'
  // sl = ('','usr','bin', ''), need to get rid of first and last empty strings, and last  bin directory to go back
  for (int i = 1 ; i < pwd.size() - 2; i ++){
    new_dir.append("/" + pwd[i]);
  }
  new_dir.append("/");
  current_remote_dir = new_dir;
  refresh_remote_file_system();
}

////////////////////////////////////////////////////////////////////////////////

void DlgTransferFile::add_file_local(const QFileInfo &fi) {
  if (fi.fileName() == "." || fi.fileName() == "..")
    return;

  OneFile local_file(fi.fileName(),
                     fi.absoluteFilePath(),
                     fi.created(),
                     fi.size(),
                     fi.isDir() ? FILE_TYPE_DIRECTORY : FILE_TYPE_SIMPLE);

  add_file_to_file_system_tw(ui->local_file_system,
                             ui->local_file_system->rowCount(),
                             local_file);
  local_files.push_back(local_file);
}

void DlgTransferFile::add_file_remote(const QString &file_info) {
  // Parsing the string
  QStringList splitted;
  DlgTransferFile::parse_remote_file(file_info, splitted);
  if (splitted.size() < 9)
    return;


  QString file_types = splitted[0];
  QString file_count = splitted[1];
  QString file_owner = splitted[2];
  QString file_group = splitted[3];
  QString file_size = splitted[4];
  QString file_month = splitted[5];
  QString file_day = splitted[6];
  QString file_year_or_time = splitted[7];
  QString file_name = splitted[8];
  QString file_path = current_remote_dir + file_name;

  if(file_path[file_path.size()-1] == '*')
    file_path.chop(1); // remove last character, which is `*`

  QDateTime created = QDateTime::fromString(parseDate(file_month, file_day, file_year_or_time),
                                            "MM/d/yyyy");

  OneFile remote_file(file_name,
                      file_path,
                      created,
                      file_size.toInt(),
                      file_name.endsWith("/") ?
                      FILE_TYPE_DIRECTORY : FILE_TYPE_SIMPLE);

  add_file_to_file_system_tw(
           ui->remote_file_system,
           ui->remote_file_system->rowCount(),
           remote_file);

  remote_files.push_back(remote_file);
}

////////////////////////////////////////////////////////////////////////////////

void DlgTransferFile::refresh_button_local() {
  set_buttons_enabled(false);
  refresh_local_file_system();
  set_buttons_enabled(true);
}

void DlgTransferFile::refresh_button_remote() {
  refresh_remote_file_system();
}

////////////////////////////////////////////////////////////////////////////////

void DlgTransferFile::refresh_local_file_system() {
  ui->lbl_local_files->setMovie(remote_movie);
  remote_movie->start();

  ui->local_file_system->setRowCount(0);
  current_local_dir.refresh();
  ui->le_local->setText(current_local_dir.absolutePath());
  local_files.clear();
  for (QFileInfo fi : current_local_dir.entryInfoList()) {
    add_file_local(fi);
  }
  remote_movie->stop();
  ui->lbl_local_files->setText("Local");
}

void DlgTransferFile::refresh_remote_file_system() {

  qDebug()
          << "Refresh remote file system"
          << current_remote_dir;

  set_remote_button_enabled(false);
  ui->lbl_remote_files->setMovie(remote_movie);
  remote_movie->start();

  ui->remote_file_system->setRowCount(0);
  ui->le_remote->setText(current_remote_dir);
  remote_files.clear();

  QString remote_user = ui->remote_user->text();
  QString remote_port = ui->remote_port->text();
  QString remote_ip = ui->remote_ip->text();
  QString remote_key = ui->remote_ssh_key_path->text();
  QString command = QString("cd \"%1\"; ls -lF;").arg(current_remote_dir);
  RemoteCommandExecutor *command_executor = new RemoteCommandExecutor(this);
  command_executor->init(remote_user, remote_ip,remote_port, command, remote_key);
  command_executor->startWork();
  connect(command_executor, &RemoteCommandExecutor::outputReceived,
          this, &DlgTransferFile::output_from_remote_command);
}

void DlgTransferFile::output_from_remote_command(system_call_wrapper_error_t res, const QStringList &output) {
  for (QString file_info : output) {
    qDebug()
            << "files from remote: "
            << file_info;
    add_file_remote(file_info);
  }
  remote_movie->stop();
  set_remote_button_enabled(true);
  if (res == SCWE_SUCCESS) {
    ui->lbl_remote_files->setStyleSheet("");
    ui->lbl_remote_files->setText("Remote");
  }
  else {
    ui->lbl_remote_files->setStyleSheet(" QLabel {color : red;} ");
    ui->lbl_remote_files->setText("Failed to refresh remote directory.");
  }
}
////////////////////////////////////////////////////////////////////////////////////////

void DlgTransferFile::parse_remote_file(const QString &file_info, QStringList &splitted){
    int counter = 0;
    bool reading_file = false;
    bool reading_file_name = false;
    int number_strings_parsed = 0;
    QString st = "";
    while(counter != file_info.size()){
        if(reading_file_name == true){
            st+=file_info[counter];
        }
        else {
            if(reading_file == true){
                if(file_info[counter] == ' '){
                    reading_file = false;
                    splitted.push_back(st);
                    st="";
                    number_strings_parsed++;
                }
                else st+=file_info[counter];
            }
            else{
                if(file_info[counter] != ' '){
                    reading_file = true;
                    st+=file_info[counter];
                    if (number_strings_parsed == 8)
                        reading_file_name = true;
                }
            }
        }
        counter++;
    }
    if(reading_file == true) splitted.push_back(st);
}
////////////////////////////////////////////////////////////////////////////////////////

DlgTransferFile::~DlgTransferFile()
{
  delete ui;
}
