#ifndef DLGCSSHX_H
#define DLGCSSHX_H

#include <QDialog>

namespace Ui {
  class DlgCsshx;
}

class DlgCsshx : public QDialog
{
  Q_OBJECT

public:
  explicit DlgCsshx(QWidget *parent = 0);
  ~DlgCsshx();

private:
  Ui::DlgCsshx *ui;
};

#endif // DLGCSSHX_H
