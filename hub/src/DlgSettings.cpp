#include <QFileDialog>
#include "DlgSettings.h"
#include "ui_DlgSettings.h"
#include "SettingsManager.h"

DlgSettings::DlgSettings(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgSettings)
{
  ui->setupUi(this);
  ui->sb_refresh_timeout->setValue(CSettingsManager::Instance().refresh_time_sec());
  ui->le_p2p_command->setText(CSettingsManager::Instance().p2p_path());
  ui->le_terminal_command->setText(CSettingsManager::Instance().terminal_path());
  ui->sb_notification_delay->setMinimum(CSettingsManager::NOTIFICATION_DELAY_MIN);
  ui->sb_notification_delay->setMaximum(CSettingsManager::NOTIFICATION_DELAY_MAX);
  ui->sb_notification_delay->setValue(CSettingsManager::Instance().notification_delay_sec());

  connect(ui->btn_ok, SIGNAL(released()), this, SLOT(btn_ok_released()));
  connect(ui->btn_cancel, SIGNAL(released()), this, SLOT(btn_cancel_released()));

  connect(ui->btn_terminal_file_dialog, SIGNAL(released()),
          this, SLOT(btn_terminal_file_dialog_released()));

  connect(ui->btn_p2p_file_dialog, SIGNAL(released()),
          this, SLOT(btn_p2p_file_dialog_released()));
}

DlgSettings::~DlgSettings()
{
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

void DlgSettings::btn_ok_released()
{
  CSettingsManager::Instance().set_refresh_time_sec(ui->sb_refresh_timeout->value());
  if (ui->le_p2p_command->text() != "")
    CSettingsManager::Instance().set_p2p_path(ui->le_p2p_command->text());
  if (ui->le_terminal_command->text() != "")
    CSettingsManager::Instance().set_terminal_path(ui->le_terminal_command->text());
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
  QString fn = QFileDialog::getOpenFileName(this, "Terminal command");
  if (fn == "") return;
  ui->le_p2p_command->setText(fn);
}
////////////////////////////////////////////////////////////////////////////
