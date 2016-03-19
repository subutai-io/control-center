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
  (void)(hash);
//  std::vector<std::string> lst_swarms =
//      CSystemCallWrapper::p2p_swarms_presented();
//  if (lst_swarms.size() == 1 &&
//      lst_swarms[0] == "No instances was foune")
//    return false;
//  std::string std_hash = hash.toStdString();
//  //todo change check
//  bool res = false;
//  for (auto i = lst_swarms.begin(); i != lst_swarms.end(); ++i) {
//    res |= (i->find(std_hash) != std::string::npos);
//  }
//  return res;
  return false; //uncomment above
}
////////////////////////////////////////////////////////////////////////////

void DlgSwarmJoin::btn_join_released()
{
//  QLineEdit* les[] = {ui->le_hash,
//                      ui->le_ip,
//                      ui->le_key,
//                      ui->le_container_ip,
//                      ui->le_ssh_user_name};
//  //check not empty
//  for (size_t i = 0; i < sizeof(les)/sizeof(QLineEdit*); ++i) {
//    if (les[i]->text() == "") {
//      les[i]->setFocus();
//      return;
//    }
//  }

//  //check ip
//  QLineEdit* les_ip[] = {ui->le_ip, ui->le_container_ip};
//  for (size_t i = 0; i < sizeof(les_ip)/sizeof(QLineEdit*); ++i) {
//    QHostAddress addr(les_ip[i]->text());
//    if (addr.isNull()) {
//      les_ip[i]->setFocus();
//      return;
//    }
//  }

//  if (is_joined_to_swarm(ui->le_hash->text())) {
//    QDialog::accept();
//    return;
//  }

//  int lst_join_res =
//      CSystemCallWrapper::join_to_p2p_swarm(ui->le_hash->text().toStdString().c_str(),
//                                            ui->le_key->text().toStdString().c_str(),
//                                            ui->le_ip->text().toStdString().c_str());

//  CSystemCallWrapper::run_ssh_in_terminal(ui->le_ssh_user_name->text().toStdString().c_str(),
//                                          ui->le_container_ip->text().toStdString().c_str());
  QDialog::accept();
}
////////////////////////////////////////////////////////////////////////////

void DlgSwarmJoin::btn_cancel_released()
{
  QDialog::reject();
}
////////////////////////////////////////////////////////////////////////////
