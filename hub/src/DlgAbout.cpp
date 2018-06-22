#include <QThread>
#include <QtConcurrent/QtConcurrent>

#include "Commons.h"
#include "DlgAbout.h"
#include "ui_DlgAbout.h"
#include "SystemCallWrapper.h"
#include "SettingsManager.h"
#include "updater/HubComponentsUpdater.h"
#include "NotificationObserver.h"
#include "OsBranchConsts.h"

using namespace update_system;

QString get_p2p_version() {
  QString p2p_version = "";
  CSystemCallWrapper::p2p_version(p2p_version);

  p2p_version = p2p_version.remove("p2p");
  p2p_version = p2p_version.remove("version");
  p2p_version = p2p_version.remove("  ");

  return p2p_version;
}

QString get_x2go_version(){
    QString x2go_version = "";
    CSystemCallWrapper::x2go_version(x2go_version);
    return x2go_version;
}

QString get_vagrant_version(){
    QString vagrant_version = "";
    CSystemCallWrapper::vagrant_version(vagrant_version);
    return vagrant_version;
}

QString get_oracle_virtualbox_version(){
    QString version = "";
    CSystemCallWrapper::oracle_virtualbox_version(version);
    return version;
}

QString get_chrome_version(){
    QString version = "";
    CSystemCallWrapper::chrome_version(version);
    return version;
}

QString get_e2e_version(){
    QString version = "";
    CSystemCallWrapper::subutai_e2e_version(version);
    return version;

}

QString get_vagrant_subutai_version(){
    QString version = "";
    CSystemCallWrapper::vagrant_subutai_version(version);
    return version;
}

QString get_vagrant_vbguest_version(){
    QString version = "";
    CSystemCallWrapper::vagrant_vbguest_version(version);
    return version;
}

QString get_subutai_box_version(){
    QString version = "";
    QString provider = "virtualbox", box = subutai_box_name();
    CSystemCallWrapper::vagrant_latest_box_version(box, provider, version);
    return version;
}
////////////////////////////////////////////////////////////////////////////

DlgAbout::DlgAbout(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgAbout)
{
  ui->setupUi(this);
  ui->lbl_tray_version_val->setText(TRAY_VERSION + branch_name_str());
  current_browser = CSettingsManager::Instance().default_browser();

  set_visible_chrome("Chrome" == current_browser);
  set_visible_firefox("Firefox" == current_browser);


  this->setMinimumWidth(700);

  ui->gridLayout->setSizeConstraint(QLayout::SetFixedSize);
  ui->gl_components->setSizeConstraint(QLayout::SetFixedSize);
  ui->gridLayout_3->setSizeConstraint(QLayout::SetFixedSize);

  QLabel* lbls[] = { this->ui->lbl_chrome_version_val,
                     this->ui->lbl_p2p_version_val,
                     this->ui->lbl_tray_version_val,
                     this->ui->lbl_x2go_version_val,
                     this->ui->lbl_vagrant_version_val,
                     this->ui->lbl_chrome_version_val,
                     this->ui->lbl_subutai_e2e_val,
                     this->ui->lbl_subutai_plugin_version_val,
                     this->ui->lbl_vbguest_plugin_version_val,
                     this->ui->lbl_subutai_box_version,
                     nullptr
                   };

  QProgressBar* pbs[] = { this->ui->pb_tray,
                          this->ui->pb_p2p,
                          this->ui->pb_vagrant,
                          this->ui->pb_e2e,
                          this->ui->pb_oracle_virtualbox,
                          this->ui->pb_chrome,
                          this->ui->pb_subutai_box,
                          this->ui->pb_subutai_plugin,
                          this->ui->pb_vbguest_plugin,
                          this->ui->pb_x2go,
                          nullptr
                        };

  for (QLabel **i = lbls; *i; ++i) {
      (*i)->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
      (*i)->setWordWrap(true);
  }

  for (QProgressBar **i = pbs; *i; ++i) {
      (*i)->setHidden(true);
  }

  bool p2p_visible = CSettingsManager::Instance().p2p_path() != snap_p2p_path();
  ui->btn_p2p_update->setVisible(p2p_visible);
  //ui->pb_p2p->setVisible(p2p_visible);

  //connect
  connect(ui->btn_p2p_update, &QPushButton::released, this, &DlgAbout::btn_p2p_update_released);
  connect(ui->btn_tray_update, &QPushButton::released, this, &DlgAbout::btn_tray_update_released);
  connect(ui->btn_recheck, &QPushButton::released, this, &DlgAbout::btn_recheck_released);
  connect(ui->btn_x2go_update, &QPushButton::released, this, &DlgAbout::btn_x2go_update_released);
  connect(ui->btn_vagrant_update, &QPushButton::released, this, &DlgAbout::btn_vagrant_update_released);
  connect(ui->btn_oracle_virtualbox_update, &QPushButton::released, this, &DlgAbout::btn_oracle_virtualbox_update_released);
  connect(ui->btn_chrome, &QPushButton::released, this, &DlgAbout::btn_chrome_update_release);
  connect(ui->btn_subutai_e2e, &QPushButton::released, this, &DlgAbout::btn_e2e_update_released);
  connect(ui->btn_subutai_plugin_update, &QPushButton::released, this, &DlgAbout::btn_subutai_plugin_update_released);
  connect(ui->btn_vbguest_plugin_update, &QPushButton::released, this, &DlgAbout::btn_vbguest_plugin_update_released);
  connect(ui->btn_subutai_box, &QPushButton::released, this, &DlgAbout::btn_subutai_box_update_released);

  connect(CHubComponentsUpdater::Instance(), &CHubComponentsUpdater::download_file_progress,
          this, &DlgAbout::download_progress);
  connect(CHubComponentsUpdater::Instance(), &CHubComponentsUpdater::update_available,
          this, &DlgAbout::update_available);
  connect(CHubComponentsUpdater::Instance(), &CHubComponentsUpdater::updating_finished,
          this, &DlgAbout::update_finished);
  connect(CHubComponentsUpdater::Instance(), &CHubComponentsUpdater::installing_finished,
          this, &DlgAbout::install_finished);

  m_dct_fpb[IUpdaterComponent::P2P] = {ui->lbl_p2p_version_val, ui->pb_p2p, ui->btn_p2p_update,
                                       get_p2p_version};
  m_dct_fpb[IUpdaterComponent::TRAY] = {ui->lbl_tray_version_val, ui->pb_tray, ui->btn_tray_update,
                                        NULL};
  m_dct_fpb[IUpdaterComponent::X2GO] = {ui->lbl_x2go_version_val, ui->pb_x2go, ui->btn_x2go_update,
                                        get_x2go_version};
  m_dct_fpb[IUpdaterComponent::VAGRANT] = {ui->lbl_vagrant_version_val, ui->pb_vagrant, ui->btn_vagrant_update,
                                           get_vagrant_version};
  m_dct_fpb[IUpdaterComponent::ORACLE_VIRTUALBOX] = {ui->lbl_oracle_virtualbox_version_val, ui->pb_oracle_virtualbox, ui->btn_oracle_virtualbox_update,
                                                     get_oracle_virtualbox_version};

  m_dct_fpb[IUpdaterComponent::CHROME] = {ui->lbl_chrome_version_val, ui->pb_chrome, ui->btn_chrome,
                                         get_chrome_version};

  m_dct_fpb[IUpdaterComponent::E2E] = {ui->lbl_subutai_e2e_val, ui->pb_e2e, ui->btn_subutai_e2e,
                                       get_e2e_version};

  m_dct_fpb[IUpdaterComponent::VAGRANT_SUBUTAI] = {ui->lbl_subutai_plugin_version_val, ui->pb_subutai_plugin, ui->btn_subutai_plugin_update,
                                                  get_vagrant_subutai_version};

  m_dct_fpb[IUpdaterComponent::VAGRANT_VBGUEST] = {ui->lbl_vbguest_plugin_version_val, ui->pb_vbguest_plugin, ui->btn_vbguest_plugin_update,
                                                  get_vagrant_vbguest_version};

  m_dct_fpb[IUpdaterComponent::SUBUTAI_BOX] = {ui->lbl_subutai_box_version, ui->pb_subutai_box, ui->btn_subutai_box,
                                              get_subutai_box_version};

  ui->pb_initialization_progress->setMaximum(DlgAboutInitializer::COMPONENTS_COUNT);

  check_for_versions_and_updates();
  this->adjustSize();
}

void DlgAbout::set_visible_chrome(bool value) {
    ui->btn_chrome->setVisible(value);
    ui->lbl_chrome_version->setVisible(value);
    ui->lbl_chrome_version_val->setVisible(value);
    ui->pb_chrome->setVisible(value);
    this->adjustSize();
}

void DlgAbout::set_visible_firefox(bool value) {
    ui->btn_firefox->setVisible(value);
    ui->pb_firefox->setVisible(value);
    ui->lbl_firefox->setVisible(value);
    ui->lbl_firefox_version_val->setVisible(value);
    this->adjustSize();
}

DlgAbout::~DlgAbout() {
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::check_for_versions_and_updates() {
  ui->btn_recheck->setEnabled(false);
  ui->pb_initialization_progress->setEnabled(true);
  DlgAboutInitializer* di = new DlgAboutInitializer();

  connect(di, &DlgAboutInitializer::finished,
          this, &DlgAbout::initialization_finished);
  connect(di, &DlgAboutInitializer::got_chrome_version,
          this, &DlgAbout::got_chrome_version_sl);
  connect(di, &DlgAboutInitializer::got_p2p_version,
          this, &DlgAbout::got_p2p_version_sl);
  connect(di, &DlgAboutInitializer::got_x2go_version,
          this, &DlgAbout::got_x2go_version_sl);
  connect(di, &DlgAboutInitializer::got_vagrant_version,
          this, &DlgAbout::got_vagrant_version_sl);
  connect(di, &DlgAboutInitializer::got_oracle_virtualbox_version,
          this, &DlgAbout::got_oracle_virtualbox_version_sl);
  connect(di, &DlgAboutInitializer::got_e2e_version,
          this, &DlgAbout::got_e2e_version_sl);
  connect(di, &DlgAboutInitializer::got_subutai_plugin_version,
          this, &DlgAbout::got_subutai_plugin_version_sl);
  connect(di, &DlgAboutInitializer::got_vbguest_plugin_version,
          this, &DlgAbout::got_vbguest_plugin_version_sl);
  connect(di, &DlgAboutInitializer::got_subutai_box_version,
          this, &DlgAbout::got_subutai_box_version_sl);
  connect(di, &DlgAboutInitializer::update_available,
          this, &DlgAbout::update_available_sl);
  connect(di, &DlgAboutInitializer::init_progress,
          this, &DlgAbout::init_progress_sl);
  di->startWork();
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::btn_tray_update_released() {
  ui->pb_tray->setHidden(false);
  ui->btn_tray_update->setEnabled(false);
  CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::TRAY);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::btn_p2p_update_released() {
  ui->pb_p2p->setHidden(false);
  ui->btn_p2p_update->setEnabled(false);
  if(ui->lbl_p2p_version_val->text()=="undefined")
      CHubComponentsUpdater::Instance()->install(IUpdaterComponent::P2P);
  else CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::P2P);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::btn_x2go_update_released() {
    ui->pb_x2go->setHidden(false);
    ui->btn_x2go_update->setEnabled(false);
    if(ui->lbl_x2go_version_val->text()=="undefined")
        CHubComponentsUpdater::Instance()->install(IUpdaterComponent::X2GO);
    else CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::X2GO);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::btn_vagrant_update_released() {
    ui->pb_vagrant->setHidden(false);
    ui->btn_vagrant_update->setEnabled(false);
    if(ui->lbl_vagrant_version_val->text()=="undefined")
        CHubComponentsUpdater::Instance()->install(IUpdaterComponent::VAGRANT);
    else CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::VAGRANT);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::btn_oracle_virtualbox_update_released() {
    ui->pb_oracle_virtualbox->setHidden(false);
    ui->btn_oracle_virtualbox_update->setHidden(false);
    ui->btn_oracle_virtualbox_update->setEnabled(false);
    if(ui->lbl_oracle_virtualbox_version_val->text()=="undefined")
        CHubComponentsUpdater::Instance()->install(IUpdaterComponent::ORACLE_VIRTUALBOX);
    else CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::ORACLE_VIRTUALBOX);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::btn_chrome_update_release() {
    ui->pb_chrome->setHidden(false);
    ui->btn_chrome->setEnabled(false);
    if(ui->lbl_chrome_version_val->text()=="undefined")
        CHubComponentsUpdater::Instance()->install(IUpdaterComponent::CHROME);
    else CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::CHROME);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::btn_e2e_update_released(){
    ui->pb_e2e->setHidden(false);
    ui->btn_subutai_e2e->setEnabled(false);
    if(ui->lbl_subutai_e2e_val->text() == "undefined") {
        CHubComponentsUpdater::Instance()->install(IUpdaterComponent::E2E);
    }
    else CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::E2E);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::btn_subutai_plugin_update_released() {
    ui->pb_subutai_plugin->setHidden(false);
    ui->btn_subutai_plugin_update->setEnabled(false);
    if(ui->lbl_subutai_plugin_version_val->text() == "undefined"){
        CHubComponentsUpdater::Instance()->install(IUpdaterComponent::VAGRANT_SUBUTAI);
    }
    else CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::VAGRANT_SUBUTAI);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::btn_vbguest_plugin_update_released() {
    ui->pb_vbguest_plugin->setHidden(false);
    ui->btn_vbguest_plugin_update->setEnabled(false);
    if(ui->lbl_vbguest_plugin_version_val->text() == "undefined"){
        CHubComponentsUpdater::Instance()->install(IUpdaterComponent::VAGRANT_VBGUEST);
    }
    else CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::VAGRANT_VBGUEST);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::btn_subutai_box_update_released() {
    ui->pb_subutai_box->setHidden(false);
    ui->btn_subutai_box->setEnabled(false);
    if(ui->lbl_subutai_box_version->text() == "undefined"){
        CHubComponentsUpdater::Instance()->install(IUpdaterComponent::SUBUTAI_BOX);
    }
    else CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::SUBUTAI_BOX);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::btn_recheck_released() {
  for (auto it = m_dct_fpb.begin(); it != m_dct_fpb.end(); it++) {
    it->second.btn->setEnabled(false);
  }
  check_for_versions_and_updates();
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::download_progress(const QString& file_id,
                            qint64 rec,
                            qint64 total) {
  if (m_dct_fpb.find(file_id) == m_dct_fpb.end()) return;
  m_dct_fpb[file_id].pb->setValue((rec*100)/total);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::update_available(const QString& file_id) {
  if (m_dct_fpb.find(file_id) == m_dct_fpb.end()) return;
  m_dct_fpb[file_id].pb->setEnabled(!(CHubComponentsUpdater::Instance()->is_in_progress(file_id)));
  m_dct_fpb[file_id].btn->setEnabled(!(CHubComponentsUpdater::Instance()->is_in_progress(file_id)));
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::update_finished(const QString& file_id,
                          bool success) {
  UNUSED_ARG(success);
  if (m_dct_fpb.find(file_id) == m_dct_fpb.end()) return;
  m_dct_fpb[file_id].btn->setEnabled(false);
  m_dct_fpb[file_id].pb->setEnabled(false);
  m_dct_fpb[file_id].pb->setValue(0);
  m_dct_fpb[file_id].pb->setRange(0, 100);
  if (m_dct_fpb[file_id].pf_version) {
    m_dct_fpb[file_id].lbl->setText(m_dct_fpb[file_id].pf_version());
  }
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::initialization_finished() {
  ui->lbl_about_init->setEnabled(false);
  ui->pb_initialization_progress->setEnabled(false);
  ui->btn_recheck->setEnabled(true);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::init_progress_sl(int part,
                           int total) {
  UNUSED_ARG(total);
  ui->pb_initialization_progress->setValue(part);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::got_p2p_version_sl(QString version) {
  ui->lbl_p2p_version_val->setText(version);
  if(version == "undefined"){
      ui->btn_p2p_update->setText(tr("Install"));
  }
  else ui->btn_p2p_update->setText(tr("Update"));
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::got_chrome_version_sl(QString version) {
  ui->lbl_chrome_version_val->setText(version);
  if(version == "undefined"){
      ui->btn_chrome->setText(tr("Install "));
  }
  else{
      ui->btn_chrome->setText(tr("Update"));
  }
}

////////////////////////////////////////////////////////////////////////////

void DlgAbout::got_e2e_version_sl(QString version) {
    ui->lbl_subutai_e2e_val->setText(version);
    if(version == "undefined"){
        ui->btn_subutai_e2e->setText(tr("Install"));
    }
    else{
        ui->btn_subutai_e2e->setText(tr("Update"));
    }
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::got_x2go_version_sl(QString version) {
    if(version == "undefined")
        ui->btn_x2go_update->setText(tr("Install"));
    else ui->btn_x2go_update->setText(tr("Update"));
    ui->lbl_x2go_version_val->setText(version);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::got_vagrant_version_sl(QString version) {
    if(version == "undefined")
        ui->btn_vagrant_update->setText(tr("Install"));
    else ui->btn_vagrant_update->setText(tr("Update"));
    ui->lbl_vagrant_version_val->setText(version);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::got_oracle_virtualbox_version_sl(QString version) {
    if(version == "undefined")
        ui->btn_oracle_virtualbox_update->setText(tr("Install"));
    else ui->btn_oracle_virtualbox_update->setText(tr("Update"));
    ui->lbl_oracle_virtualbox_version_val->setText(version);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::got_subutai_plugin_version_sl(QString version) {
    if(version == "undefined") {
        // TODO
        ui->btn_subutai_plugin_update->setHidden(false);
        ui->btn_subutai_plugin_update->setText(tr("Install"));
    } else {
        ui->btn_subutai_plugin_update->setText(tr("Update"));
    }
    ui->lbl_subutai_plugin_version_val->setText(version);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::got_vbguest_plugin_version_sl(QString version) {
    if(version == "undefined")
        ui->btn_vbguest_plugin_update->setText(tr("Install"));
    else ui->btn_vbguest_plugin_update->setText(tr("Update"));
    ui->lbl_vbguest_plugin_version_val->setText(version);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::got_subutai_box_version_sl(QString version) {
    if(version == "undefined") {
        ui->btn_subutai_box->setText(tr("Install"));
    } else {
        ui->btn_subutai_box->setText(tr("Update"));
    }
    ui->lbl_subutai_box_version->setText(version);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::update_available_sl(const QString& component_id, bool available) {
    bool update_available = (!(CHubComponentsUpdater::Instance()->is_in_progress(component_id)) && available);

    if(m_dct_fpb.find(component_id) == m_dct_fpb.end()) {
        return;
    }

    // if not available component updates but checkbox icon
    if (!update_available) {
        m_dct_fpb[component_id].btn->setHidden(true);
        ui->gl_components->replaceWidget(m_dct_fpb[component_id].btn, DlgAbout::checked_btn());
    }

    m_dct_fpb[component_id].pb->setEnabled(update_available);
    m_dct_fpb[component_id].btn->setEnabled(update_available);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAboutInitializer::do_initialization() {
  try {
    int initialized_component_count = 0;

    QString p2p_version = get_p2p_version();
    emit got_p2p_version(p2p_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString chrome_version;
    CSystemCallWrapper::chrome_version(chrome_version);
    emit got_chrome_version(chrome_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

/*currently not used but we might need *
    QString rh_version = CSystemCallWrapper::rh_version();
    emit got_rh_version(rh_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString rhm_version = CSystemCallWrapper::rhm_version();
    emit got_rh_management_version(rhm_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);
*/
    QString x2go_version = get_x2go_version();
    emit got_x2go_version(x2go_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString vagrant_version = get_vagrant_version();
    emit got_vagrant_version(vagrant_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString oracle_virtualbox_version = get_oracle_virtualbox_version();
    emit got_oracle_virtualbox_version(oracle_virtualbox_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString subutai_e2e_version = get_e2e_version();
    emit got_e2e_version(subutai_e2e_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString subutai_plugin_versin = get_vagrant_subutai_version();
    emit got_subutai_plugin_version(subutai_plugin_versin);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString vbguest_plugin_versin = get_vagrant_vbguest_version();
    emit got_vbguest_plugin_version(vbguest_plugin_versin);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString subutai_box_version = get_subutai_box_version();
    emit got_subutai_box_version(subutai_box_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString uas[] = {
      IUpdaterComponent::P2P, IUpdaterComponent::TRAY,
      IUpdaterComponent::X2GO, IUpdaterComponent::E2E,
      IUpdaterComponent::VAGRANT, IUpdaterComponent::ORACLE_VIRTUALBOX,
      IUpdaterComponent::CHROME, IUpdaterComponent::VAGRANT_SUBUTAI, IUpdaterComponent::VAGRANT_VBGUEST,
      IUpdaterComponent::SUBUTAI_BOX, ""};
    std::vector <bool> ua;
    for (int i = 0; uas[i] != ""; ++i) {
      ua.push_back(CHubComponentsUpdater::Instance()->is_update_available(uas[i]));
      emit init_progress(++initialized_component_count, COMPONENTS_COUNT);
    }
    for (int i = 0; uas[i] != ""; i++) {
      emit update_available(uas[i], ua[i]);
    }
  } catch (std::exception& ex) {
    qCritical("Err in DlgAboutInitializer::do_initialization() . %s", ex.what());
  }

  emit finished();
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::install_finished(const QString &file_id, bool success) {
    qDebug()<<"Install finished  for"<<file_id<<"with result"<<success;
    if (m_dct_fpb.find(file_id) == m_dct_fpb.end()) return;
    m_dct_fpb[file_id].btn->setEnabled(false);
    m_dct_fpb[file_id].pb->setEnabled(false);
    m_dct_fpb[file_id].pb->setValue(0);
    m_dct_fpb[file_id].pb->setRange(0, 100);
    m_dct_fpb[file_id].btn->setText(tr("Update %1").arg(CHubComponentsUpdater::Instance()->component_name(file_id)));
    if (m_dct_fpb[file_id].pf_version) {
      m_dct_fpb[file_id].lbl->setText(m_dct_fpb[file_id].pf_version());
    }

    if (success) {
        m_dct_fpb[file_id].pb->setHidden(true);
        m_dct_fpb[file_id].btn->setHidden(true);
        ui->gl_components->replaceWidget(m_dct_fpb[file_id].btn, DlgAbout::checked_btn());
    }
}

////////////////////////////////////////////////////////////////////////////
void DlgAboutInitializer::abort() {
  emit finished();
}

////////////////////////////////////////////////////////////////////////////
/// \brief DlgAbout::checked_btn
/// Creates button with checkboxed icon
/// \return
///
QPushButton* DlgAbout::checked_btn() {
    QPushButton* btn_checked = new QPushButton("");
    static QPixmap pixmap(":/hub/check_box.png");
    static QIcon icon(pixmap);
    btn_checked->setIcon(icon);
    btn_checked->setIconSize(pixmap.rect().size());
    btn_checked->setStyleSheet("border-style: outset; border-width: 0px;");

    return btn_checked;
}

////////////////////////////////////////////////////////////////////////////
