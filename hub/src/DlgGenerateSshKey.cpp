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
#include "SshKeysController.h"

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
        CSshKeysController::Instance().current_key_environments_bitmask()[r] ? Qt::Checked : Qt::Unchecked;
    item->setCheckState(st);
    item->setEditable(st != Qt::Checked);
  }
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
DlgGenerateSshKey::rebuild_keys_model() {
  m_model_keys->clear();
  for (auto i = CSshKeysController::Instance().lst_key_files().begin();
       i != CSshKeysController::Instance().lst_key_files().end(); ++i) {
    QStandardItem* item = new QStandardItem(*i);
    item->setEditable(false);
    m_model_keys->appendRow(item);
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
  CSshKeysController::Instance().generate_new_ssh_key(this);
  CSshKeysController::Instance().refresh_key_files();
  rebuild_keys_model();
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::btn_send_to_hub_released() {
  CSshKeysController::Instance().send_data_to_hub();
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::environments_updated(int update_result) {
  UNUSED_ARG(update_result);
  CSshKeysController::Instance().rebuild_bitmasks();
  rebuild_environments_model();
  set_environments_checked_flag();
}
/////////////////////////////////////////////////////// /////////////////////

void
DlgGenerateSshKey::lstv_keys_current_changed(QModelIndex ix0,
                                             QModelIndex ix1) {
  UNUSED_ARG(ix1);
  UNUSED_ARG(ix0);
  CSshKeysController::Instance().set_current_key(
        ui->lstv_sshkeys->currentIndex().data().toString());
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
  CSshKeysController::Instance().current_key_environments_bitmask()[item->index().row()] =
      item->checkState() == Qt::Checked;
  ui->btn_send_to_hub->setEnabled(CSshKeysController::Instance().something_changed());
}
////////////////////////////////////////////////////////////////////////////
