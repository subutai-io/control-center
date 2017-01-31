#include <QMessageBox>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <RestWorker.h>
#include <QFileDialog>
#include <QThread>

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
  ui->btn_send_to_hub->setEnabled(false);

  m_model_environments  = new QStandardItemModel(this);
  m_model_keys          = new QStandardItemModel(this);

  ui->lstv_environments->setModel(m_model_environments);
  ui->lstv_sshkeys->setModel(m_model_keys);

  connect(ui->btn_generate_new_key, SIGNAL(released()), this, SLOT(btn_generate_released()));
  connect(ui->btn_send_to_hub, SIGNAL(released()), this, SLOT(btn_send_to_hub_released()));  
  connect(&CHubController::Instance(), SIGNAL(environments_updated(int)),
          this, SLOT(environments_updated(int)));

  connect(ui->lstv_sshkeys->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
          this, SLOT(lstv_keys_current_changed(QModelIndex,QModelIndex)));

  connect(m_model_environments, SIGNAL(itemChanged(QStandardItem*)),
          this, SLOT(environments_item_changed(QStandardItem*)));

  //don't change order of these 2 functions :)
  refresh_key_files();
  rebuild_environments_model();
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
  QStringList tmp_list = dir.entryList(name_filters, QDir::Files | QDir::NoSymLinks);
  m_model_keys->clear();

  for (auto i = tmp_list.begin(); i != tmp_list.end(); ++i) {
    QString file_path = dir.path() + QDir::separator() + *i;
    QFile key_file(file_path);
    if (!key_file.open(QFile::ReadOnly)) {
      CApplicationLog::Instance()->LogError("Can't open ssh-key file : %s, reason : %s",
                                            file_path.toStdString().c_str(),
                                            key_file.errorString().toStdString().c_str());
      continue;
    }
    m_lst_key_files.push_back(*i);
    QByteArray arr_content = key_file.readAll();
//    arr_content.truncate(arr_content.size() - 1);
    m_lst_key_content.push_back(QString(arr_content));
    key_file.close();

    QStandardItem* item = new QStandardItem(*i);
    item->setEditable(false);
    m_model_keys->appendRow(item);
  }

  rebuild_bitmasks();
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::rebuild_bitmasks() {
  for (auto i = CHubController::Instance().lst_healthy_environments().begin();
       i != CHubController::Instance().lst_healthy_environments().end(); ++i) {
    m_dct_environment_keyflags[i->id()] =
        CRestWorker::Instance()->is_sshkeys_in_environment(m_lst_key_content, i->id());
  }

  for (int i = 0; i < m_lst_key_files.size(); ++i) {
    m_dct_key_environments[m_lst_key_files[i]] =
        std::vector<bool>(CHubController::Instance().lst_healthy_environments().size());

    int k = 0;
    for (auto j = m_dct_environment_keyflags.begin();
         j != m_dct_environment_keyflags.end(); ++j, ++k) {
       m_dct_key_environments[m_lst_key_files[i]][k] = j->second[i];
    }
  }

  m_dct_key_environments_original.insert(m_dct_key_environments.begin(),
                                         m_dct_key_environments.end());

  if (m_model_keys->rowCount()) {
    ui->lstv_sshkeys->selectionModel()->setCurrentIndex(
          m_model_keys->index(0, 0), QItemSelectionModel::Select);
  }
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::set_environments_checked_flag() {
  if (m_current_key.isEmpty()) return;

  for (int r = 0; r < m_model_environments->rowCount(); ++r) {
    QStandardItem* item = m_model_environments->item(r);
    Qt::CheckState st = m_dct_key_environments[m_current_key][r] ? Qt::Checked : Qt::Unchecked;
    item->setCheckState(st);
    item->setEditable(st != Qt::Checked);
  }
}
////////////////////////////////////////////////////////////////////////////

bool
DlgGenerateSshKey::something_changed() const {
  return m_dct_key_environments != m_dct_key_environments_original;
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::rebuild_environments_model() {
  m_model_environments->clear();
  for (auto i = CHubController::Instance().lst_healthy_environments().begin();
       i != CHubController::Instance().lst_healthy_environments().end(); ++i) {
    QStandardItem* nitem = new QStandardItem(i->name());
    nitem->setCheckable(true);
    nitem->setCheckState(Qt::Unchecked);
    nitem->setEditable(false);
    m_model_environments->appendRow(nitem);
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
  map_string_bitmask::iterator current = m_dct_key_environments.begin();
  map_string_bitmask::iterator original = m_dct_key_environments_original.begin();
  std::map<QString, std::vector<QString> > dct_to_send;

  for (size_t k = 0; current != m_dct_key_environments.end(); ++current, ++original, ++k) {
    if (current->second == original->second) continue;

    for (size_t i = 0; i < current->second.size(); ++i) {
      if (current->second[i] == original->second[i]) continue;
      if (!current->second[i]) continue;

      if (dct_to_send.find(m_lst_key_content[k]) == dct_to_send.end())
        dct_to_send[m_lst_key_content[k]] = std::vector<QString>();

      dct_to_send[m_lst_key_content[k]].push_back(
        CHubController::Instance().lst_healthy_environments()[i].id());
    }
  }

  QThread* st = new QThread;
  DlgGenerateSshBackgroundWorker* bw = new DlgGenerateSshBackgroundWorker(dct_to_send);

  connect(bw, SIGNAL(send_key_finished()), st, SLOT(quit()));
  connect(st, SIGNAL(started()), bw, SLOT(start_send_keys_to_hub()));
  connect(st, SIGNAL(finished()), st, SLOT(deleteLater()));
  connect(st, SIGNAL(finished()), bw, SLOT(deleteLater()));
  connect(this, SIGNAL(finished(int)), st, SLOT(quit()));
  connect(bw, SIGNAL(send_key_progress(int,int)), this, SLOT(ssh_key_send_progress(int,int)));

  bw->moveToThread(st);
  st->start();
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::environments_updated(int update_result) {
  UNUSED_ARG(update_result);
  rebuild_environments_model();
  rebuild_bitmasks();
  set_environments_checked_flag();
}
/////////////////////////////////////////////////////// /////////////////////

void
DlgGenerateSshKey::lstv_keys_current_changed(QModelIndex ix0,
                                             QModelIndex ix1) {
  UNUSED_ARG(ix1);
  UNUSED_ARG(ix0);
  m_current_key = ui->lstv_sshkeys->currentIndex().data().toString();
  set_environments_checked_flag();
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::ssh_key_send_progress(int part, int total) {
  qDebug() << part << "   " << total;
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::environments_item_changed(QStandardItem *item) {
  m_dct_key_environments[m_current_key][item->index().row()] =
      item->checkState() == Qt::Checked;
  ui->btn_send_to_hub->setEnabled(something_changed());
}
////////////////////////////////////////////////////////////////////////////

DlgGenerateSshBackgroundWorker::DlgGenerateSshBackgroundWorker(const std::map<QString, std::vector<QString> > &dct_key_environments) :
  m_dct_key_environments(dct_key_environments) {
}
////////////////////////////////////////////////////////////////////////////

DlgGenerateSshBackgroundWorker::~DlgGenerateSshBackgroundWorker() {
  /*do nothing*/
}

void
DlgGenerateSshBackgroundWorker::start_initialization() {
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshBackgroundWorker::start_send_keys_to_hub() {
  int part = 0;
  int total = (int) m_dct_key_environments.size();

  for (auto i = m_dct_key_environments.begin(); i != m_dct_key_environments.end(); ++i) {
    int http_code, err_code, network_err;
    CRestWorker::Instance()->add_sshkey_to_environments(i->first, i->second,
                                                        http_code, err_code, network_err);
    emit send_key_progress(++part, total);
  }

  emit send_key_finished();
}
////////////////////////////////////////////////////////////////////////////
