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
#include <QtConcurrent/QtConcurrent>

#include "DlgGenerateSshKey.h"
#include "ui_DlgGenerateSshKey.h"
#include "SystemCallWrapper.h"
#include "HubController.h"
#include "SettingsManager.h"
#include "NotificationObserver.h"
#include "SshKeysController.h"
#include "SshKeyController.h"
#include "RhController.h"

DlgGenerateSshKey::DlgGenerateSshKey(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgGenerateSshKey),
  m_change_everything_on_all_select(true)
{
  ui->setupUi(this);
  //ui->btn_send_to_hub->setEnabled(false);

  m_model_environments  = new QStandardItemModel(this);
  m_model_keys          = new QStandardItemModel(this);

  // Current ssh key selected index
  m_current_key_index = 0;

  ui->lstv_environments->setModel(m_model_environments);
  ui->lstv_sshkeys->setModel(m_model_keys);
  ui->pb_send_to_hub->setVisible(false);

  connect(ui->btn_generate_new_key, &QPushButton::released,
          this, &DlgGenerateSshKey::btn_generate_released);

  connect(&SshKeyController::Instance(), &SshKeyController::finished_check_environment_keys,
          this, &DlgGenerateSshKey::keys_updated_slot);

  connect(ui->lstv_sshkeys->selectionModel(), &QItemSelectionModel::currentChanged,
          this, &DlgGenerateSshKey::lstv_keys_current_changed);

  connect(ui->chk_select_all, &QCheckBox::clicked,
          this, &DlgGenerateSshKey::chk_select_all_checked_changed);

  connect(ui->btn_remove_key, &QPushButton::released,
            this, &DlgGenerateSshKey::btn_remove_released);

  connect(&SshKeyController::Instance(), &SshKeyController::progress_ssh_key_rest,
          this, &DlgGenerateSshKey::ssh_key_send_progress_sl);

  connect(m_model_environments, &QStandardItemModel::itemChanged,
          this, &DlgGenerateSshKey::environments_item_changed);

  connect(ui->btn_send_to_hub, &QPushButton::released,
          this, &DlgGenerateSshKey::btn_send_to_hub_released);

  connect(&SshKeyController::Instance(), &SshKeyController::ssh_key_send_finished,
          this, &DlgGenerateSshKey::ssh_key_send_finished_sl);

  rebuild_keys_model();
  rebuild_environments_model();
  set_environments_checked_flag();
}
////////////////////////////////////////////////////////////////////////////

DlgGenerateSshKey::~DlgGenerateSshKey() {
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::set_environments_checked_flag() {
  int count = 0;
  bool tmp;
  QString env_id; // get env id from environment model
  QStringList env_ids; // get env ids from key model

  if (m_model_keys->rowCount() == 0)
    return;

  for (int r = 0; r < m_model_environments->rowCount(); ++r) {
    QStandardItem* item = m_model_environments->item(r);
    Qt::CheckState st;
    QVariant env_data = item->data(Qt::UserRole + 1);
    env_id = ""; // empty

    if (!env_data.isNull())
      env_id = env_data.toString();

    if (m_all_state_envs[m_current_key_index].states.find(env_id) != m_all_state_envs[m_current_key_index].states.end()) {
      st = m_all_state_envs[m_current_key_index].states[env_id] ?
            Qt::Checked : Qt::Unchecked;
    } else {
      QStandardItem* key = m_model_keys->item(m_current_key_index);
      QVariant key_data = key->data(Qt::UserRole + 1);

      if (!key_data.isNull())
        env_ids = key_data.toStringList();

      tmp = env_ids.contains(env_id);
      st = tmp ? Qt::Checked : Qt::Unchecked;
    }

    item->setCheckState(st);

    if (st == Qt::Checked)
      count++;
  }

  if (count == m_model_environments->rowCount())
    ui->chk_select_all->setCheckState(Qt::Checked);
  else
    ui->chk_select_all->setCheckState(Qt::Unchecked);
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::rebuild_environments_model() {
  m_model_environments->clear();
  std::map<QString, QString> tmp = SshKeyController::Instance().list_healthy_envs();
  for (auto i : tmp) {
    QStandardItem* nitem = new QStandardItem(i.second);
    nitem->setData(QVariant::fromValue(i.first));
    nitem->setCheckable(true);
    nitem->setCheckState(Qt::Unchecked);
    nitem->setEditable(false);
    m_model_environments->appendRow(nitem);
  }
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::rebuild_keys_model() {
  m_model_keys->clear();
  for (auto i : SshKeyController::Instance().list_keys()) {
    // set environment ids to ssh key model
    QVariant v(i.env_ids);
    QStandardItem* item = new QStandardItem(i.file_name);
    item->setData(v);
    item->setEditable(false);
    m_model_keys->appendRow(item);
  }

  if (m_model_keys->rowCount()) {
    if (m_current_key_index == 0)
      ui->lstv_sshkeys->selectionModel()->setCurrentIndex(
           m_model_keys->index(0, 0), QItemSelectionModel::Select);
    else
      ui->lstv_sshkeys->selectionModel()->setCurrentIndex(
           m_model_keys->index(m_current_key_index, 0), QItemSelectionModel::Select);
  }
}
////////////////////////////////////////////////////////////////////////////

void DlgGenerateSshKey::btn_generate_released() {
  QFileInfo fi(CSettingsManager::Instance().ssh_keys_storage());
  if (!fi.isDir() || !fi.isWritable()) {
    CNotificationObserver::Instance()->Info(
          tr("You don't have administrator privileges to write into SSH directory. "
          "Please add rights or change SSH-keys storage in settings."), DlgNotification::N_SETTINGS);
    return;
  }
  SshKeyController::Instance().generate_keys(this);
  SshKeyController::Instance().refresh_key_files();
}

////////////////////////////////////////////////////////////////////////////

void DlgGenerateSshKey::btn_remove_released() {
  if (m_model_keys->rowCount() > 0) {
    QMessageBox *msg = new QMessageBox(
          QMessageBox::Information, "Attention!", "",
          QMessageBox::Yes | QMessageBox::No);
    msg->setTextFormat(Qt::RichText);

    QStandardItem *key = m_model_keys->item(m_current_key_index);
    QString file_name = key->text();
    QStringList env_ids = key->data().toStringList();
    QString tmp;
    std::map<QString, QString> lst_envs =
        SshKeyController::Instance().list_healthy_envs();

    if (env_ids.size() > 0 && (lst_envs.size() == static_cast<size_t>(m_model_environments->rowCount()))) {
      for (auto i : env_ids)
        tmp += "<i>" + lst_envs[i] + "</i><br/>";

      msg->setText(QString("You are going to remove an SSH-key <i>%1</i>. "
                           "Do you want to proceed?").arg(file_name));
      msg->setDetailedText(QString("This SSH-key is deployed to this environments:<br>%1"
                                   "Your SSH-key will be "
                                   "removed from all of the environments.<br>").arg(tmp));
    } else {
      msg->setText(QString("You are going to remove an SSH-key <i>%1</i>. This "
                           "SSH-key is not deployed to the environments.<br> "
                           "Do you want to proceed?").arg(file_name));
    }

    connect(msg, &QMessageBox::finished, msg, &QMessageBox::deleteLater);
    if (msg->exec() != QMessageBox::Yes) {
      return;
    }
    ui->pb_send_to_hub->setVisible(true);
    //SshKeyController::Instance().remove_key(file_name);
    QtConcurrent::run(&SshKeyController::Instance(),
                      &SshKeyController::remove_key, file_name);
  }
}

////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::btn_send_to_hub_released() {
  ui->btn_send_to_hub->setEnabled(false);
  ui->pb_send_to_hub->setVisible(true);             
  ui->pb_send_to_hub->setValue(1);
  //SshKeyController::Instance().upload_key(m_all_state_envs);
  QtConcurrent::run(&SshKeyController::Instance(),
                    &SshKeyController::upload_key, m_all_state_envs);
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::lstv_keys_current_changed(QModelIndex current,
                                             QModelIndex previous) {
  UNUSED_ARG(previous);

  m_current_key_index = current.row();
  set_environments_checked_flag();
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::ssh_key_send_progress_sl(int part, int total) {
  ui->pb_send_to_hub->setMaximum(total);
  ui->pb_send_to_hub->setValue(part);
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::ssh_key_send_finished_sl() {
  ui->btn_send_to_hub->setEnabled(true);
  ui->pb_send_to_hub->setMaximum(100);
  ui->pb_send_to_hub->setValue(ui->pb_send_to_hub->maximum());
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::chk_select_all_checked_changed(bool is_checked) {
  for (int r = 0; r < m_model_environments->rowCount(); ++r) {
    QStandardItem* item = m_model_environments->item(r);
    item->setCheckState(is_checked ? Qt::Checked : Qt::Unchecked);
  }
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::keys_updated_slot() {
  // if ssh key list change,
  // clean "m_all_state_envs" and set "m_current_key_index" 0
  if (SshKeyController::Instance().list_keys().size()
      != static_cast<size_t>(m_model_keys->rowCount())) {
    m_all_state_envs.clear();
    m_current_key_index = 0;
  }

  rebuild_keys_model();
  rebuild_environments_model();
  set_environments_checked_flag();
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::environments_item_changed(QStandardItem *item) {
  if (m_model_keys->rowCount() == 0)
    return;

  QString env_id;
  env_id = item->data(Qt::UserRole+1).toString();

  m_all_state_envs[m_current_key_index].states[env_id] =
      (item->checkState() == Qt::Checked);
}
////////////////////////////////////////////////////////////////////////////
