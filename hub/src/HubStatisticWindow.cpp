#include <QCloseEvent>
#include "HubStatisticWindow.h"
#include "ui_HubStatisticWindow.h"
#include "SettingsManager.h"
#include "DlgSettings.h"
#include "TreeModelCSSContainer.h"
#include "TreeModelSSPeerUser.h"

HubStatisticWindow::HubStatisticWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::HubStatisticWindow),
  m_cont_model(NULL),
  m_peer_user_model(NULL){
//  init_form();
}

HubStatisticWindow::~HubStatisticWindow() {
  if (m_cont_model) delete m_cont_model;
  if (m_peer_user_model) delete m_peer_user_model;
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

void HubStatisticWindow::init_form() {
  ui->setupUi(this);
  refresh_timer_timeout();
  m_refresh_timer.setInterval(CSettingsManager::Instance().refresh_time_sec() * 1000);
  m_refresh_timer.start();

  connect(&m_refresh_timer, SIGNAL(timeout()),
          this, SLOT(refresh_timer_timeout()));
  connect(ui->ma_settings, SIGNAL(triggered(bool)), this, SLOT(ma_settings_triggered(bool)));
  connect(ui->ma_quit, SIGNAL(triggered(bool)), this, SLOT(ma_quit_triggered(bool)));
}
////////////////////////////////////////////////////////////////////////////

void HubStatisticWindow::adjust_lbl_font_size(QLabel *lbl) {
  QFont font = lbl->font();
  int height = lbl->height();
  QFontMetrics fm(font);
  int fontSize = -1;
  do {
    ++fontSize;
    font.setPointSize(fontSize + 1);
    fm = QFontMetrics(font);
  } while(fm.height() <= height);
  font.setPointSize(fontSize);
  lbl->setFont(font);
}
////////////////////////////////////////////////////////////////////////////

void HubStatisticWindow::refresh_timer_timeout() {
  int err_code = EL_SUCCESS;
  int http_code = 200;

  //do nothing yet
  std::vector<CSSEnvironment> lst_env = CRestWorker::get_environments(http_code, err_code);
  if (err_code == 0) {
  }

  err_code = 0;
  CSSContainer cont_data = CRestWorker::get_containers(http_code, err_code);
  if (err_code == 0) {
    if (m_cont_model == NULL || m_cont_model->container_changed(cont_data)) {
      m_cont_model = new CTreeModelCSSContainer(cont_data);
      ui->tree_v_containers->setModel(m_cont_model);
      ui->tree_v_containers->expandAll();
    }
  }

  err_code = 0;
  CSSPeerUser user_data = CRestWorker::get_peer_users(http_code, err_code);
  if (err_code == 0) {
    if (m_peer_user_model == NULL || m_peer_user_model->peer_user_changed(user_data)) {
      m_peer_user_model = new CTreeModelSSPeerUser(user_data);
      ui->tree_v_peers_users->setModel(m_peer_user_model);
      ui->tree_v_peers_users->expandAll();
    }
  }

  err_code = 0;
  CSSBalance balance = CRestWorker::get_balance(http_code, err_code);
  if (err_code == 0) {
    ui->lbl_balance->setText(QString("Balance: %1").arg(balance.value()));
  }
}
////////////////////////////////////////////////////////////////////////////

void HubStatisticWindow::ma_settings_triggered(bool)
{
  DlgSettings dlg(this);
  dlg.exec();
  if (dlg.result() != QDialog::Accepted) return;
  m_refresh_timer.setInterval(CSettingsManager::Instance().refresh_time_sec()*1000);
  m_refresh_timer.start(); //will be restarted
}
////////////////////////////////////////////////////////////////////////////

void HubStatisticWindow::ma_quit_triggered(bool) {
  qApp->quit();
}
////////////////////////////////////////////////////////////////////////////

void HubStatisticWindow::resizeEvent(QResizeEvent *event) {
  QMainWindow::resizeEvent(event);
  adjust_lbl_font_size(ui->lbl_balance);
}
////////////////////////////////////////////////////////////////////////////

void HubStatisticWindow::showEvent(QShowEvent *event) {
  QMainWindow::showEvent(event);
  adjust_lbl_font_size(ui->lbl_balance);
}
////////////////////////////////////////////////////////////////////////////

void HubStatisticWindow::closeEvent(QCloseEvent *event) {
  if (QSystemTrayIcon::isSystemTrayAvailable()) {
    hide();
    event->ignore();
  }
}
////////////////////////////////////////////////////////////////////////////
