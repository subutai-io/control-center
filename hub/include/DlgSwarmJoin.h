#ifndef DLGSWARMJOIN_H
#define DLGSWARMJOIN_H

#include <QDialog>

namespace Ui {
  class DlgSwarmJoin;
}

class DlgSwarmJoin : public QDialog
{
  Q_OBJECT

public:
  explicit DlgSwarmJoin(QWidget *parent = 0);
  ~DlgSwarmJoin();

private:
  Ui::DlgSwarmJoin *ui;
  bool is_joined_to_swarm(const QString& hash);

private slots:
  void btn_join_released();
  void btn_cancel_released();
};

#endif // DLGSWARMJOIN_H
