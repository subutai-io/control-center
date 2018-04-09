#include "DlgCreatePeer.h"
#include "ui_DlgCreatePeer.h"
#include "SettingsManager.h"
#include "NotificationObserver.h"
#include "TrayControlWindow.h"
#include "DlgNotification.h"
#include "SystemCallWrapper.h"
#include "QDir"
#include "QStandardPaths"

DlgCreatePeer::DlgCreatePeer(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgCreatePeer)
{
    ui->setupUi(this);
    ui->le_disk->setText("100");
    QStringList bridged_ifs = CSystemCallWrapper::list_interfaces();
    ui->cmb_bridge->addItems(bridged_ifs);
    connect(ui->btn_cancel, &QPushButton::clicked, [this]() { this->close(); });
    connect(ui->btn_create, &QPushButton::clicked, this, &DlgCreatePeer::create_button_pressed);
}

DlgCreatePeer::~DlgCreatePeer()
{
  delete ui;
}

void DlgCreatePeer::create_button_pressed(){
    ui->btn_create->setEnabled(false);

    QString name = "subutai-peer_";
    name += ui->le_name->text();
    QString ram = ui->le_ram->text();
    QString cpu = ui->cmb_cpu->currentText();
    QString os = ui->cmb_os->currentText();
    QString disk = ui->le_disk->text();

    QRegExp check_ram("\\d*");

    if(ui->le_name->text().isEmpty()){
        CNotificationObserver::Error(tr("Name can't be empty"), DlgNotification::N_NO_ACTION);
        ui->btn_create->setEnabled(true);
        return;
    }
    if(name.contains(" ")){
        CNotificationObserver::Error(tr("Name can't have space"), DlgNotification::N_NO_ACTION);
        ui->btn_create->setEnabled(true);
        return;
    }
    if(!check_ram.exactMatch(ram)){
        CNotificationObserver::Error(tr("Ram shold be integer"), DlgNotification::N_NO_ACTION);
        ui->btn_create->setEnabled(true);
        return;
    }
    if(ram.isEmpty()){
        CNotificationObserver::Error(tr("Ram can't be empty"), DlgNotification::N_NO_ACTION);
        ui->btn_create->setEnabled(true);
        return;
    }
    if(ram.toInt() < 512 ){
        CNotificationObserver::Error(tr("Ram should be more than 512 MB"), DlgNotification::N_NO_ACTION);
        ui->btn_create->setEnabled(true);
        return;
    }
    if(disk.isEmpty()){
        CNotificationObserver::Error(tr("Disk size can't be empty"), DlgNotification::N_NO_ACTION);
        ui->btn_create->setEnabled(true);
        return;
    }
    if(!check_ram.exactMatch(disk)){
        CNotificationObserver::Error(tr("Disk size should be in integer"), DlgNotification::N_NO_ACTION);
        ui->btn_create->setEnabled(true);
        return;
    }

    if(disk.toInt() < 100){
        CNotificationObserver::Error(tr("Disk size can't be less than 100 GB"), DlgNotification::N_NO_ACTION);
        ui->btn_create->setEnabled(true);
        return;
    }

    QString dir = create_dir(name);

    if(dir.isEmpty()){
        CNotificationObserver::Error(tr("Name already exists"), DlgNotification::N_NO_ACTION);
        ui->btn_create->setEnabled(false);
        return;
    }

    InitPeer *thread_init = new InitPeer(this);
    thread_init->init(dir, os);
    thread_init->startWork();
    connect(thread_init, &InitPeer::outputReceived, [dir, ram, cpu, disk, this](system_call_wrapper_error_t res){
       this->init_completed(res, dir, ram, cpu, disk);
    });
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

void DlgCreatePeer::init_completed(system_call_wrapper_error_t res, QString dir, QString ram, QString cpu, QString disk){
    ui->btn_create->setEnabled(true);
    if(res != SCWE_SUCCESS){
        CNotificationObserver::Instance()->Error("Coudn't create peer, sorry. Check if all software is installed correctly", DlgNotification::N_NO_ACTION);
        return;
    }
    CNotificationObserver::Instance()->Info("Initialization completed. Installing peer... Don't close terminal until instalation is compeleted", DlgNotification::N_NO_ACTION);
    QString filename = QString("%1/vagrant-subutai.yml").arg(dir);
    QFile file(filename);
    if ( file.open(QIODevice::ReadWrite) ){
        QTextStream stream( &file );
        stream << "SUBUTAI_RAM : " << ram << endl;
        stream << "SUBUTAI_CPU : " << cpu << endl;
        QString branch = current_branch_name();
        if(branch == "production")
            stream << "SUBUTAI_ENV : " << "prod" << endl;
        else if (branch == "development")
             stream << "SUBUTAI_ENV : "<< "dev" << endl;
        else stream << "SUBUTAI_ENV : "<< "master" << endl;
        stream << "DISK_SIZE : "<< disk << endl;
        stream << "BRIDGE : "<< QString("\"%1\"").arg(this->ui->cmb_bridge->currentText())<<endl;
    }
    file.close();
    static QString vagrant_up_string = "up";
    res = CSystemCallWrapper::vagrant_command_terminal(dir, vagrant_up_string, ui->le_name->text());
    if(res != SCWE_SUCCESS){
        CNotificationObserver::Instance()->Error("Coudn't start  peer, sorry", DlgNotification::N_NO_ACTION);
    }
}
