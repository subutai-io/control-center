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
  ui->le_updater_command->setText(CSettingsManager::Instance().ss_updater_path());
  ui->le_ssh_command->setText(CSettingsManager::Instance().ssh_path());
  ui->le_ssh_user->setText(CSettingsManager::Instance().ssh_user());
  ui->le_rhip_host->setText(CSettingsManager::Instance().rh_host());
  ui->le_rhip_password->setText(CSettingsManager::Instance().rh_pass());
  ui->le_rhip_port->setText(CSettingsManager::Instance().rh_port());
  ui->le_rhip_user->setText(CSettingsManager::Instance().rh_user());

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
}

DlgSettings::~DlgSettings()
{
  if (m_tab_resize_filter) delete m_tab_resize_filter;
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

void DlgSettings::btn_ok_released()
{
  //todo refactor this. see SettingsManager.cpp for more details
  CSettingsManager::Instance().set_refresh_time_sec(ui->sb_refresh_timeout->value());
  if (ui->le_p2p_command->text() != "")
    CSettingsManager::Instance().set_p2p_path(ui->le_p2p_command->text());
  if (ui->le_terminal_command->text() != "")
    CSettingsManager::Instance().set_terminal_path(ui->le_terminal_command->text());
  if (ui->le_ssh_command->text() != "")
    CSettingsManager::Instance().set_ssh_path(ui->le_ssh_command->text());
  if (ui->le_updater_command->text() != "")
    CSettingsManager::Instance().set_ss_updater_path(ui->le_updater_command->text());
  if (ui->le_ssh_user->text() != "")
    CSettingsManager::Instance().set_ssh_user(ui->le_ssh_user->text());
  if (ui->le_rhip_host->text() != "")
    CSettingsManager::Instance().set_rhip_getter_host(ui->le_rhip_host->text());
  if (ui->le_rhip_password->text() != "")
    CSettingsManager::Instance().set_rhip_getter_pass(ui->le_rhip_password->text());
  if (ui->le_rhip_port->text() != "")
    CSettingsManager::Instance().set_rhip_getter_port(ui->le_rhip_port->text());
  if (ui->le_rhip_user->text() != "")
    CSettingsManager::Instance().set_rhip_getter_user(ui->le_rhip_user->text());

  CSettingsManager::Instance().save_all();
  QDialog::accept();
}
////////////////////////////////////////////////////////////////////////////

void DlgSettings::btn_cancel_released()
{
  QDialog::reject();
}
////////////////////////////////////////////////////////////////////////////

void DlgSettings::btn_terminal_file_dialog_released()
{
  QString fn = QFileDialog::getOpenFileName(this, "Terminal command");
  if (fn == "") return;
  ui->le_terminal_command->setText(fn);
}
////////////////////////////////////////////////////////////////////////////

void DlgSettings::btn_p2p_file_dialog_released()
{
  QString fn = QFileDialog::getOpenFileName(this, "P2P command");
  if (fn == "") return;
  ui->le_p2p_command->setText(fn);
}
////////////////////////////////////////////////////////////////////////////

void DlgSettings::btn_updater_path_dialog_released()
{
  QString fn = QFileDialog::getOpenFileName(this, "Libssh2 command");
  if (fn == "") return;
  ui->le_updater_command->setText(fn);
}
////////////////////////////////////////////////////////////////////////////

void DlgSettings::btn_ssh_command_released()
{
  QString fn = QFileDialog::getOpenFileName(this, "Ssh command");
  if (fn == "") return;
  ui->le_ssh_command->setText(fn);
}
////////////////////////////////////////////////////////////////////////////
