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


DlgTransferFile::DlgTransferFile(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgTransferFile){

  qInfo() <<"Open new upload dialog";

  ui->setupUi(this);

  connect(ui->more_info, &QCheckBox::toggled,
          this, &DlgTransferFile::check_more_info);


  ui->groupBox->setVisible(false);
  current_local_dir.setCurrent("/");
  current_remote_dir = "/";

  this->setMinimumWidth(this->width());
  connect(ui->btn_refresh_local, &QPushButton::clicked,
          this, &DlgTransferFile::refresh_button);
  connect(ui->btn_refresh_remote, &QPushButton::clicked,
          this, &DlgTransferFile::refresh_button);
  connect(ui->le_local, &QLineEdit::textChanged,
          this, &DlgTransferFile::refresh_button);

  connect(ui->le_local, &QLineEdit::textChanged,
          this, &DlgTransferFile::refresh_button);

  Init();
  connect(ui->btn_local_back, &QPushButton::clicked,
          this, &DlgTransferFile::local_back);
  connect(ui->btn_remote_back, &QPushButton::clicked,
          this, &DlgTransferFile::remote_back);

  connect(ui->local_file_system, &QTableWidget::doubleClicked,
          this, &DlgTransferFile::file_local_selected);

  connect(ui->remote_file_system, &QTableWidget::doubleClicked,
          this, &DlgTransferFile::file_remote_selected);

  connect(ui->btn_add_local, &QPushButton::clicked,
          this, &DlgTransferFile::file_to_upload);

  connect(ui->btn_add_remote, &QPushButton::clicked,
          this, &DlgTransferFile::file_to_download);

}
void DlgTransferFile::file_transfer_field_add_file
(const QString fileName, const QString filePath,
 quint64 fileSize, QDateTime created_date,
 FILE_OPERATION_TYPE operation_type,
 FILE_TYPE file_type) {

}

void DlgTransferFile::upload_selected() {
  // TODO
}
void DlgTransferFile::download_selected() {
  // TODO

}

void DlgTransferFile::file_to_upload() {
  // TODO
}

void DlgTransferFile::file_to_download() {
  // TODO
}

void DlgTransferFile::path_remote_changed(const QString &new_path) {
  current_remote_dir = new_path;
  refresh_remote_file_system();
}

void DlgTransferFile::path_local_changed(const QString &new_path) {
  current_local_dir.setCurrent(new_path);
  refresh_local_file_system();
}


void DlgTransferFile::file_local_selected(const QModelIndex &index) {
  QTableWidgetItem *wi = ui->local_file_system->item(index.row(), 0);
  CNotificationObserver::Instance()->Info(wi->text(), DlgNotification::N_NOTF_HISTORY);
  current_local_dir.cd(wi->text());
  refresh_local_file_system();
}

void DlgTransferFile::file_remote_selected(const QModelIndex &index) {
  QTableWidgetItem *wi = ui->remote_file_system->item(index.row(), 0);
  //CNotificationObserver::Instance()->Info(wi->text(), DlgNotification::N_NOTF_HISTORY);
  //current_local_dir.cd(wi->text());
  //if (current_remote_dir.length() >= 1)
    //current_remote_dir += "/";
  if (!current_remote_dir.endsWith("/")) {
    current_remote_dir += "/";
  }
  current_remote_dir += wi->text();
  refresh_remote_file_system();
}

void DlgTransferFile::local_back() {
  current_local_dir.cdUp();
  refresh_local_file_system();
}

void DlgTransferFile::remote_back() {
  QStringList ls = current_remote_dir.split("/");
  if (ls.size() == 1) {
    current_remote_dir = "/";
    refresh_remote_file_system();
    return;
  }


  QString new_dir = "";
  for (int i = 0 ; i < ls.size() - 1; i ++) {
    new_dir.append("/" + ls[i]);
    CNotificationObserver::Instance()->Info("hello// :  " + ls[i], DlgNotification::N_NO_ACTION);
  }

  current_local_dir = new_dir;
  refresh_remote_file_system();
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


void DlgTransferFile::add_file(QTableWidget *tw,
                               int row,
                               const std::vector<QString> &values) {
  tw->insertRow(row);
  for (int col = 0 ; col < (int)values.size() && col < 2 ; col ++) {
    QTableWidgetItem *wi = new QTableWidgetItem(values[col]);
    tw->setItem(row, col, wi);
  }
}

void DlgTransferFile::add_file_local(const QFileInfo &fi) {
  std::vector<QString> values;
  values.push_back(fi.fileName());
  values.push_back(QString::number(fi.size()));
  add_file(ui->local_file_system, ui->local_file_system->rowCount(), values);
}

void DlgTransferFile::Init() {
  QStringList file_system_header {
    "File Name",
    "File Size"
  };

  design_table_widget(ui->local_file_system, file_system_header);
  design_table_widget(ui->remote_file_system, file_system_header);

  QStringList transfer_file_field_header {
    "File Name",
    "Operation Type",
    "Size",
    "Operation Status",
  };

  design_table_widget(ui->tw_transfer_file, transfer_file_field_header);
}

void DlgTransferFile::refresh_button() {
  refresh_local_file_system();
  refresh_remote_file_system();
}

void DlgTransferFile::refresh_local_file_system() {
  ui->local_file_system->setRowCount(0);
  ui->le_local->setText(current_local_dir.path());
  for (QFileInfo fi : current_local_dir.entryInfoList()) {
    add_file_local(fi);
  }
}

void DlgTransferFile::add_file_remote(const QString file_name,
                                      const QString file_size) {
  std::vector<QString> values;
  values.push_back(file_name);
  values.push_back(file_size);
  add_file(ui->remote_file_system,
           ui->remote_file_system->rowCount(),
           values);
}

void DlgTransferFile::refresh_remote_file_system() {
  ui->remote_file_system->setRowCount(0);
  ui->le_remote->setText(current_remote_dir);

  //QStringList infoList = ssh_cmd(current_remote_dir, "cd %1; ls -l;".arg(current_remote_dir));
  QString remote_user = ui->remote_user->text();
  QString remote_port = ui->remote_port->text();
  QString remote_ip = ui->remote_ip->text();
  QString command = QString("cd %1; ls -l;").arg(current_remote_dir);

  CNotificationObserver::Instance()->Info("sdfmklsdmfk ", DlgNotification::N_NO_ACTION);
  //QString remote_
  QFutureWatcher<QStringList> *watcher =
      new QFutureWatcher<QStringList>(this);

  QFuture<QStringList> res;
  watcher->setFuture(res);
  res = QtConcurrent::run(CSystemCallWrapper::get_output_from_ssh_command,
                    remote_user, remote_ip, remote_port, command);
  connect(watcher, &QFutureWatcher<QStringList>::finished,
          [this, res](){
    QStringList output = res.result();
    for (QString ou : output) {
      //CNotificationObserver::Instance()->Info(ou, DlgNotification::N_NO_ACTION);
      QStringList splitted = ou.split(" ");
      this->add_file_remote(splitted.last(), splitted.first());
    }

  });

  //if (res == SCWE_SUCCESS)
  //  CNotificationObserver::Instance()->Info("Yea, Success", DlgNotification::N_NO_ACTION);
  //else
  //  CNotificationObserver::Instance()->Info("NO, not Success", DlgNotification::N_NO_ACTION);


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

DlgTransferFile::~DlgTransferFile()
{
  delete ui;
}
