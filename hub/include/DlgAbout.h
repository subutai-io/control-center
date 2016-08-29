#ifndef DLGABOUT_H
#define DLGABOUT_H

#include <QDialog>
#include <map>

#include <QProgressBar>
#include <QPushButton>

namespace Ui {
  class DlgAbout;
}

class DlgAboutInitializer : public QObject {
  Q_OBJECT
private:
public:
  static const int COMPONENTS_COUNT = 7;
public slots:
  void do_initialization();
signals:
  void init_progress(int part, int total);
  void finished();
  void got_p2p_version(QString version);
  void got_chrome_version(QString version);
  void got_vbox_version(QString version);
  void got_rh_version(QString version);
  void update_available(QString component_id, bool available);
};
////////////////////////////////////////////////////////////////////////////

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
  void btn_rh_update_released();

  void download_progress(QString file_id, qint64 rec, qint64 total);
  void update_available(QString file_id);
  void update_finished(QString file_id, bool success);  

  void initialization_finished();
  void init_progress_sl(int part, int total);
  void got_p2p_version_sl(QString version);
  void got_chrome_version_sl(QString version);
  void got_vbox_version_sl(QString version);
  void got_rh_version_sl(QString version);
  void update_available_sl(QString component_id, bool available);
};

#endif // DLGABOUT_H
