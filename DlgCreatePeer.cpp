#include "DlgCreatePeer.h"
#include "ui_DlgCreatePeer.h"
#include "SettingsManager.h"
#include "NotificationObserver.h"
#include "TrayControlWindow.h"
#include "DlgNotification.h"
#include "QDir"
#include "QStandardPaths"

DlgCreatePeer::DlgCreatePeer(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgCreatePeer)
{

    ui->setupUi(this);
    connect(ui->btn_create, &QPushButton::clicked, this, &DlgCreatePeer::create_button_pressed);
}

DlgCreatePeer::~DlgCreatePeer()
{
  delete ui;
}

void DlgCreatePeer::create_button_pressed(){
    ui->btn_create->setEnabled(false);

    QString name = ui->le_name->text();
    QString ram = ui->le_ram->text();
    QString cpu = ui->cmb_cpu->currentText();
    QString os = ui->cmb_os->currentText();
    QString dir = create_dir(name);
    CNotificationObserver::Instance()->Info(dir, DlgNotification::N_NO_ACTION);

    if(dir.isEmpty()){
        CNotificationObserver::Error(tr("Name already exists"), DlgNotification::N_NO_ACTION);
    }

    if(name.isEmpty()){
        CNotificationObserver::Error(tr("Name can't be empty"), DlgNotification::N_NO_ACTION);
        ui->btn_create->setEnabled(true);
        return;
    }
    if(name.contains(" ")){
        CNotificationObserver::Error(tr("Name can't have space"), DlgNotification::N_NO_ACTION);
        ui->btn_create->setEnabled(true);
        return;
    }

    if(ram.contains(" ")){
        CNotificationObserver::Error(tr("Ram can't have space"), DlgNotification::N_NO_ACTION);
        ui->btn_create->setEnabled(true);
        return;
    }
    if(ram.isEmpty()){
        CNotificationObserver::Error(tr("Ram can't be empty"), DlgNotification::N_NO_ACTION);
        ui->btn_create->setEnabled(true);
        return;
    }

    system_call_wrapper_error_t res = CSystemCallWrapper::vagrant_init(dir, os);
    bool flag = true;
    do{
        if(res == SCWE_SUCCESS){
            if(flag)
                res = CSystemCallWrapper::run_vagrant_up_in_terminal(dir);
            flag = false;
            continue;
        }
        else{
            CNotificationObserver::Instance()->Error("Failed to create peer. Please if all software is installed correctly",  DlgNotification::N_NO_ACTION);
        }
    }while(0);
    ui->btn_create->setEnabled(true);
}

//for peers, empty if that peer dir exists
QString DlgCreatePeer::create_dir(const QString &name){
    QString new_dir = "";
    QDir current_local_dir;
    QStringList stdDirList = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    QStringList::iterator stdDir = stdDirList.begin();
    if(stdDir == stdDirList.end())
      current_local_dir.setCurrent("/");
    else
      current_local_dir.setCurrent(*stdDir);
    current_local_dir.cd("Subutai-peers");
    if(!current_local_dir.mkdir(name))
        return new_dir;
    current_local_dir.cd(name);
    return current_local_dir.absolutePath();
}
