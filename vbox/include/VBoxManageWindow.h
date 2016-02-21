#ifndef VBOX_MANAGE_WINDOW_H
#define VBOX_MANAGE_WINDOW_H

#include <QMainWindow>
#include <map>
#include <stdint.h>
#include <VBox/com/string.h>

class IVirtualMachine;
class QProgressBar;
class QPushButton;
class QLabel;

namespace Ui {
  class VBoxManageWindow;
}

class CVmControl : public QObject {
  Q_OBJECT

public:
  static const int WIDGETS_COUNT = 5;
  static const int WIDGETS_COLS[WIDGETS_COUNT];

  QLabel *m_lbl_name;
  QLabel *m_lbl_id;
  QLabel *m_lbl_state;
  QPushButton *m_btn_turn;
  QProgressBar *m_pb_launch; //progress bar launch

  explicit CVmControl(const IVirtualMachine* vm);
  ~CVmControl(void);

  static const int COL_NAME = 0;
  static const int COL_ID = 1;
  static const int COL_STATE = 2;
  static const int COL_TURN = 3;
  static const int COL_LAUNCH_PROGRESS = 3;

  void set_progress_bar_visible(bool vis);
  void set_progress_bar_value(uint32_t val);

  QWidget* const* widgets() const {return m_widgets;}

private:
  com::Bstr m_id;
  QWidget* m_widgets[WIDGETS_COUNT];

private slots:
  void btn_turn_released();

signals:
  void btn_turn_released_sig(const com::Bstr& id);
};
////////////////////////////////////////////////////////////////////////////

class VBoxManageWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit VBoxManageWindow(QWidget *parent = 0);
  ~VBoxManageWindow();
  void init_machines();
  void show_err(int code);

private:
  std::map<com::Bstr, CVmControl*> m_dct_vm_controls;
  void add_vm_control(const com::Bstr &vm_id);
  void remove_vm_control(const com::Bstr &vm_id);

  Ui::VBoxManageWindow *ui;

private slots:
  void vm_added(const com::Bstr& vm_id);
  void vm_removed(const com::Bstr& vm_id);
  void vm_state_changed(const com::Bstr& vm_id);
  void vm_session_state_changed(const com::Bstr& vm_id);

  void vmc_turn_released(const com::Bstr& vm_id);
  void vm_launch_progress(const com::Bstr& vm_id, uint32_t percent);
  void vm_turn_off_progress(const com::Bstr& vm_id, uint32_t percent);

  // QWidget interface
protected:
  virtual void closeEvent(QCloseEvent *event);
};

#endif // VBOX_MANAGE_WINDOW_H
