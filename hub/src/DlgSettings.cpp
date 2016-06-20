#include <QToolTip>
#include <QFileDialog>
#include "DlgSettings.h"
#include "ui_DlgSettings.h"
#include "SettingsManager.h"

DlgSettings::DlgSettings(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgSettings),
  m_tab_resize_filter(NULL)
{
  ui->setupUi(this);
  ui->sb_refresh_timeout->setValue(CSettingsManager::Instance().refresh_time_sec());
  ui->le_p2p_command->setText(CSettingsManager::Instance().p2p_path());
  ui->le_terminal_command->setText(CSettingsManager::Instance().terminal_path());
  ui->sb_notification_delay->setMinimum(CSettingsManager::NOTIFICATION_DELAY_MIN);
  ui->sb_notification_delay->setMaximum(CSettingsManager::NOTIFICATION_DELAY_MAX);
  ui->sb_notification_delay->setValue(CSettingsManager::Instance().notification_delay_sec());
  ui->le_updater_command->setText(CSettingsManager::Instance().libssh2_app_path());
  ui->le_ssh_command->setText(CSettingsManager::Instance().ssh_path());
  ui->le_ssh_user->setText(CSettingsManager::Instance().ssh_user());
  ui->le_rhip_host->setText(CSettingsManager::Instance().rh_host());
  ui->le_rhip_password->setText(CSettingsManager::Instance().rh_pass());
  ui->le_rhip_port->setText(CSettingsManager::Instance().rh_port());
  ui->le_rhip_user->setText(CSettingsManager::Instance().rh_user());
  ui->le_logs_storage->setText(CSettingsManager::Instance().logs_storage());
  ui->le_ssh_keys_storage->setText(CSettingsManager::Instance().ssh_keys_storage());

  m_tab_resize_filter = new TabResizeFilter(ui->tabWidget);
  ui->tabWidget->installEventFilter(m_tab_resize_filter);

  connect(ui->btn_ok, SIGNAL(released()), this, SLOT(btn_ok_released()));
  connect(ui->btn_cancel, SIGNAL(released()), this, SLOT(btn_cancel_released()));

  connect(ui->btn_terminal_file_dialog, SIGNAL(released()),
          this, SLOT(btn_terminal_file_dialog_released()));

  connect(ui->btn_p2p_file_dialog, SIGNAL(released()),
          this, SLOT(btn_p2p_file_dialog_released()));

  connect(ui->btn_ssh_command, SIGNAL(released()),
          this, SLOT(btn_ssh_command_released()));

  connect(ui->btn_updater_command, SIGNAL(released()),
          this, SLOT(btn_updater_path_dialog_released()));

  connect(ui->btn_logs_storage, SIGNAL(released()),
          this, SLOT(btn_logs_storage_released()));

  connect(ui->btn_ssh_keys_storage, SIGNAL(released()),
          this, SLOT(btn_ssh_keys_storage_released()));
}

DlgSettings::~DlgSettings()
{
  if (m_tab_resize_filter) delete m_tab_resize_filter;
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

//return true if field is valid.
typedef bool (*pf_validator)(const QLineEdit*);
struct field_validator_t {
  QLineEdit* le;
  pf_validator f_validator;
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

void
DlgSettings::btn_ok_released() {
  static const char* empty_validator_msg = "Field can't be empty";
  static const char* folder_permission_validator_msg = "You haven't write permission to this folder";

  field_validator_t validators[] = {
    {ui->le_ssh_user, is_le_empty_validate, 0, empty_validator_msg},
    {ui->le_logs_storage, is_le_empty_validate, 0, empty_validator_msg},
    {ui->le_ssh_keys_storage, is_le_empty_validate, 0, empty_validator_msg},
    {ui->le_logs_storage, folder_has_write_permission, 0, folder_permission_validator_msg},
    {ui->le_ssh_keys_storage, folder_has_write_permission, 0, folder_permission_validator_msg},

    {ui->le_p2p_command, is_le_empty_validate, 1, empty_validator_msg},
    {ui->le_ssh_command, is_le_empty_validate, 1, empty_validator_msg},
    {ui->le_terminal_command, is_le_empty_validate, 1, empty_validator_msg},
    {ui->le_updater_command, is_le_empty_validate, 1, empty_validator_msg},

    {ui->le_rhip_host, is_le_empty_validate, 2, empty_validator_msg},
    {ui->le_rhip_password, is_le_empty_validate, 2, empty_validator_msg},
    {ui->le_rhip_port, is_le_empty_validate, 2, empty_validator_msg},
    {ui->le_rhip_user, is_le_empty_validate, 2, empty_validator_msg},

    {NULL, NULL, -1, ""}
  };

  field_validator_t* tmp = validators;
  do {
    if (!tmp->f_validator(tmp->le)) {
      ui->tabWidget->setCurrentIndex(tmp->tab_index);
      tmp->le->setFocus();
      QToolTip::showText(tmp->le->mapToGlobal(QPoint()), tmp->validator_msg);
      return;
    }
  } while ((++tmp)->le);

  CSettingsManager::Instance().set_refresh_time_sec(ui->sb_refresh_timeout->value());
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
DlgSettings::btn_terminal_file_dialog_released() {
  QString fn = QFileDialog::getOpenFileName(this, "Terminal command");
  if (fn == "") return;
  ui->le_terminal_command->setText(fn);
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
DlgSettings::btn_updater_path_dialog_released() {
  QString fn = QFileDialog::getOpenFileName(this, "Libssh2 command");
  if (fn == "") return;
  ui->le_updater_command->setText(fn);
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
  ui->le_logs_storage->setText(dir);
}
////////////////////////////////////////////////////////////////////////////
