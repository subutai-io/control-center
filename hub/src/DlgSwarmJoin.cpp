#include <QMessageBox>
#include <QHostAddress>
#include "DlgSwarmJoin.h"
#include "ui_DlgSwarmJoin.h"
#include "SystemCallWrapper.h"

DlgSwarmJoin::DlgSwarmJoin(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgSwarmJoin)
{
  ui->setupUi(this);
  connect(ui->btn_join, SIGNAL(released()), this, SLOT(btn_join_released()));
  connect(ui->btn_cancel, SIGNAL(released()), this, SLOT(btn_cancel_released()));
}

DlgSwarmJoin::~DlgSwarmJoin()
{
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

bool DlgSwarmJoin::is_joined_to_swarm(const QString &hash)
{
  std::vector<std::string> lst_swarms =
      CSystemCallWrapper::p2p_swarms_presented();
  if (lst_swarms.size() >= 4 &&
      lst_swarms[0] == "No" &&
      lst_swarms[1] == "instnances" &&
      lst_swarms[2] == "was" &&
      lst_swarms[4] == "found")
    return false;

  std::string std_hash = hash.toStdString();

  //todo change check
  bool res = false;
  for (auto i = lst_swarms.begin(); i != lst_swarms.end(); ++i) {
    res |= (i->find(std_hash) != std::string::npos);
  }
  return res;
}
////////////////////////////////////////////////////////////////////////////

void DlgSwarmJoin::btn_join_released()
{
  QLineEdit* les[] = {ui->le_hash, ui->le_ip, ui->le_key};
  for (size_t i = 0; i < sizeof(les)/sizeof(QLineEdit*); ++i) {
    if (les[i]->text() == "") {
      les[i]->setFocus();
      return;
    }
  }

  QHostAddress addr(ui->le_ip->text());
  if (addr.isNull()) {
    ui->le_ip->setFocus();
    return;
  }

  if (is_joined_to_swarm(ui->le_hash->text())) {
    QDialog::accept();
    return;
  }

  std::vector<std::string> lst_join_res =
      CSystemCallWrapper::join_to_p2p_swarm(ui->le_hash->text().toStdString().c_str(),
                                            ui->le_key->text().toStdString().c_str(),
                                            ui->le_ip->text().toStdString().c_str());

//  CSystemCallWrapper::run_ssh_in_terminal();
  QDialog::accept();
}
////////////////////////////////////////////////////////////////////////////

void DlgSwarmJoin::btn_cancel_released()
{
  QDialog::reject();
}
////////////////////////////////////////////////////////////////////////////
