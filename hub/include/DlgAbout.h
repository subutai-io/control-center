#ifndef DLGABOUT_H
#define DLGABOUT_H

#include <QDialog>
#include <map>

#include <QProgressBar>
#include <QPushButton>

namespace Ui {
  class DlgAbout;
}

class DlgAbout : public QDialog
{
  Q_OBJECT

public:
  explicit DlgAbout(QWidget *parent = 0);
  ~DlgAbout();

private:
  Ui::DlgAbout *ui;
  struct progress_item_t{
    QProgressBar* pb;
    QPushButton* btn;
  };
  std::map<QString, progress_item_t> m_dct_fpb;

private slots:
  void btn_tray_update_released();
  void btn_p2p_update_released();

  void download_progress(QString file_id, qint64 rec, qint64 total);
  void update_available(QString file_id);
  void update_finished(QString file_id, bool success);
};

#endif // DLGABOUT_H
