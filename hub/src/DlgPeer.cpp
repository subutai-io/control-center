#include "DlgPeer.h"
#include "ui_DlgPeer.h"
#include "SystemCallWrapper.h"
#include "DlgRegisterPeer.h"
#include "TrayControlWindow.h"

#include <QGroupBox>
#include <QLayout>
#include <QFormLayout>
#include <QDir>
#include <QLineEdit>
#include "OsBranchConsts.h"
#include "RhController.h"

#include "LibsshController.h"

DlgPeer::DlgPeer(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgPeer){
    qDebug() << "Peer dialog is initialized";
    //ui
    ui->setupUi(this);
    this->setMinimumWidth(this->width());
    this->ui->le_pass->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    ui->show_ssh->setChecked(true);
    ui->gr_ssh->setVisible(true);
    ui->gr_peer_control->setVisible(false);
    ui->show_peer_control->setChecked(false);
    ui->btn_launch_console->setEnabled(false);

    ui->le_name->setReadOnly(true);
    ui->le_cpu->setReadOnly(true);
    ui->le_ram->setReadOnly(true);
    ui->le_disk->setReadOnly(true);

    ui->cmb_bridge->setEnabled(false);
    ui->le_ram->setEnabled(false);
    ui->le_disk->setEnabled(false);
    ui->le_cpu->setEnabled(false);

    ui->le_cpu->setValidator(new QIntValidator(1, 16, this));
    ui->le_ram->setValidator(new QIntValidator(1, 100000, this));
    ui->le_disk->setValidator(new QIntValidator(1, 100000, this));

    QStringList bridges = CPeerController::Instance()->get_bridgedifs();
    ui->cmb_bridge->addItems(bridges);

    //slots
    connect(CRhController::Instance(), &CRhController::ssh_to_rh_finished,
          this, &DlgPeer::ssh_to_rh_finished_sl);
    connect(ui->show_ssh, &QCheckBox::toggled, [this](bool checked){
      this->ui->gr_ssh->setVisible(checked);
      this->adjustSize();
    });
    connect(ui->show_peer_control, &QCheckBox::toggled, [this](bool checked){
      this->ui->gr_peer_control->setVisible(checked);
      this->adjustSize();
    });
    connect(ui->btn_launch_console, &QPushButton::clicked, [this](){
        QString console_address = advanced ? "https://localhost:%1" : "https://%1:8443";
        CHubController::Instance().launch_browser(QString(console_address).arg(this->ssh_ip));
    });
    //vars
    registration_dialog = nullptr;
    ssh_available = false;
    advanced = false;
    hub_available = false;
}

void DlgPeer::addLocalPeer(std::pair<QString, QString> peer) {
    peer_fingerprint = peer.first;
    ssh_ip = peer.second;
    ssh_available = true;
    ui->btn_launch_console->setEnabled(true);
}

void DlgPeer::addHubPeer(CMyPeerInfo peer) {
    hub_available = true;
    connect(ui->btn_peer_on_hub, &QPushButton::clicked, [peer]() {
    CHubController::Instance().launch_peer_page(peer.id().toInt());
    });
    peer_name = peer.name();
    peer_fingerprint = peer.fingerprint();
    ssh_available = true;
    const std::vector<CMyPeerInfo::env_info> envs = peer.peer_environments();
    update_environments(envs);
}

void DlgPeer::addMachinePeer(CLocalPeer peer){
    advanced = true;
    peer_status = peer.status();
    peer_dir = peer.dir();
    peer_name = peer.name();
    peer_update_available = peer.update_available() == "true" ? true : false;
    if(peer.fingerprint() != "loading" || peer.fingerprint() != "undefined")
        peer_fingerprint = peer.fingerprint();
    if(peer.ip() != "undefined" && !peer.ip().isEmpty()){
        ssh_ip = peer.ip();
        ui->label->setText(tr("Host port"));
        ui->btn_launch_console->setEnabled(true);
        ssh_available = true;
    }else{
        ui->btn_launch_console->setEnabled(false);
        if(peer_status == "running")
            peer_status = "not ready";
        else ssh_available = false;
    }
    if(peer_fingerprint == "undefined" || peer_fingerprint == ""){
        ui->btn_launch_console->setEnabled(false);
        if(peer_status == "running")
            peer_status = "not ready";
    }
    if(peer_update_available){
        ui->btn_update_peer->setText(tr("Update is available"));
        ui->btn_update_peer->setEnabled(true);
        ui->btn_update_peer->setToolTip(tr("Update for PeerOS is available"));
    } else {
      if (peer.update_available() == "false"){

      }
    }
    timer_refresh_machine_peer = new QTimer(this);
    timer_refresh_machine_peer->setInterval(7000);
    connect(timer_refresh_machine_peer, &QTimer::timeout, this, [this](){
      std::map<QString, CLocalPeer> local_peers = TrayControlWindow::Instance()->machine_peers_table;
      auto it_peer = local_peers.find(this->peer_name);
      if(it_peer == local_peers.end()){
        this->close();
        return;
      }
      CLocalPeer peer = it_peer->second;
      this->peer_update_available = peer.update_available() == "true" ? true : false;
      if(peer_update_available){
          ui->btn_update_peer->setText("Update is available");
          ui->btn_update_peer->setEnabled(true);
          ui->btn_update_peer->setToolTip(tr("Update for PeerOS is available"));
      }
    });
    timer_refresh_machine_peer->start();
    parse_yml();
}

void DlgPeer::parse_yml(){
    QString filename = QString("%1/vagrant-subutai.yml").arg(peer_dir);
    QFile file(filename);
    if(file.exists()){
        if (file.open(QIODevice::ReadWrite) ){
            QTextStream stream( &file );
            QString output = QString(stream.readAll());
            QStringList vagrant_info = output.split("\n", QString::SkipEmptyParts);
            for (auto s : vagrant_info){
                QString flag, value;
                bool reading_value = false;
                flag = value = "";
                reading_value = false;
                for (int i=0; i < s.size(); i++){
                    if(s[i]=='\r')continue;
                    if(reading_value){
                        value += s[i];
                        continue;
                    }
                    if(s[i] == ':'){
                        flag = value;
                        value = "";
                        continue;
                    }
                    if(s[i] != ' '){
                        if(value == "" && !flag.isEmpty())
                            reading_value = true;
                        value+=s[i];
                    }
                }
                if(flag == "SUBUTAI_RAM")
                    peer_ram = value;
                if(flag == "SUBUTAI_CPU")
                    peer_cpu = value;
                if(flag == "DISK_SIZE")
                    peer_disk = value;
                if(flag == "BRIDGE")
                    peer_bridge = value;
                }
            }
        file.close();
    }
}


void DlgPeer::addPeer(CMyPeerInfo *hub_peer, std::pair<QString, QString> local_peer, std::vector<CLocalPeer> lp) {
    static QPalette *green_text = new QPalette();
    green_text->setColor(QPalette::Text, Qt::green);
    static QPalette *yellow_text = new QPalette();
    yellow_text->setColor(QPalette::Text, Qt::yellow);
    static QPalette *red_text = new QPalette();
    red_text->setColor(QPalette::Text, Qt::red);
    if(hub_peer != nullptr)
        addHubPeer(*hub_peer);
    else{
        ui->btn_peer_on_hub->setEnabled(false);
        hideEnvs();
    }
    if(!local_peer.first.isEmpty())
        addLocalPeer(local_peer);
    if(!lp.empty())
        addMachinePeer(lp[0]);
    if(ssh_available){ // parse SSH information
        if(ssh_ip.isEmpty())
            ssh_ip = CSettingsManager::Instance().rh_host(peer_fingerprint);
        if(ssh_ip.isEmpty())
            ssh_ip = QString("127.0.0.1");
        ssh_port = QString::number(CSettingsManager::Instance().rh_port(peer_fingerprint));
        if(ssh_port.isEmpty() || ssh_port == "0")
            ssh_port = QString("22");
        ssh_user = CSettingsManager::Instance().rh_user(peer_fingerprint);
        if(ssh_user.isEmpty())
            ssh_user = QString("subutai");
        ssh_pass = CSettingsManager::Instance().rh_pass(peer_fingerprint);
        if(ssh_pass.isEmpty())
            ssh_pass = QString("ubuntai");

        ui->le_ip->setText(ssh_ip); ui->le_ip->setReadOnly(true);
        ui->le_port->setText(ssh_port); ui->le_port->setReadOnly(true);
        ui->le_user->setText(ssh_user);
        ui->le_pass->setText(ssh_pass);

        CSettingsManager::Instance().set_rh_host(peer_fingerprint, this->ui->le_ip->text());
        CSettingsManager::Instance().set_rh_port(peer_fingerprint, this->ui->le_port->text().toInt());
        CSettingsManager::Instance().set_rh_user(peer_fingerprint, this->ui->le_user->text());
        CSettingsManager::Instance().set_rh_pass(peer_fingerprint, this->ui->le_pass->text());
        //slots
        if(!advanced)
            connect(ui->btn_ssh_peer, &QPushButton::clicked, [this]() {
                this->ui->btn_ssh_peer->setEnabled(false);
                this->ui->btn_ssh_peer->setText(tr("PROCESSING.."));
                emit this->ssh_to_rh_sig(this->peer_fingerprint);
            });
        else{
            connect(ui->btn_ssh_peer, &QPushButton::clicked, this, &DlgPeer::sshPeer);
        }
    }
    else
        hideSSH();
    if(advanced){
        ui->gr_peer_control->setTitle(tr("Peer is in your machine."));
        hub_available ?
            ui->btn_register->hide() : ui->btn_unregister->hide();
        ui->le_name->setText(peer_name);
        if(peer_status == "running"){
            ui->btn_start->hide();
            ui->le_status->setText(tr("Peer is running"));
        }
        else if(peer_status == "not ready"){
                ui->btn_start->hide();
                ui->btn_unregister->setEnabled(false);
                ui->btn_register->setEnabled(false);
                ui->le_status->setText(tr("Peer is not ready."));
        }
        else if(peer_status == "not_created"){
                ui->btn_stop->hide();
                ui->btn_start->setEnabled(false);
                ui->btn_reload->setEnabled(false);
                ui->btn_register->setEnabled(false);
                ui->btn_unregister->setEnabled(false);
                ui->le_status->setText(tr("Peer is not created"));
        }
        else{
            ui->btn_stop->hide();
            ui->btn_register->setEnabled(false);
            ui->btn_unregister->setEnabled(false);
            ui->btn_reload->setEnabled(false);
            ui->le_status->setText(tr("Peer is ") + peer_status);
        }
        ui->le_status->setToolTip(CPeerController::Instance()->status_description(peer_status));
        configs();

        connect(ui->btn_destroy, &QPushButton::clicked, [this](){this->destroyPeer();});
        connect(ui->btn_start, &QPushButton::clicked, [this](){this->startPeer();});
        connect(ui->btn_stop, &QPushButton::clicked, [this](){this->stopPeer();});
        connect(ui->btn_reload, &QPushButton::clicked, [this](){this->reloadPeer();});
        connect(ui->btn_register, &QPushButton::clicked, this, &DlgPeer::registerPeer);
        connect(ui->btn_unregister, &QPushButton::clicked, this, &DlgPeer::unregisterPeer);
        connect(ui->btn_update_peer, &QPushButton::clicked, this, &DlgPeer::updatePeer);
        connect(ui->change_confugre, &QCheckBox::toggled, [this](bool checked){
            ui->le_cpu->setReadOnly(!checked);
            ui->le_ram->setReadOnly(!checked);
            ui->le_disk->setReadOnly(!checked);

            ui->cmb_bridge->setEnabled(checked);
            ui->le_disk->setEnabled(checked);
            ui->le_cpu->setEnabled(checked);
            ui->le_ram->setEnabled(checked);
            if(checked == false){
                this->configs();
            }
            static std::vector<QString> states = {tr("Locked configs"), tr("Unlocked configs")};
            ui->change_confugre->setText(states[checked]);
        });
        ui->show_peer_control->toggle();
        this->adjustSize();
    }
    else hidePeer();
}

void DlgPeer::configs(){
    ui->le_cpu->setText(peer_cpu);
    ui->le_ram->setText(peer_ram);
    ui->le_disk->setText(peer_disk);
    QStringList bridges = CSystemCallWrapper::list_interfaces();
    int index_bridge = -1;
    for(int i=0; i < bridges.size(); i++){
        QString s = bridges[i];
        s += "\"";
        s = "\"" + s;
        if (s == peer_bridge){
            index_bridge = i;
            break;
        }
    }
    if(index_bridge >= 0)
        ui->cmb_bridge->setCurrentIndex(index_bridge);
}

bool DlgPeer::check_configs(){
    static bool bool_me;
    static int base = 10;
    int ram = ui->le_ram->text().toInt(&bool_me, base);
    int cpu = ui->le_cpu->text().toInt(&bool_me, base);
    int disk = ui->le_disk->text().toInt(&bool_me, base);
    if(ram < 2048){
        CNotificationObserver::Error(tr("2048 GB is the minimum size for RAM. Increase your RAM size please"), DlgNotification::N_NO_ACTION);
        return false;
    }

    if(cpu < 1){
        CNotificationObserver::Error(tr("Quantity of CPU can't be less than 1"), DlgNotification::N_NO_ACTION);
        return false;
    }

    if(disk < peer_disk.toInt(&bool_me, base)){
        CNotificationObserver::Error(tr("Sorry you can only increase disk size"), DlgNotification::N_NO_ACTION);
        return false;
    }
    return true;
}

bool DlgPeer::change_configs(){
    QString filename = QString("%1/vagrant-subutai.yml").arg(peer_dir);
    QFile file(filename);
    if(!check_configs())
        return false;
    peer_ram = ui->le_ram->text();
    peer_cpu = ui->le_cpu->text();
    peer_disk = ui->le_disk->text();
    peer_bridge = ui->cmb_bridge->currentText();
    if(file.exists()){
        if (file.remove() && file.open(QIODevice::ReadWrite)){
            QTextStream stream(&file);
            stream << "SUBUTAI_RAM : " << peer_ram << endl;
            stream << "SUBUTAI_CPU : " << peer_cpu << endl;
            QString branch = current_branch_name();
            if(branch == "production")
                stream << "SUBUTAI_ENV : " << "prod" << endl;
            else if (branch == "development")
                 stream << "SUBUTAI_ENV : "<< "dev" << endl;
            else stream << "SUBUTAI_ENV : "<< "master" << endl;
            stream << "DISK_SIZE : "<< peer_disk << endl;
            stream << "BRIDGE : "<< QString("\"%1\"").arg(peer_bridge)<<endl;
        }
        else{
            CNotificationObserver::Error(tr("Failed to create new configuration file"), DlgNotification::N_NO_ACTION);
            return false;
        }
        file.close();
    }
    else{
        CNotificationObserver::Error(tr("Configuration file was not set"), DlgNotification::N_NO_ACTION);
        return false;
    }
    return true;
}

void DlgPeer::enabled_peer_buttons(bool state){
    if(peer_status == "running"){
        ui->btn_stop->setEnabled(state);
        ui->btn_reload->setEnabled(state);
        if(hub_available){
            ui->btn_unregister->setEnabled(state);
        }
        else ui->btn_register->setEnabled(state);
    }
    else if(peer_status == "not ready"){
            ui->btn_stop->setEnabled(state);
            ui->btn_reload->setEnabled(state);
    }
    else {
        ui->btn_start->setEnabled(state);
    }
}


void DlgPeer::ssh_to_rh_finished_sl(const QString &peer_fingerprint, system_call_wrapper_error_t res, int libbssh_exit_code) {
  qDebug()<<"ssh to rh finished";
  UNUSED_ARG(res);
  UNUSED_ARG(libbssh_exit_code); // need to use this variables to give feedback to user
  if (QString::compare(peer_fingerprint, peer_fingerprint, Qt::CaseInsensitive) != 0)
      return;

  ui->btn_ssh_peer->setEnabled(true);
  ui->btn_ssh_peer->setText(tr("SSH into Peer"));
}

void DlgPeer::registerPeer(){
    ui->btn_register->setEnabled(false);
    DlgRegisterPeer* dlg_register = new DlgRegisterPeer(this);
    const QString ip_addr=ui->le_ip->text();
    dlg_register->init(ip_addr, peer_name);
    dlg_register->setRegistrationMode();
    dlg_register->setWindowTitle(tr("Register peer: %1").arg(peer_name));
    dlg_register->show();
    registration_dialog = dlg_register;
    connect (dlg_register, &QDialog::finished, this, &DlgPeer::regDlgClosed);
    connect (dlg_register, &DlgRegisterPeer::register_finished, [this](){this->close();});
}

void DlgPeer::unregisterPeer(){
    if(envs_available){
        CNotificationObserver::Error(tr("You can't unregister Peer until it has environments"), DlgNotification::N_GO_TO_HUB);
        return;
    }
    ui->btn_unregister->setEnabled(false);
    DlgRegisterPeer* dlg_unregister = new DlgRegisterPeer(this);
    const QString ip_addr=ui->le_ip->text();
    dlg_unregister->init(ip_addr, peer_name);
    dlg_unregister->setUnregistrationMode();
    dlg_unregister->setWindowTitle(tr("Unregister peer: %1").arg(peer_name));
    dlg_unregister->show();
    registration_dialog = dlg_unregister;
    connect (dlg_unregister, &QDialog::finished, this, &DlgPeer::regDlgClosed);
    connect (dlg_unregister, &DlgRegisterPeer::register_finished, [this](){this->close();});
}

void DlgPeer::regDlgClosed(){
    ui->btn_register->setEnabled(true);
    ui->btn_unregister->setEnabled(true );
    if(registration_dialog == nullptr)
        return;
    registration_dialog->deleteLater();
}

void DlgPeer::hideSSH(){
    ui->label->hide();
    ui->label_3->hide();
    ui->label_2->hide();
    ui->label_4->hide();
    ui->le_ip->hide();
    ui->le_port->hide();
    ui->le_user->hide();
    ui->le_pass->hide();
    ui->btn_ssh_peer->hide();
    ui->show_ssh->toggle(); //lifehack
    ui->show_ssh->toggle();
    this->adjustSize();
}

void DlgPeer::hidePeer(){
    ui->lbl_name->hide(); ui->le_name->hide();
    ui->lbl_status->hide(); ui->le_status->hide();
    ui->lbl_bridge->hide(); ui->cmb_bridge->hide();
    ui->lbl_cpu->hide(); ui->le_cpu->hide();
    ui->lbl_disk->hide(); ui->le_disk->hide();
    ui->lbl_ram->hide(); ui->le_ram->hide();

    //buttons
    ui->btn_start->hide();
    ui->btn_stop->hide();
    ui->change_confugre->hide();
    ui->btn_reload->hide();
    ui->btn_destroy->hide();
    ui->btn_register->hide();
    ui->btn_unregister->hide();
    ui->btn_update_peer->hide();

    ui->show_peer_control->toggle(); //lifehack :D
    ui->show_peer_control->toggle();

    ui->gr_peer_control->setTitle(tr("This peer is not in your machine"));
    this->adjustSize();
}

void DlgPeer::hideEnvs(){
    ui->gr_ssh->setTitle(tr("This peer doesn't have any \"Environments\""));
    ui->lbl_env->hide();
    ui->lbl_env_owner->hide();
    ui->lbl_env_status->hide();
    ui->line_1->hide();
    ui->line_2->hide();
    ui->line_3->hide();
    this->adjustSize();
}

void DlgPeer::stopPeer(){
    enabled_peer_buttons(false);
    static QString stop_command = "halt";
    CommandPeerTerminal *thread_init = new CommandPeerTerminal(this);
    ui->btn_stop->setText(tr("Trying to stop peer..."));
    thread_init->init(peer_dir, stop_command, peer_name);
    emit peer_modified(peer_name);
    thread_init->startWork();
    connect(thread_init, &CommandPeerTerminal::outputReceived, [this](system_call_wrapper_error_t res){
        if(res == SCWE_SUCCESS){
            CNotificationObserver::Instance()->Info(tr("Process to stop peer %1 started. Don't close terminal until it's finished").arg(this->ui->le_name->text()), DlgNotification::N_NO_ACTION);
            this->close();
        }
        else{
            CNotificationObserver::Instance()->Error(tr("Sorry, could not stop peer \"%1\"").arg(this->ui->le_name->text()), DlgNotification::N_NO_ACTION);
            enabled_peer_buttons(true);
            ui->btn_stop->setText(tr("Stop"));
        }
    });
}

void DlgPeer::startPeer(){
    static QString up_command = "up";
    enabled_peer_buttons(false);
    if(ui->change_confugre->isChecked()){
        if(!change_configs()){
            enabled_peer_buttons(true);
            return;
        }
    }
    CommandPeerTerminal *thread_init = new CommandPeerTerminal(this);
    ui->btn_stop->setText(tr("Trying to launch peer..."));
    thread_init->init(peer_dir, up_command, peer_name);
    thread_init->startWork();
    connect(thread_init, &CommandPeerTerminal::outputReceived, [this](system_call_wrapper_error_t res){
        if(res == SCWE_SUCCESS){
            emit peer_modified(this->peer_name);
            CNotificationObserver::Instance()->Info(tr("The process has started. Do not close terminal before it is finished"), DlgNotification::N_NO_ACTION);
            this->close();
        }
        else{
            CNotificationObserver::Instance()->Error(tr("Sorry, could not stop peer \"%1\"").arg(this->ui->le_name->text()), DlgNotification::N_NO_ACTION);
             enabled_peer_buttons(true);
            ui->btn_stop->setText(tr("Start"));
        }
    });
}

void DlgPeer::sshPeer(){
    static QString up_command = "ssh";
    enabled_peer_buttons(false);
    CommandPeerTerminal *thread_init = new CommandPeerTerminal(this);
    ui->btn_ssh_peer->setText(tr("Processing..."));
    thread_init->init(peer_dir, up_command, peer_name);
    thread_init->startWork();
    connect(thread_init, &CommandPeerTerminal::outputReceived, [this](system_call_wrapper_error_t res){
        if(res == SCWE_SUCCESS){
            qDebug()<<"sshed to peer"<<peer_name;
        }
        else{
            CNotificationObserver::Instance()->Error(tr("Sorry, could not SSH to peer \"%1\"").arg(this->ui->le_name->text()), DlgNotification::N_NO_ACTION);
        }
        enabled_peer_buttons(true);
        ui->btn_ssh_peer->setText(tr("SSH into Peer"));
    });
}

void DlgPeer::destroyPeer(){
    if(hub_available){
        CNotificationObserver::Error(tr("You can't destroy the Peer until it's registered to the Bazaar. "
                                        "Please run 'unregister' to unregister it from the Bazaar"), DlgNotification::N_NO_ACTION);
        return;
    }
    enabled_peer_buttons(false);
    if(peer_status == "not_created"){
        QDir  del_me(peer_dir);
        if(del_me.removeRecursively()){
            CNotificationObserver::Instance()->Info(tr("Peer has been destroyed."), DlgNotification::N_NO_ACTION);
            this->close();
        }
        else{
            CNotificationObserver::Instance()->Error(tr("Failed to delete peer directory. Make sure you have permission"), DlgNotification::N_NO_ACTION);
            enabled_peer_buttons(true);
        }
        return;
    }
    static QString delete_command = "destroy -f";
    CommandPeerTerminal *thread_init = new CommandPeerTerminal(this);
    ui->btn_destroy->setText(tr("Trying to destroy peer..."));
    thread_init->init(peer_dir, delete_command, peer_name);
    thread_init->startWork();
    connect(thread_init, &CommandPeerTerminal::outputReceived, [this](system_call_wrapper_error_t res){
        if(res == SCWE_SUCCESS){
            CNotificationObserver::Instance()->Info(tr("Process to destroy peer %1 has started").arg(peer_name), DlgNotification::N_NO_ACTION);
       //     emit peer_deleted(this->peer_name);
            this->close();
        }
        else{
            CNotificationObserver::Instance()->Error(tr("Sorry, could not destroy peer \"%1\"").arg(peer_name), DlgNotification::N_NO_ACTION);
            enabled_peer_buttons(true);
            ui->btn_destroy->setText(tr("Destroy"));
        }
    });
    return;
}

void DlgPeer::reloadPeer(){
    static QString reload_command = "reload";
    enabled_peer_buttons(false);
    if(ui->change_confugre->isChecked()){
        if(!change_configs()){
            enabled_peer_buttons(true);
            return;
        }
    }
    CommandPeerTerminal *thread_init = new CommandPeerTerminal(this);
    ui->btn_reload->setText(tr("Trying to reload peer..."));
    thread_init->init(peer_dir, reload_command, peer_name);
    thread_init->startWork();
    emit peer_modified(this->peer_name);
    connect(thread_init, &CommandPeerTerminal::outputReceived, [this](system_call_wrapper_error_t res){
        if(res == SCWE_SUCCESS){
            CNotificationObserver::Instance()->Info(tr("Peer reloading has started. Don't close terminal until it is finished"), DlgNotification::N_NO_ACTION);
            this->close();
        }
        else{
            CNotificationObserver::Instance()->Error(tr("Sorry, could not reload peer \"%1\"").arg(this->ui->le_name->text()), DlgNotification::N_NO_ACTION);
            enabled_peer_buttons(true);
            ui->btn_stop->setText(tr("Reload"));
        }
    });
}

void DlgPeer::updatePeer(){
  emit this->peer_update_peeros(peer_fingerprint);
}

void DlgPeer::update_environments(const std::vector<CMyPeerInfo::env_info> envs){
    if (!envs.empty()) {
      for (CMyPeerInfo::env_info env : envs) {
        QLabel *env_name = new QLabel(env.envName);
        QLabel *env_owner = new QLabel(QString("<a href=\"%1\">%2</a>")
                                       .arg(hub_user_profile_url() + QString::number(env.ownerId)).arg(env.ownerName));
        QLabel *env_status = new QLabel(env.status);

        env_name->setAlignment(Qt::AlignHCenter);
        env_name->setTextInteractionFlags(Qt::TextSelectableByMouse);

        env_owner->setAlignment(Qt::AlignHCenter);
        env_owner->setTextFormat(Qt::RichText);
        env_owner->setTextInteractionFlags(Qt::TextBrowserInteraction);
        env_owner->setOpenExternalLinks(true);

        env_status->setAlignment(Qt::AlignHCenter);
        env_status->setTextInteractionFlags(Qt::TextSelectableByMouse);

        ui->env_name->addWidget(env_name);
        ui->env_owner->addWidget(env_owner);
        ui->env_status->addWidget(env_status);
      }
      envs_available = true;
      this->adjustSize();
    }
    else{
        envs_available = false;
        hideEnvs();
    }
}

DlgPeer::~DlgPeer()
{
  qDebug() << "Deleting DlgPeer";
  if (advanced && timer_refresh_machine_peer != nullptr)
    timer_refresh_machine_peer->deleteLater();
  delete ui;
}
