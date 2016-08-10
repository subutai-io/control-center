#include "DlgAbout.h"
#include "ui_DlgAbout.h"
#include "SystemCallWrapper.h"
#include "SettingsManager.h"
#include "updater/HubComponentsUpdater.h"

using namespace update_system;

DlgAbout::DlgAbout(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgAbout)
{
  ui->setupUi(this);

  ui->lbl_tray_version_val->setText(GIT_VERSION);
  std::string str_version;

  CSystemCallWrapper::p2p_version(str_version);
  ui->lbl_p2p_version_val->setText(QString::fromStdString(str_version));
  CSystemCallWrapper::chrome_version(str_version);
  ui->lbl_chrome_version_val->setText(QString::fromStdString(str_version));
  ui->lbl_vbox_version_val->setText(CSystemCallWrapper::virtual_box_version());

  //init dct
  m_dct_fpb["p2p"] = {ui->pb_p2p, ui->btn_p2p_update};
  m_dct_fpb["tray"] = {ui->pb_tray, ui->btn_tray_update};

  bool p2p_updates_available = CHubComponentsUpdater::Instance()->is_update_available(IUpdaterComponent::P2P);
  ui->pb_p2p->setEnabled(p2p_updates_available);
  ui->btn_p2p_update->setEnabled(p2p_updates_available);

  bool tray_updates_available = CHubComponentsUpdater::Instance()->is_update_available(IUpdaterComponent::TRAY);
  ui->pb_tray->setEnabled(tray_updates_available);
  ui->btn_tray_update->setEnabled(tray_updates_available);

  //connect
  connect(ui->btn_p2p_update, SIGNAL(released()), this, SLOT(btn_p2p_update_released()));
  connect(ui->btn_tray_update, SIGNAL(released()), this, SLOT(btn_tray_update_released()));

  connect(CHubComponentsUpdater::Instance(), SIGNAL(download_file_progress(QString,qint64,qint64)),
          this, SLOT(download_progress(QString,qint64,qint64)));
  connect(CHubComponentsUpdater::Instance(), SIGNAL(update_available(QString)),
          this, SLOT(update_available(QString)));
  connect(CHubComponentsUpdater::Instance(), SIGNAL(updating_finished(QString,bool)),
          this, SLOT(update_finished(QString,bool)));
}

DlgAbout::~DlgAbout() {
  delete ui;
}

void
DlgAbout::btn_tray_update_released() {
  CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::TRAY);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::btn_p2p_update_released() {
  CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::P2P);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::download_progress(QString file_id,
                            qint64 rec,
                            qint64 total) {
  if (m_dct_fpb.find(file_id) == m_dct_fpb.end()) return;
  m_dct_fpb[file_id].pb->setValue((rec*100)/total);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::update_available(QString file_id) {
  if (m_dct_fpb.find(file_id) == m_dct_fpb.end()) return;
  m_dct_fpb[file_id].btn->setEnabled(true);
  m_dct_fpb[file_id].pb->setEnabled(true);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::update_finished(QString file_id,
                          bool success) {
  (void)success;
  if (m_dct_fpb.find(file_id) == m_dct_fpb.end()) return;
  m_dct_fpb[file_id].btn->setEnabled(false);
  m_dct_fpb[file_id].pb->setEnabled(false);
  m_dct_fpb[file_id].pb->setValue(0);
}
////////////////////////////////////////////////////////////////////////////
