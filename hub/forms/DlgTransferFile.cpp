#include "DlgTransferFile.h"
#include "ui_DlgTransferFile.h"
#include "QFileDialog"
#include "NotificationObserver.h"
#include <QDateTime>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>


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
    this->adjustSize();});
  connect(ui->btn_upload_files, &QPushButton::clicked,
          this, &DlgTransferFile::upload_files);
  connect(ui->btn_choose_files, &QPushButton::clicked,
          this, &DlgTransferFile::select_file);
  connect(ui->btn_clear, &QPushButton::clicked,
          this, &DlgTransferFile::clear_all_files);

}
void DlgTransferFile::clear_all_files(){
  qInfo()<<"Clear files list";

  files_to_upload.clear();
  ui->tableWidget->clear();
  ui->progressBar->setValue(0);
  ui->tableWidget->setRowCount(0);
  ui->tableWidget->setHorizontalHeaderLabels(headers);
}
void DlgTransferFile::add_file(const QString &filename) {
  qInfo()<<"add file";

  QFileInfo fi(filename);
  int last_index = ui->tableWidget->rowCount();

  QTableWidgetItem *file_name = new QTableWidgetItem(fi.fileName());
  file_name->setTextAlignment(Qt::AlignHCenter);
  file_name->setTextAlignment(Qt::AlignVCenter);

  QTableWidgetItem *file_size = new QTableWidgetItem(QString::number(fi.size()/1024) + "KB");
  file_size->setTextAlignment(Qt::AlignHCenter);
  file_size->setTextAlignment(Qt::AlignVCenter);

  QDateTime created_date = fi.created();
  QTableWidgetItem *file_date = new QTableWidgetItem(created_date.date().toString());
  file_date->setTextAlignment(Qt::AlignHCenter);
  file_date->setTextAlignment(Qt::AlignVCenter);

  QTableWidgetItem *file_path = new QTableWidgetItem(fi.filePath());
  file_path->setTextAlignment(Qt::AlignHCenter);
  file_path->setTextAlignment(Qt::AlignVCenter);

  static QIcon icon_not_upload(":/hub/OK.png");
  QTableWidgetItem *upload_status = new QTableWidgetItem(icon_not_upload, "Not yet uploaded");
  upload_status->setTextAlignment(Qt::AlignHCenter);
  upload_status->setTextAlignment(Qt::AlignVCenter);

  ui->tableWidget->insertRow(last_index);

  ui->tableWidget->setItem(last_index, 0, file_name);
  ui->tableWidget->setItem(last_index, 1, file_size);
  ui->tableWidget->setItem(last_index, 2, file_date);
  ui->tableWidget->setItem(last_index, 3, file_path);
  ui->tableWidget->setItem(last_index, 4, upload_status);

  files_to_upload.push_back(fi.filePath());
}

void DlgTransferFile::select_file() {
  QString fn = QFileDialog::getOpenFileName(this, tr("File to upload"));
  if (fn == "")
    return;
  add_file(fn);
}

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

void DlgTransferFile::upload_files() {
    qInfo()<<"uploading files";
  if (files_to_upload.empty())
    return;

  ui->btn_upload_files->setEnabled(false);
  ui->btn_clear->setEnabled(false);
  ui->btn_upload_files->setText(tr("Uploading..."));
  static QIcon uploaded(":/hub/GOOD.png");
  static QIcon failed_upload(":/hub/BAD.png");
  ui->progressBar->setMaximum(files_to_upload.size());

  //ui->tableWidget->clear();
  QString ip = ui->remote_ip->text();
  QString user = ui->remote_user->text();
  QString port = ui->remote_port->text();
  QString destination = ui->remote_destination->text();

  for (int file_id = 0 ; file_id < (int) files_to_upload.size() ; file_id++) {

    QTableWidgetItem *upload_status = (ui->tableWidget->item(file_id, 4));
    QString file = files_to_upload[file_id];

    QFutureWatcher<system_call_wrapper_error_t> *future_watcher =
            new QFutureWatcher<system_call_wrapper_error_t>(this);

    QFuture<system_call_wrapper_error_t> res =
        QtConcurrent::run(CSystemCallWrapper::copy_paste, user, ip, port, destination, file);
    future_watcher->setFuture(res);


    connect(future_watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished,
            [this, res, upload_status, uploaded, failed_upload](){

                if (res.result() == SCWE_SUCCESS) {
                  upload_status->setText(tr("Uploaded"));
                  upload_status->setIcon(uploaded);
                }
                else {
                  upload_status->setText(tr("Failed to upload"));
                  upload_status->setIcon(failed_upload);
                }

                static int files_uploaded = 0;
                this->ui->progressBar->setValue(++files_uploaded);
                if (this->ui->progressBar->maximum() == files_uploaded) {
                    this->ui->btn_clear->setEnabled(true);
                    this->ui->btn_upload_files->setEnabled(true);
                    ui->btn_choose_files->setText(tr("Upload files"));
                }
    });
  }

}


DlgTransferFile::~DlgTransferFile()
{
  delete ui;
}
