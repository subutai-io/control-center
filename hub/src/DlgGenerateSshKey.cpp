#include <QMessageBox>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <RestWorker.h>
#include <QFileDialog>

#include <QStandardItem>
#include <QStandardItemModel>
#include <QListView>
#include <QModelIndexList>
#include <QItemSelectionModel>
#include <algorithm>

#include "DlgGenerateSshKey.h"
#include "ui_DlgGenerateSshKey.h"
#include "SystemCallWrapper.h"
#include "HubController.h"
#include "SettingsManager.h"
#include "NotifiactionObserver.h"

DlgGenerateSshKey::DlgGenerateSshKey(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgGenerateSshKey)
{
  ui->setupUi(this);

  //don't change order of these 2 functions :)
  environments_updated(0);
  refresh_key_files();

  connect(ui->btn_generate_new_key, SIGNAL(released()), this, SLOT(btn_generate_released()));
  connect(ui->btn_send_to_hub, SIGNAL(released()), this, SLOT(btn_send_to_hub_released()));  
  connect(&CHubController::Instance(), SIGNAL(environments_updated(int)),
          this, SLOT(environments_updated(int)));
}
////////////////////////////////////////////////////////////////////////////

DlgGenerateSshKey::~DlgGenerateSshKey() {
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::generate_new_ssh() {

  QString str_file = QFileDialog::getSaveFileName(this, "Generate new ssh key pair. Don't change directory, please",
                                                  CSettingsManager::Instance().ssh_keys_storage(),
                                                  "Ssh keys (*.pub);; All files (*.*)");
  if (str_file.isEmpty()) return;
  QFileInfo fi(str_file);

  QString str_private = CSettingsManager::Instance().ssh_keys_storage() +
                        QDir::separator() + fi.baseName();

  QString str_public = str_file + (str_private == str_file ? ".pub" : "");

  QFile key(str_private);
  QFile key_pub(str_public);

  if (key.exists() && key_pub.exists()) {
    key.remove();
    key_pub.remove();
  }

  system_call_wrapper_error_t scwe =
      CSystemCallWrapper::generate_ssh_key(CHubController::Instance().current_user(),
                                           str_private);
  if (scwe != SCWE_SUCCESS) {
    CNotificationObserver::Instance()->NotifyAboutError(
          QString("Can't generate ssh-key. Err : %1").arg(CSystemCallWrapper::scwe_error_to_str(scwe)));
    return;
  }
}
////////////////////////////////////////////////////////////////////////////


void
DlgGenerateSshKey::refresh_key_files() {
  QDir dir(CSettingsManager::Instance().ssh_keys_storage());
  if (!dir.exists()) {
    CApplicationLog::Instance()->LogError("Wrong ssh keys storage");
    return;
  }

  QStringList name_filters({"*.pub"});
  m_lst_key_files = dir.entryList(name_filters, QDir::Files | QDir::NoSymLinks);

  QStringList lst_key_content;
  for (auto i = m_lst_key_files.begin(); i != m_lst_key_files.end(); ++i) {
    QString file_path = dir.path() + QDir::separator() + *i;
    QFile key_file(file_path);
    key_file.open(QFile::ReadOnly);
    lst_key_content.push_back(QString(key_file.readAll()));
    key_file.close();
  }

  for (auto i = CHubController::Instance().lst_environments().begin();
       i != CHubController::Instance().lst_environments().end(); ++i) {
    m_dct_environment_keyflags[i->id()] =
        CRestWorker::Instance()->is_sshkeys_in_environment(lst_key_content, i->id());
  }
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::btn_generate_released() {
  QFileInfo fi(CSettingsManager::Instance().ssh_keys_storage());
  if (!fi.isDir() || !fi.isWritable()) {
    CNotificationObserver::Instance()->NotifyAboutInfo(
          "You don't have write permission to ssh-keys directory. "
          "Please add write permission or change ssh-keys storage in settings. Thanks");
    return;
  }
  generate_new_ssh();
  refresh_key_files();
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::btn_send_to_hub_released() {

  refresh_key_files();
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::environments_updated(int update_result) {
  UNUSED_ARG(update_result);
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKeyInitializer::start_initialization() {

}
////////////////////////////////////////////////////////////////////////////
