#include "DlgTransferFile.h"
#include "ui_DlgTransferFile.h"
#include "HubController.h"
#include "QFileDialog"

DlgTransferFile::DlgTransferFile(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgTransferFile)
{
  ui->setupUi(this);

  connect(ui->btn_upload_files, &QPushButton::clicked,
          this, &DlgTransferFile::upload_files);
 connect(ui->btn_choose_files, &QPushButton::clicked,
          this, &DlgTransferFile::select_file);

}
void DlgTransferFile::select_file(){
  QString fn = QFileDialog::getOpenFileName(this, tr("File to upload"));
  if (fn == "")
    return;
  ui->listWidget->addItem(fn);
}

void DlgTransferFile::addSSHKey(const QString &key) {

}
void DlgTransferFile::addIPPort(const QString &ip, const QString &port) {
  ui->remote_ip->setText(ip);
  ui->remote_port->setText(port);
}

void DlgTransferFile::addUser(const QString &user) {
  ui->remote_user->setText(user);
}



//void DlgTransferFile::addRemoteHost() {
// ui->remote_user->setText(CSettingsManager::Instance().ssh_user());
//
//}
//
//void DlgTransferFile::addContainer(const CEnvironment *env, const CHubContainer *cont) {
//  ui->remote_ip->setText(cont->rh_ip());
//  ui->remote_port->setText(cont->port());
//  ui->remote_ssh_key_path->setText();
//}

void DlgTransferFile::upload_files() {
  QListWidgetItem *file_item = ui->listWidget->item(0);

  QString ip = ui->remote_ip->text();
  QString user = ui->remote_user->text();
  QString port = ui->remote_port->text();
  QString destination = ui->remote_destination->text();
  QString file = file_item->text();

  CSystemCallWrapper::copy_paste(user, ip, port, destination, file);

}


DlgTransferFile::~DlgTransferFile()
{
  delete ui;
}
