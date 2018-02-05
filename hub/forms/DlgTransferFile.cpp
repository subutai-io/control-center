#include "DlgTransferFile.h"
#include "ui_DlgTransferFile.h"
#include "HubController.h"
#include "QFileDialog"

DlgTransferFile::DlgTransferFile(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgTransferFile)
{
  ui->setupUi(this);
  QStringList headers;
  headers << "File Name"
          << "Size"
          << "Date"
          << "Path"
          << "Status";

  ui->tableWidget->setHorizontalHeaderLabels(headers);
  ui->tableWidget->setColumnCount(5);


  this->setMinimumWidth(this->width());

  connect(ui->more_info, &QCheckBox::toggled, [this](bool checked){
    this->ui->groupBox->setVisible(checked);
    this->adjustSize();
  });
ui->tableWidget->verticalHeader()->setVisible(true);
ui->tableWidget->setShowGrid(false);

  connect(ui->btn_upload_files, &QPushButton::clicked,
          this, &DlgTransferFile::upload_files);
  connect(ui->btn_choose_files, &QPushButton::clicked,
          this, &DlgTransferFile::select_file);
  connect(ui->btn_clear, &QPushButton::clicked,
          this, &DlgTransferFile::clear_all_files);

}
void DlgTransferFile::clear_all_files(){
  files_to_upload.clear();
  ui->tableWidget->clear();
}
void DlgTransferFile::add_file(const QString &filename) {
  QFileInfo fi(filename);
  static int last_index = ui->tableWidget->rowCount();

  QTableWidgetItem *file_name = new QTableWidgetItem(fi.fileName());
  QTableWidgetItem *file_size = new QTableWidgetItem(QString::number(fi.size()/1024) + "KB");
  QTableWidgetItem *file_date = new QTableWidgetItem("fi.created()");
  QTableWidgetItem *file_path = new QTableWidgetItem(fi.filePath());
  static QIcon icon_not_upload(":/hub/OK.png");
  QTableWidgetItem *upload_status = new QTableWidgetItem(icon_not_upload, "Not yet uploaded");

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
#include "NotificationObserver.h"

void DlgTransferFile::upload_files() {
  static QIcon uploaded(":/hub/GOOD.png");
  static QIcon failed_upload(":/hub/BAD.png");

  ui->tableWidget->clear();
  QString ip = ui->remote_ip->text();
  QString user = ui->remote_user->text();
  QString port = ui->remote_port->text();
  QString destination = ui->remote_destination->text();
  for (int item_id = 0 ; item_id <  files_to_upload.size() ; ++item_id){
    CNotificationObserver::Instance()->Info(QString::number(ui->tableWidget->rowCount()),
                                            DlgNotification::N_NO_ACTION);

    QTableWidgetItem *upload_status = ui->tableWidget->item(item_id, 0);
    QString file = files_to_upload[item_id];
    QStringList output;


    system_call_wrapper_error_t res
        = CSystemCallWrapper::copy_paste(user, ip, port, destination, file, output);
    if (res == SCWE_SUCCESS) {
      upload_status->setIcon(uploaded);
    }
    else {
      upload_status->setIcon(failed_upload);
    }

    if (!output.empty()) {
    }
  }
}


DlgTransferFile::~DlgTransferFile()
{
  delete ui;
}
