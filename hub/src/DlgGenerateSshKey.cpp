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

  m_model_disabled_keys = new QStandardItemModel(this);
  m_model_enabled_keys = new QStandardItemModel(this);
  m_model_environments = new QStandardItemModel(this);

  ui->lstv_disabled->setModel(m_model_disabled_keys);
  ui->lstv_enabled->setModel(m_model_enabled_keys);
  ui->cb_environments->setModel(m_model_environments);

  //don't change order of these 2 functions :)
  environments_updated(0);
  refresh_key_files();

  connect(ui->btn_generate_new_key, SIGNAL(released()), this, SLOT(btn_generate_released()));
  connect(ui->btn_send_to_hub, SIGNAL(released()), this, SLOT(btn_send_to_hub_released()));

  connect(ui->btn_to_disabled, SIGNAL(released()), this, SLOT(btn_to_disabled_released()));
  connect(ui->btn_to_enabled, SIGNAL(released()), this, SLOT(btn_to_enabled_released()));

  connect(ui->cb_environments, SIGNAL(currentIndexChanged(int)),
          this, SLOT(cb_environments_index_changed(int)));

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
DlgGenerateSshKey::move_items(QListView *src,
                              QListView *dst,
                              std::vector<QString>& lst_src,
                              std::vector<QString>& lst_dst) {
  QStandardItemModel* src_model = static_cast<QStandardItemModel*>(src->model());
  QStandardItemModel* dst_model = static_cast<QStandardItemModel*>(dst->model());

  QModelIndexList indexes = src->selectionModel()->selectedIndexes();
  std::sort(indexes.begin(), indexes.end());

  for(auto i = indexes.cbegin(); i != indexes.cend(); ++i) {
    dst_model->appendRow(new QStandardItem(i->data().toString()));
    lst_dst.push_back(i->data().toString());
  }

  //remove in desc order!!!
  for(auto i = indexes.crbegin(); i != indexes.crend(); ++i) {
    src_model->removeRow(i->row(), i->parent());
    lst_src.erase(lst_src.begin() + i->row());
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

  cb_environments_index_changed(ui->cb_environments->currentIndex());
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
  for (auto i = m_current_enabled.begin(); i != m_current_enabled.end(); ++i) {
    qDebug() << *i;
  }
  refresh_key_files();
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::btn_to_disabled_released() {
  move_items(ui->lstv_enabled, ui->lstv_disabled, m_current_enabled, m_current_disabled);
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::btn_to_enabled_released() {
  move_items(ui->lstv_disabled, ui->lstv_enabled, m_current_disabled, m_current_enabled);
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::cb_environments_index_changed(int ix) {
  QString env_id = CHubController::Instance().lst_environments()[ix].id();
  ix = 0;
  m_model_disabled_keys->clear();
  m_model_enabled_keys->clear();
  for (auto i = m_dct_environment_keyflags[env_id].begin();
       i != m_dct_environment_keyflags[env_id].end(); ++i, ++ix) {
    if (*i) {
      m_model_enabled_keys->appendRow(new QStandardItem(m_lst_key_files.at(ix)));
      m_current_enabled.push_back(m_lst_key_files.at(ix));
    } else {
      m_model_disabled_keys->appendRow(new QStandardItem(m_lst_key_files.at(ix)));
      m_current_disabled.push_back(m_lst_key_files.at(ix));
    }
  }
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::environments_updated(int update_result) {
  UNUSED_ARG(update_result);
  for (auto i = CHubController::Instance().lst_environments().begin();
       i != CHubController::Instance().lst_environments().end(); ++i) {
    m_model_environments->appendRow(new QStandardItem(i->name()));
  }
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKeyInitializer::start_initialization() {

}
////////////////////////////////////////////////////////////////////////////
