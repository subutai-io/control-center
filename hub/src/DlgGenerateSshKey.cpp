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
#include "NotificationObserver.h"
#include "SshKeysController.h"
#include "RhController.h"

DlgGenerateSshKey::DlgGenerateSshKey(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgGenerateSshKey),
  m_change_everything_on_all_select(true)
{
  ui->setupUi(this);
  ui->btn_send_to_hub->setEnabled(false);

  m_model_environments  = new QStandardItemModel(this);
  m_model_keys          = new QStandardItemModel(this);

  ui->lstv_environments->setModel(m_model_environments);
  ui->lstv_sshkeys->setModel(m_model_keys);
  ui->pb_send_to_hub->setVisible(false);

  connect(ui->btn_generate_new_key, &QPushButton::released,
          this, &DlgGenerateSshKey::btn_generate_released);

  connect(ui->btn_send_to_hub, &QPushButton::released,
          this, &DlgGenerateSshKey::btn_send_to_hub_released); 

  connect(ui->lstv_sshkeys->selectionModel(), &QItemSelectionModel::currentChanged,
          this, &DlgGenerateSshKey::lstv_keys_current_changed);

  connect(ui->chk_select_all, &QCheckBox::stateChanged,
          this, &DlgGenerateSshKey::chk_select_all_checked_changed);

  connect(m_model_environments, &QStandardItemModel::itemChanged,
          this, &DlgGenerateSshKey::environments_item_changed);

  connect(&CSshKeysController::Instance(), &CSshKeysController::ssh_key_send_progress,
          this, &DlgGenerateSshKey::ssh_key_send_progress_sl);

  connect(&CSshKeysController::Instance(), &CSshKeysController::ssh_key_send_finished,
          this, &DlgGenerateSshKey::ssh_key_send_finished_sl);

  connect(&CSshKeysController::Instance(), &CSshKeysController::matrix_updated,
          this, &DlgGenerateSshKey::matrix_updated_slot);

  CSshKeysController::Instance().refresh_key_files();
  rebuild_keys_model();

  rebuild_environments_model();
  set_environments_checked_flag();

  if (m_model_keys->rowCount() && CSshKeysController::Instance().has_current_key()) {
    ui->lstv_sshkeys->selectionModel()->setCurrentIndex(
          m_model_keys->index(0, 0), QItemSelectionModel::Select);
  }
}
////////////////////////////////////////////////////////////////////////////

DlgGenerateSshKey::~DlgGenerateSshKey() {
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::set_environments_checked_flag() {
  for (int r = 0; r < m_model_environments->rowCount(); ++r) {
    QStandardItem* item = m_model_environments->item(r);
    Qt::CheckState st =
        CSshKeysController::Instance().get_key_environments_bit(r) ? Qt::Checked : Qt::Unchecked;
    item->setCheckState(st);
  }
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::rebuild_environments_model() {
  m_model_environments->clear();
  std::vector<CEnvironment> tmp = CSshKeysController::Instance().lst_healthy_environments();
  for (auto i : tmp) {
    QStandardItem* nitem = new QStandardItem(i.name());
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
  for (auto i : CSshKeysController::Instance().lst_key_files()) {
    QStandardItem* item = new QStandardItem(i);
    item->setEditable(false);
    m_model_keys->appendRow(item);
  }
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::btn_generate_released() {
  QFileInfo fi(CSettingsManager::Instance().ssh_keys_storage());
  if (!fi.isDir() || !fi.isWritable()) {
    CNotificationObserver::Instance()->Info(
          "You don't have write permission to ssh-keys directory. "
          "Please add write permission or change ssh-keys storage in settings. Thanks");
    return;
  }
  CSshKeysController::Instance().generate_new_ssh_key(this);
  CSshKeysController::Instance().refresh_key_files();
  rebuild_keys_model();
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::btn_send_to_hub_released() {
  ui->pb_send_to_hub->setVisible(true);
  CSshKeysController::Instance().send_data_to_hub();
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::lstv_keys_current_changed(QModelIndex ix0,
                                             QModelIndex ix1) {
  UNUSED_ARG(ix1);
  UNUSED_ARG(ix0);
  CSshKeysController::Instance().set_current_key(
        ui->lstv_sshkeys->currentIndex().data().toString());
  set_environments_checked_flag();
  ui->chk_select_all->setChecked(
        CSshKeysController::Instance().current_key_is_allselected());
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
  ui->pb_send_to_hub->setValue(ui->pb_send_to_hub->maximum());
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::chk_select_all_checked_changed(int st) {
  if (!m_change_everything_on_all_select) return;

  CSshKeysController::Instance().set_current_key_allselected(st == Qt::Checked);
  set_environments_checked_flag();
}

void
DlgGenerateSshKey::matrix_updated_slot() {
  rebuild_environments_model();
  set_environments_checked_flag();
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::environments_item_changed(QStandardItem *item) {
  CSshKeysController::Instance().set_key_environments_bit(item->index().row(),
      item->checkState() == Qt::Checked);
  ui->btn_send_to_hub->setEnabled(CSshKeysController::Instance().something_changed());
  m_change_everything_on_all_select = false;
  ui->chk_select_all->setChecked(CSshKeysController::Instance().current_key_is_allselected());
  m_change_everything_on_all_select = true;
}
////////////////////////////////////////////////////////////////////////////
