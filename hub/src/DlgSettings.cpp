#include <QToolTip>
#include <QFileDialog>
#include <QStandardPaths>

#include <QStandardItemModel>
#include <QStandardItem>
#include <QListView>

#include "DlgSettings.h"
#include "ui_DlgSettings.h"
#include "SettingsManager.h"
#include "SystemCallWrapper.h"
#include "Commons.h"
#include "RhController.h"
#include "ApplicationLog.h"

static void fill_freq_combobox(QComboBox* cb) {
  for (int i = 0; i < CSettingsManager::UF_LAST; ++i)
    cb->addItem(CSettingsManager::update_freq_to_str((CSettingsManager::update_freq_t) i));
}
////////////////////////////////////////////////////////////////////////////

DlgSettings::DlgSettings(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgSettings),
  m_tab_resize_filter(nullptr),
  m_model_resource_hosts(nullptr),
  m_refresh_rh_list_progress_val(0)
{
  ui->setupUi(this);
  ui->sb_refresh_timeout->setValue(CSettingsManager::Instance().refresh_time_sec());
  ui->le_p2p_command->setText(CSettingsManager::Instance().p2p_path());
  ui->sb_notification_delay->setMinimum(CSettingsManager::NOTIFICATION_DELAY_MIN);
  ui->sb_notification_delay->setMaximum(CSettingsManager::NOTIFICATION_DELAY_MAX);
  ui->sb_notification_delay->setValue(CSettingsManager::Instance().notification_delay_sec());
  ui->le_ssh_command->setText(CSettingsManager::Instance().ssh_path());
  ui->le_ssh_user->setText(CSettingsManager::Instance().ssh_user());
  ui->le_rhip_host->setText(CSettingsManager::Instance().rh_host());
  ui->le_rhip_password->setText(CSettingsManager::Instance().rh_pass());
  ui->le_rhip_port->setText(QString("%1").arg(CSettingsManager::Instance().rh_port()));
  ui->le_rhip_user->setText(CSettingsManager::Instance().rh_user());
  ui->le_logs_storage->setText(CSettingsManager::Instance().logs_storage());
  ui->le_ssh_keys_storage->setText(CSettingsManager::Instance().ssh_keys_storage());

  ui->le_rtm_db_folder->setText(CSettingsManager::Instance().rtm_db_dir());
  ui->le_rtm_db_folder->setVisible(false);
  ui->btn_rtm_db_folder->setVisible(false);
  ui->lbl_rtm_db_folder->setVisible(false);

  fill_freq_combobox(ui->cb_p2p_frequency);
  fill_freq_combobox(ui->cb_rh_frequency);
  fill_freq_combobox(ui->cb_tray_frequency);
  fill_freq_combobox(ui->cb_rhm_frequency);

  ui->cb_p2p_frequency->setCurrentIndex(CSettingsManager::Instance().p2p_update_freq());
  ui->cb_rh_frequency->setCurrentIndex(CSettingsManager::Instance().rh_update_freq());
  ui->cb_tray_frequency->setCurrentIndex(CSettingsManager::Instance().tray_update_freq());
  ui->cb_rhm_frequency->setCurrentIndex(CSettingsManager::Instance().rh_management_update_freq());

  ui->chk_p2p_autoupdate->setChecked(CSettingsManager::Instance().p2p_autoupdate());
  ui->chk_rh_autoupdate->setChecked(CSettingsManager::Instance().rh_autoupdate());
  ui->chk_tray_autoupdate->setChecked(CSettingsManager::Instance().tray_autoupdate());
  ui->chk_rhm_autoupdate->setCheckable(CSettingsManager::Instance().rh_management_autoupdate());

  m_tab_resize_filter = new TabResizeFilter(ui->tabWidget);
  ui->tabWidget->installEventFilter(m_tab_resize_filter);

  ui->le_terminal_cmd->setText(CSettingsManager::Instance().terminal_cmd());
  ui->le_terminal_arg->setText(CSettingsManager::Instance().terminal_arg());

#ifdef RT_OS_DARWIN
  ui->gb_terminal_settings->setVisible(false);
#endif

  m_model_resource_hosts = new QStandardItemModel(this);
  ui->lstv_resource_hosts->setModel(m_model_resource_hosts);

  ui->pb_refresh_rh_list->setMinimum(0);
  ui->pb_refresh_rh_list->setMaximum(CRhController::REFRESH_DELAY_SEC);
  ui->pb_refresh_rh_list->setValue(m_refresh_rh_list_progress_val);
  m_refresh_rh_list_timer.setInterval(1000);

  rebuild_rh_list_model();

  connect(ui->btn_ok, SIGNAL(released()), this, SLOT(btn_ok_released()));
  connect(ui->btn_cancel, SIGNAL(released()), this, SLOT(btn_cancel_released()));
  connect(ui->btn_p2p_file_dialog, SIGNAL(released()),
          this, SLOT(btn_p2p_file_dialog_released()));
  connect(ui->btn_ssh_command, SIGNAL(released()),
          this, SLOT(btn_ssh_command_released()));
  connect(ui->btn_logs_storage, SIGNAL(released()),
          this, SLOT(btn_logs_storage_released()));
  connect(ui->btn_ssh_keys_storage, SIGNAL(released()),
          this, SLOT(btn_ssh_keys_storage_released()));  
  connect(ui->btn_refresh_rh_list, SIGNAL(released()),
          this, SLOT(btn_refresh_rh_list_released()));
  connect(ui->lstv_resource_hosts, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(lstv_resource_hosts_double_clicked(QModelIndex)));
  connect(CRhController::Instance(), SIGNAL(resource_host_list_updated(bool)),
          this, SLOT(resource_host_list_updated_sl(bool)));
  connect(&m_refresh_rh_list_timer, SIGNAL(timeout()),
          this, SLOT(refresh_rh_list_timer_timeout()));
}

DlgSettings::~DlgSettings()
{
  if (m_tab_resize_filter) delete m_tab_resize_filter;
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

void
DlgSettings::rebuild_rh_list_model() {
  m_model_resource_hosts->clear();
  for (auto i = CRhController::Instance()->dct_resource_hosts().begin();
       i != CRhController::Instance()->dct_resource_hosts().end(); ++i) {
    QStandardItem* item = new QStandardItem(i->second);
    item->setEditable(false);
    m_model_resource_hosts->appendRow(item);
  }
}
////////////////////////////////////////////////////////////////////////////

/*!
 * \brief This template structure allows us to validate every field.
 * When validation is failed user receives notification and focus moves to wrong field.
 */
template<class TC> struct field_validator_t {
  TC* fc; //field control
  bool (*f_validator)(const TC*);
  int8_t tab_index;
  QString validator_msg;
};

bool
is_le_empty_validate(const QLineEdit* le) {
  return !le->text().trimmed().isEmpty();
}

bool
folder_has_write_permission(const QLineEdit* le) {
  QFileInfo fi(le->text());
  return fi.isDir() && fi.isWritable();
}

bool
is_path_valid(const QLineEdit* le) {
  QFileInfo fi(le->text());
  return fi.exists();
}

bool
can_launch_application(const QLineEdit* le) {
  QFileInfo fi(le->text());
  if (fi.exists() && fi.isExecutable())
    return true;
  QString cmd;
  system_call_wrapper_error_t which_res =
      CSystemCallWrapper::which(le->text(), cmd);
  if (which_res != SCWE_SUCCESS) return false;
  QFileInfo fi2(cmd);
  return fi2.exists() && fi2.isExecutable();
}
////////////////////////////////////////////////////////////////////////////

void
DlgSettings::btn_ok_released() {
  static const char* empty_validator_msg = "Field can't be empty";
  static const char* folder_permission_validator_msg = "You don't have write permission to this folder";
  static const char* path_invalid_validator_msg = "Invalid path";
  static const char* can_launch_application_msg = "Can't launch application";

  QLineEdit* le[] = {ui->le_logs_storage, ui->le_ssh_keys_storage,
                    ui->le_p2p_command, ui->le_ssh_command, ui->le_rtm_db_folder};
  QStringList lst_home = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
  QString home_folder = lst_home.empty() ? "~" : lst_home[0];

  for (size_t i = 0; i < sizeof(le)/sizeof(QLineEdit*); ++i) {
    QString txt = le[i]->text();
    if (txt.at(0) == QChar('~')) {
      txt.replace(0, 1, home_folder);
      le[i]->setText(txt);
    }
  }

  field_validator_t<QLineEdit> le_validators[] = {
    {ui->le_ssh_user, is_le_empty_validate, 0, empty_validator_msg},

    {ui->le_ssh_keys_storage, is_le_empty_validate, 0, empty_validator_msg},
    {ui->le_ssh_keys_storage, is_path_valid, 0, path_invalid_validator_msg},
    {ui->le_ssh_keys_storage, folder_has_write_permission, 0, folder_permission_validator_msg},

    {ui->le_logs_storage, is_le_empty_validate, 0, empty_validator_msg},
    {ui->le_logs_storage, is_path_valid, 0, path_invalid_validator_msg},
    {ui->le_logs_storage, folder_has_write_permission, 0, folder_permission_validator_msg},

    {ui->le_rtm_db_folder, is_le_empty_validate, 0, empty_validator_msg},
    {ui->le_rtm_db_folder, is_path_valid, 0, path_invalid_validator_msg},
    {ui->le_rtm_db_folder, folder_has_write_permission, 0, folder_permission_validator_msg},

    {ui->le_p2p_command, is_le_empty_validate, 1, empty_validator_msg},
    {ui->le_ssh_command, is_le_empty_validate, 1, empty_validator_msg},
    {ui->le_ssh_command, can_launch_application, 1, can_launch_application_msg},
    {ui->le_terminal_cmd, is_le_empty_validate, 1, empty_validator_msg},
    {ui->le_terminal_cmd, can_launch_application, 1, can_launch_application_msg},
    {ui->le_terminal_arg, is_le_empty_validate, 1, empty_validator_msg},

    {ui->le_rhip_host, is_le_empty_validate, 2, empty_validator_msg},
    {ui->le_rhip_password, is_le_empty_validate, 2, empty_validator_msg},
    {ui->le_rhip_port, is_le_empty_validate, 2, empty_validator_msg},
    {ui->le_rhip_user, is_le_empty_validate, 2, empty_validator_msg},

    {NULL, NULL, -1, ""}
  };

  field_validator_t<QLineEdit>* tmp = le_validators;
  do {
    if (!tmp->fc->isVisible()) continue;
    if (!tmp->f_validator(tmp->fc)) {
      ui->tabWidget->setCurrentIndex(tmp->tab_index);
      tmp->fc->setFocus();
      QToolTip::showText(tmp->fc->mapToGlobal(QPoint()), tmp->validator_msg);
      return;
    }
  } while ((++tmp)->fc);

  CSettingsManager::Instance().set_ssh_user(ui->le_ssh_user->text());
  CSettingsManager::Instance().set_logs_storage(ui->le_logs_storage->text());
  CSettingsManager::Instance().set_ssh_keys_storage(ui->le_ssh_keys_storage->text());
  CSettingsManager::Instance().set_p2p_path(ui->le_p2p_command->text());
  CSettingsManager::Instance().set_ssh_path(ui->le_ssh_command->text());

  CSettingsManager::Instance().set_rh_host(ui->le_rhip_host->text());
  CSettingsManager::Instance().set_rh_pass(ui->le_rhip_password->text());
  CSettingsManager::Instance().set_rh_port(ui->le_rhip_port->text().toUInt());
  CSettingsManager::Instance().set_rh_user(ui->le_rhip_user->text());

  CSettingsManager::Instance().set_refresh_time_sec(ui->sb_refresh_timeout->value());
  CSettingsManager::Instance().set_notification_delay_sec(
        ui->sb_notification_delay->value());

  CSettingsManager::Instance().set_p2p_autoupdate(
        ui->chk_p2p_autoupdate->checkState()==Qt::Checked);
  CSettingsManager::Instance().set_rh_autoupdate(
        ui->chk_rh_autoupdate->checkState()==Qt::Checked);
  CSettingsManager::Instance().set_tray_autoupdate(
        ui->chk_tray_autoupdate->checkState()==Qt::Checked);
  CSettingsManager::Instance().set_rh_management_autoupdate(
        ui->chk_rhm_autoupdate->checkState()==Qt::Checked);

  CSettingsManager::Instance().set_p2p_update_freq(ui->cb_p2p_frequency->currentIndex());
  CSettingsManager::Instance().set_rh_update_freq(ui->cb_rh_frequency->currentIndex());
  CSettingsManager::Instance().set_tray_update_freq(ui->cb_tray_frequency->currentIndex());
  CSettingsManager::Instance().set_rh_management_freq(ui->cb_rhm_frequency->currentIndex());

  CSettingsManager::Instance().set_terminal_cmd(ui->le_terminal_cmd->text());
  CSettingsManager::Instance().set_terminal_arg(ui->le_terminal_arg->text());

  CSettingsManager::Instance().set_rtm_db_dir(ui->le_rtm_db_folder->text());
  CSettingsManager::Instance().save_all();
  this->close();
}
////////////////////////////////////////////////////////////////////////////

void
DlgSettings::btn_cancel_released() {
  this->close();
}
////////////////////////////////////////////////////////////////////////////

void
DlgSettings::btn_p2p_file_dialog_released() {
  QString fn = QFileDialog::getOpenFileName(this, "P2P command");
  if (fn == "") return;
  ui->le_p2p_command->setText(fn);
}
////////////////////////////////////////////////////////////////////////////

void
DlgSettings::btn_ssh_command_released() {
  QString fn = QFileDialog::getOpenFileName(this, "Ssh command");
  if (fn == "") return;
  ui->le_ssh_command->setText(fn);
}
////////////////////////////////////////////////////////////////////////////

void
DlgSettings::btn_logs_storage_released() {
  QString dir = QFileDialog::getExistingDirectory(this, "Logs storage");
  if (dir == "") return;
  ui->le_logs_storage->setText(dir);
}
////////////////////////////////////////////////////////////////////////////

void
DlgSettings::btn_ssh_keys_storage_released() {
  QString dir = QFileDialog::getExistingDirectory(this, "SSH-keys storage");
  if (dir == "") return;
  ui->le_ssh_keys_storage->setText(dir);
}
////////////////////////////////////////////////////////////////////////////

void
DlgSettings::btn_rtm_db_folder_released() {
  QString dir = QFileDialog::getExistingDirectory(this, "DB storage");
  if (dir == "") return;
  ui->le_rtm_db_folder->setText(dir);
}
////////////////////////////////////////////////////////////////////////////

void
DlgSettings::lstv_resource_hosts_double_clicked(QModelIndex ix0) {
  if (!ix0.isValid()) return;
  ui->le_rhip_host->setText(ix0.data().toString());
}
////////////////////////////////////////////////////////////////////////////

void
DlgSettings::btn_refresh_rh_list_released() {
  m_refresh_rh_list_progress_val = 0;
  ui->btn_refresh_rh_list->setEnabled(false);
  m_refresh_rh_list_timer.start();
  CRhController::Instance()->refresh();
}
////////////////////////////////////////////////////////////////////////////

void
DlgSettings::refresh_rh_list_timer_timeout() {
  ui->pb_refresh_rh_list->setValue(++m_refresh_rh_list_progress_val);
  if (m_refresh_rh_list_progress_val == CRhController::REFRESH_DELAY_SEC)
    m_refresh_rh_list_timer.stop();
}

void
DlgSettings::resource_host_list_updated_sl(bool has_changes) {
  UNUSED_ARG(has_changes);
  rebuild_rh_list_model();
  ui->btn_refresh_rh_list->setEnabled(true);
}
////////////////////////////////////////////////////////////////////////////
