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
    QStringList bridged_ifs = CPeerController::Instance()->get_bridgedifs();
    if(bridged_ifs.size() == 0){
        CNotificationObserver::Error(tr("Peer manager is not ready yet, try again later"), DlgNotification::N_NO_ACTION);
        this->close();
    }
    ui->cmb_bridge->addItems(bridged_ifs);
    hide_err_labels();
    //slots
    connect(ui->btn_cancel, &QPushButton::clicked, [this]() { this->close(); });
    connect(ui->btn_create, &QPushButton::clicked, this, &DlgCreatePeer::create_button_pressed);

    ui->le_pass->setEchoMode(QLineEdit::Password);
    ui->le_pass_confirm->setEchoMode(QLineEdit::Password);
    ui->le_name->setMaxLength(20);

    // validators
    ui->le_ram->setValidator(new QIntValidator(1, 100000, this));
    ui->le_disk->setValidator(new QIntValidator(1, 100000, this));
    m_invalid_chars.setPattern("\\W");

    this->adjustSize();
}

DlgCreatePeer::~DlgCreatePeer()
{
  delete ui;
}

DlgCreatePeer::pass_err DlgCreatePeer::check_pass(QString pass){
    if(pass.isEmpty())
        return PASS_EMPTY; // empty error;
    if(pass.size() < 7) // too short
        return PASS_SMALL;
    if(pass.contains(m_invalid_chars)) // should contain chars and digits
        return PASS_INVALID;
    return PASS_FINE;
}

void DlgCreatePeer::create_button_pressed(){
    ui->btn_create->setEnabled(false);

    QString name = "subutai-peer_";
    name += ui->le_name->text();
    QString ram = ui->le_ram->text();
    QString cpu = ui->cmb_cpu->currentText();
    QString os = ui->cmb_os->currentText();
    QString disk = ui->le_disk->text();
    QString password1 = ui->le_pass->text();
    QString password2 = ui->le_pass_confirm->text();

    bool errors_exist = false;

    pass_err pass_error = check_pass(password1);
    qDebug() << "password" << pass_error;
    if(pass_error != 3){
        QString error_message = "";
        switch (pass_error) {
        case PASS_EMPTY:
            error_message = tr("Password cannot be empty");
            break;
        case PASS_SMALL:
            error_message = tr("Password size should be more than 7");
            break;
        case PASS_INVALID:
            error_message = tr("Password has invalid symbols");
            break;
        default:
            error_message = tr("Unknown error");
            break;
        }
        ui->lbl_err_pass->setText(error_message);
        ui->lbl_err_pass->setStyleSheet("QLabel {color : red}");
        ui->lbl_err_pass->show();
        ui->btn_create->setEnabled(true);
        errors_exist = true;
    }
    else
    if(password1 != password2){
        ui->lbl_err_pass->setText(tr("Passwords do not match. Please check again"));
        ui->lbl_err_pass->setStyleSheet("QLabel {color : red}");
        ui->lbl_err_pass->show();
        ui->btn_create->setEnabled(true);
        errors_exist = true;
    }
    else ui->lbl_err_pass->hide();

    if(ui->le_name->text().isEmpty()){
        ui->lbl_err_name->setText(tr("Name cannot be empty"));
        ui->lbl_err_name->setStyleSheet("QLabel {color : red}");
        ui->lbl_err_name->show();
        ui->btn_create->setEnabled(true);
        errors_exist = true;
    }
    else
    if(ui->le_name->text().contains(m_invalid_chars) || ui->le_name->text().contains("_")){
            ui->lbl_err_name->setText(tr("You can use only letters and numbers"));
            ui->lbl_err_name->setStyleSheet("QLabel {color : red}");
            ui->lbl_err_name->show();
            ui->btn_create->setEnabled(true);
            errors_exist = true;
    }
    else ui->lbl_err_name->hide();
    if(ram.toInt() < 4096){
        ui->lbl_err_ram->setText(tr("Ram cannot be less than 4096 MB"));
        ui->lbl_err_ram->setStyleSheet("QLabel {color : red}");
        ui->lbl_err_ram->show();
        ui->btn_create->setEnabled(true);
        errors_exist = true;
    }
    else ui->lbl_err_ram->hide();
    if(disk.toInt() < 40){
        ui->lbl_err_disk->setText(tr("Disk cannot be less than 40 GB"));
        ui->lbl_err_disk->setStyleSheet("QLabel {color : red}");
        ui->lbl_err_disk->show();
        ui->btn_create->setEnabled(true);
        errors_exist = true;
    }
    else ui->lbl_err_disk->hide();

    if(errors_exist)
        return;

    QString dir = create_dir(name);

    if(dir.isEmpty()){
        ui->lbl_err_name->setText(tr("Name already exists"));
        ui->lbl_err_name->setStyleSheet("QLabel {color : red}");
        ui->lbl_err_name->show();
        ui->btn_create->setEnabled(true);
        return;
    }

    set_enabled_buttons(false);

    InitPeer *thread_init = new InitPeer(this);
    thread_init->init(dir, os);
    thread_init->startWork();
    connect(thread_init, &InitPeer::outputReceived, [dir, ram, cpu, disk, this](system_call_wrapper_error_t res){
       this->init_completed(res, dir, ram, cpu, disk);
    });
}

void DlgCreatePeer::set_enabled_buttons(bool state){
    ui->le_disk->setEnabled(state);
    ui->le_name->setEnabled(state);
    ui->le_pass->setEnabled(state);
    ui->le_pass_confirm->setEnabled(state);
    ui->le_ram->setEnabled(state);
}

void DlgCreatePeer::hide_err_labels(){
    ui->lbl_err_cpu->hide();
    ui->lbl_err_disk->hide();
    ui->lbl_err_ifs->hide();
    ui->lbl_err_name->hide();
    ui->lbl_err_pass->hide();
    ui->lbl_err_ram->hide();
    ui->lbl_err_os->hide();
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
    static QString vagrant_up_string = "up --provider virtualbox";
    QString peer_name = ui->le_name->text(), peer_pass = ui->le_pass->text();
    CSettingsManager::Instance().set_peer_pass(peer_name, peer_pass);
    res = CSystemCallWrapper::vagrant_command_terminal(dir, vagrant_up_string, ui->le_name->text());
    if(res != SCWE_SUCCESS){
        CNotificationObserver::Instance()->Error("Coudn't start  peer, sorry", DlgNotification::N_NO_ACTION);
        set_enabled_buttons(true);
    }
    else this->close();
}
