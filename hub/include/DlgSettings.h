#ifndef DLGSETTINGS_H
#define DLGSETTINGS_H

#include <QDialog>

namespace Ui {
  class DlgSettings;
}

class DlgSettings : public QDialog
{
  Q_OBJECT

public:
  explicit DlgSettings(QWidget *parent = 0);
  ~DlgSettings();

private:
  Ui::DlgSettings *ui;

private slots:
  void btn_ok_released();
  void btn_cancel_released();
};

#endif // DLGSETTINGS_H
