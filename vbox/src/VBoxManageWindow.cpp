#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QCloseEvent>

#include "VBoxManageWindow.h"
#include "ui_VBoxManageWindow.h"

#include "IVirtualMachine.h"
#include "IVBoxManager.h"
#include <QDebug>


VBoxManageWindow::VBoxManageWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::VBoxManageWindow)
{
  ui->setupUi(this);
  ui->gl_vm_controls->setRowStretch(0, 1);


}

VBoxManageWindow::~VBoxManageWindow()
{
  for (auto i = m_dct_vm_controls.begin(); i != m_dct_vm_controls.end(); ++i) {
    if (i->second) delete i->second;
  }
  m_dct_vm_controls.erase(m_dct_vm_controls.begin(), m_dct_vm_controls.end());
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

void VBoxManageWindow::init_machines() {
  for (auto i = CVBoxManagerSingleton::Instance()->dct_machines().begin();
       i != CVBoxManagerSingleton::Instance()->dct_machines().end(); ++i) {
    add_vm_control(i->first);
  }

  connect(CVBoxManagerSingleton::Instance(), SIGNAL(vm_add(const com::Bstr&)),
          this, SLOT(vm_added(const com::Bstr&)));
  connect(CVBoxManagerSingleton::Instance(), SIGNAL(vm_remove(const com::Bstr&)),
          this, SLOT(vm_removed(const com::Bstr&)));
  connect(CVBoxManagerSingleton::Instance(), SIGNAL(vm_state_changed(const com::Bstr&)),
          this, SLOT(vm_state_changed(const com::Bstr&)));
  connect(CVBoxManagerSingleton::Instance(), SIGNAL(vm_launch_progress(const com::Bstr&, uint32_t)),
          this, SLOT(vm_launch_progress(const com::Bstr&, uint32_t)));
}
////////////////////////////////////////////////////////////////////////////


void VBoxManageWindow::add_vm_control(const com::Bstr& vm_id) {
  CVmControl* vm_c = new CVmControl(CVBoxManagerSingleton::Instance()->vm_by_id(vm_id));
  m_dct_vm_controls[vm_id] = vm_c;

  for (size_t i = 0; i < CVmControl::WIDGETS_COUNT; ++i) {
    if (vm_c->widgets()[i] == NULL) continue;
    ui->gl_vm_controls->addWidget(vm_c->widgets()[i], m_dct_vm_controls.size(), CVmControl::WIDGETS_COLS[i]);
  }  
  ui->gl_vm_controls->setRowStretch(m_dct_vm_controls.size(), 5);
  connect(vm_c, SIGNAL(btn_turn_released_sig(const com::Bstr&)), this, SLOT(vmc_turn_released(const com::Bstr&)));
}
////////////////////////////////////////////////////////////////////////////

void VBoxManageWindow::remove_vm_control(const com::Bstr &vm_id) {
  auto vmc_it = m_dct_vm_controls.find(vm_id);
  if (vmc_it == m_dct_vm_controls.end())
    return;
  CVmControl* vmc = vmc_it->second;
  for (size_t i = 0; i < CVmControl::WIDGETS_COUNT; ++i) {
    if (vmc->widgets()[i] == NULL) continue;
    ui->gl_vm_controls->removeWidget(vmc->widgets()[i]);
  }
  disconnect(vmc, SIGNAL(btn_turn_released_sig(const com::Bstr&)), this, SLOT(vmc_turn_released(const com::Bstr&)));

  delete vmc;
  m_dct_vm_controls.erase(vmc_it);
}
////////////////////////////////////////////////////////////////////////////

void VBoxManageWindow::vm_added(const com::Bstr &vm_id) {
  add_vm_control(vm_id);
}
////////////////////////////////////////////////////////////////////////////

void VBoxManageWindow::vm_removed(const com::Bstr &vm_id) {
  remove_vm_control(vm_id);
}
////////////////////////////////////////////////////////////////////////////

void VBoxManageWindow::vm_state_changed(const com::Bstr &vm_id) {
  if (m_dct_vm_controls.find(vm_id) == m_dct_vm_controls.end()) return;

  m_dct_vm_controls[vm_id]->m_lbl_state->setText(
        CCommons::vm_state_to_str(
          CVBoxManagerSingleton::Instance()->vm_by_id(vm_id)->state()));
  m_dct_vm_controls[vm_id]->m_btn_turn->setText(
        (CVBoxManagerSingleton::Instance()->vm_by_id(vm_id)->state() == VMS_PoweredOff ||
         CVBoxManagerSingleton::Instance()->vm_by_id(vm_id)->state() == VMS_Aborted) ? "Start" : "Stop");
}
////////////////////////////////////////////////////////////////////////////

void VBoxManageWindow::vm_session_state_changed(const com::Bstr &vm_id) {
  UNUSED_ARG(vm_id);
}
////////////////////////////////////////////////////////////////////////////

void VBoxManageWindow::vmc_turn_released(const com::Bstr &vm_id) {
  if (m_dct_vm_controls.find(vm_id) == m_dct_vm_controls.end()) return;

  bool on = CVBoxManagerSingleton::Instance()->vm_by_id(vm_id)->state() == VMS_PoweredOff;

  if (on) {
    m_dct_vm_controls[vm_id]->set_progress_bar_visible(true);
    m_dct_vm_controls[vm_id]->set_progress_bar_value(0);
    int lr = CVBoxManagerSingleton::Instance()->launch_vm(vm_id);
    m_dct_vm_controls[vm_id]->set_progress_bar_visible(false);
    if (!lr) return;

    QMessageBox msg(QMessageBox::Critical,
                    "Launch VM error",
                    QString("Launch VM failed. Error code : %1").arg(lr),
                    QMessageBox::Ok);
    msg.exec();
    return;
  } //turn on

  //turn off
  m_dct_vm_controls[vm_id]->set_progress_bar_visible(true);
  m_dct_vm_controls[vm_id]->set_progress_bar_value(0);
  int tor = CVBoxManagerSingleton::Instance()->turn_off(vm_id, false);
  m_dct_vm_controls[vm_id]->set_progress_bar_visible(false);
  if (!tor) return;

  QMessageBox msg(QMessageBox::Critical,
                  "Shutdown VM error",
                  QString("Shutdown VM failed. Error code : %1").arg(tor),
                  QMessageBox::Ok);
  msg.exec();
}
////////////////////////////////////////////////////////////////////////////

void VBoxManageWindow::vm_launch_progress(const com::Bstr &vm_id, uint32_t percent) {
  if (m_dct_vm_controls.find(vm_id) == m_dct_vm_controls.end()) return;
  m_dct_vm_controls[vm_id]->set_progress_bar_value(percent);
}
////////////////////////////////////////////////////////////////////////////

void VBoxManageWindow::vm_turn_off_progress(const com::Bstr &vm_id, uint32_t percent) {
  if (m_dct_vm_controls.find(vm_id) == m_dct_vm_controls.end()) return;
  m_dct_vm_controls[vm_id]->set_progress_bar_value(percent);
}
////////////////////////////////////////////////////////////////////////////

void VBoxManageWindow::closeEvent(QCloseEvent *event) {
  if (QSystemTrayIcon::isSystemTrayAvailable()) {
    hide();
    event->ignore();
  }
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


const int CVmControl::WIDGETS_COLS[] = {
  CVmControl::COL_NAME, CVmControl::COL_ID, CVmControl::COL_STATE,
  CVmControl::COL_TURN, CVmControl::COL_LAUNCH_PROGRESS
};

CVmControl::CVmControl(const IVirtualMachine *vm) :
  m_lbl_name(NULL),
  m_lbl_id(NULL),
  m_lbl_state(NULL),
  m_btn_turn(NULL),
  m_pb_launch(NULL),
  m_id(vm->id())
{  
  m_lbl_name = new QLabel(QString::fromUtf16((ushort*)vm->name().raw()));
  m_lbl_id = new QLabel(QString::fromUtf16((ushort*)vm->id().raw()));
  m_lbl_state = new QLabel(CCommons::vm_state_to_str(vm->state()));
  m_btn_turn = new QPushButton((vm->state() == VMS_Aborted || vm->state() == VMS_PoweredOff) ? "Start" : "Stop");
  m_pb_launch = new QProgressBar;
  m_pb_launch->setVisible(false);
  m_pb_launch->setValue(0);
  m_pb_launch->setTextVisible(false);

  QWidget* tmp[] = {m_lbl_name, m_lbl_id, m_lbl_state, m_btn_turn, m_pb_launch};
  memcpy(m_widgets, tmp, sizeof(tmp));

  QLabel* labels[] = {m_lbl_id, m_lbl_name, m_lbl_state};
  for (size_t i = 0; i < sizeof(labels) / sizeof(QLabel*); ++i) {
    labels[i]->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
  }

  for (size_t i = 0; i < sizeof(m_widgets) / sizeof(QWidget*); ++i) {
    m_widgets[i]->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  }

  connect(m_btn_turn, SIGNAL(released()), this, SLOT(btn_turn_released()));
}

CVmControl::~CVmControl() {
  for (size_t i = 0; i < sizeof(m_widgets) / sizeof(QWidget*); ++i) {
    if (m_widgets[i]) delete m_widgets[i];
  }
}
////////////////////////////////////////////////////////////////////////////

void CVmControl::set_progress_bar_visible(bool vis) {
  if (m_pb_launch == NULL) return;
  m_pb_launch->setVisible(vis);
}
////////////////////////////////////////////////////////////////////////////

void CVmControl::set_progress_bar_value(uint32_t val) {
  if (m_pb_launch == NULL) return;
  m_pb_launch->setValue((int)val);
}
////////////////////////////////////////////////////////////////////////////

void CVmControl::btn_turn_released() {
  emit btn_turn_released_sig(m_id);
}
////////////////////////////////////////////////////////////////////////////
