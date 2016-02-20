#include "DlgSettings.h"
#include "ui_DlgSettings.h"
#include "SettingsManager.h"

DlgSettings::DlgSettings(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgSettings)
{
  ui->setupUi(this);
  ui->sb_refresh_timeout->setValue(CSettingsManager::Instance().refresh_time_sec());
  connect(ui->btn_ok, SIGNAL(released()), this, SLOT(btn_ok_released()));
  connect(ui->btn_cancel, SIGNAL(released()), this, SLOT(btn_cancel_released()));
}

DlgSettings::~DlgSettings()
{
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

void DlgSettings::btn_ok_released()
{
  CSettingsManager::Instance().set_refresh_time_sec(ui->sb_refresh_timeout->value());
  CSettingsManager::Instance().save_all();
  QDialog::accept();
}
////////////////////////////////////////////////////////////////////////////

void DlgSettings::btn_cancel_released()
{
  QDialog::reject();
}
////////////////////////////////////////////////////////////////////////////
