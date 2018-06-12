#include "PeerController.h"
#include "SettingsManager.h"
#include "SystemCallWrapper.h"
#include <QPushButton>
#include <QMessageBox>
#include "NotificationObserver.h"
#include "RestContainers.h"
#include "RestWorker.h"
#include "QStandardPaths"


CPeerController::CPeerController(QObject *parent) :
    QObject(parent){}

CPeerController::~CPeerController() {}

void CPeerController::init() {
    m_refresh_timer.setInterval(6*1000); // each 6 seconds update peer list
    m_logs_timer.setInterval(3*1000); // 2 seconds update peer list
    number_threads = 0;
    connect(&m_refresh_timer, &QTimer::timeout,
          this, &CPeerController::refresh_timer_timeout);
    connect(&m_logs_timer, &QTimer::timeout,
            this, &CPeerController::check_logs);
    m_refresh_timer.start();
    m_logs_timer.start();
    refresh();
}

void CPeerController::refresh() {
    if(number_threads != 0)
        return; 
    UpdateVMInformation *update_thread = new UpdateVMInformation(this);
    update_thread->startWork();
    connect(update_thread, &UpdateVMInformation::outputReceived, [this](std::pair<QStringList, system_call_res_t> res){
        this->bridged_interfaces = res.first;
        this->vagrant_global_status = res.second;
        this->search_local();
    });
}

const QString& CPeerController::status_description(const QString &status){
    static std::map<QString, QString> dct_desp = {

        {"aborted", tr ("The Peer is aborted because it was abruptly stopped. "
                        "Press start to resume the virtual machine. "
                        "If you still face the problem, you can destroy and restart it")},

        {"gurumeditation", tr ("The Peer is in the \"guru meditation\" state. This is a rare case which means "
                               "that an internal error in VirtualBox caused the Peer to fail. This is always "
                               "the sign of a bug in VirtualBox. You can try to bring your VM back online "
                               "with a `start`.")},

        {"running", tr ("The Peer is running. You can press `stop` to shut it down")},

        {"poweroff", tr ("The Peer is powered off. To restart the Peer, simply press 'start'")},

        {"not_created", tr ("The VM for the Peer is not created yet. \n"
                            "If you have started the installation process, please wait for it to finish")},

        {"not ready", tr ("Peer’s management is loading, this might take some time.")},

        {"inaccessible", tr ("The Peer is inaccessible! This is a rare case which means that VirtualBox "
                             "can't find your Peer configuration. This usually happens when upgrading "
                             "VirtualBox, moving to a new computer, etc. Please consult VirtualBox "
                             "for how to handle this issue.")},

        {"paused", tr ("The Peer is paused. This Peer may have been paused via the VirtualBox "
                       "GUI or the VBoxManage command line interface. To unpause, please "
                       "use the VirtualBox GUI and/or VBoxManage command line interface so "
                       "that vagrant would be able to control the Peer again.")},

        {"stopping", tr ("The Peer is stopping.")},

        {"saving", tr ("The Peer is currently saving its state. In a few moments this state "
                       "should transition to \"saved.\"")},

        {"saved", tr ("To resume this Peer, simply press 'start'.")},

        {"stuck", tr ("The Peer is \"stuck!\" This is a very rare state which means that "
                      "VirtualBox is unable to recover the current state of the Peer. "
                      "The only known solution to this problem is to restart your "
                      "machine, sorry. ")},

        {"listing", tr ("This environment represents multiple VMs. The VMs are all listed "
                        "above with their current state. For more information about a specific "
                        "VM, run `vagrant status NAME`.")},

        {"undefined", ""}
    };

    if (dct_desp.find(status) == dct_desp.end()){
        return dct_desp["undefined"];
    }
    else{
        return dct_desp[status];
    }
}


void CPeerController::refresh_timer_timeout() {
    refresh();
}

void CPeerController::search_local(){
    //get correct path;
    QStringList stdDirList = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    QDir peers_dir;
    QStringList::iterator stdDir = stdDirList.begin();
    if(stdDir == stdDirList.end())
      peers_dir.setCurrent("/");
    else
      peers_dir.setCurrent(*stdDir);
    peers_dir.mkdir("Subutai-peers");
    peers_dir.cd("Subutai-peers");

    //start looking each subfolder
    for (QFileInfo fi : peers_dir.entryInfoList()) {
        get_peer_info(fi, peers_dir);
    }
    if(number_threads == 0){
         emit got_peer_info(0, "update", "peer", "menu");
    }
}

void CPeerController::check_logs(){
    //get correct path;
    QStringList stdDirList = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    QDir peers_dir;
    QStringList::iterator stdDir = stdDirList.begin();
    if(stdDir == stdDirList.end())
      peers_dir.setCurrent("/");
    else
      peers_dir.setCurrent(*stdDir);
    peers_dir.mkdir("Subutai-peers");
    peers_dir.cd("Subutai-peers");

    //start looking each subfolder
    QStringList file_name;
    QDir peer_dir;
    QString peer_name;
//   / QString error_code;
    bool deleted_flag = false;
    for (QFileInfo fi : peers_dir.entryInfoList()) {
        if(fi.isDir()){
            if(fi.fileName() == "." || fi.fileName() == "..")
                continue;
            peer_dir = peers_dir;
            peer_name = parse_name(fi.fileName());
            if(peer_name.isEmpty())
                continue;
            deleted_flag = false;
            peer_dir.cd(fi.fileName());
            for (QFileInfo logs : peer_dir.entryInfoList()){
                if(logs.isDir())
                    continue;
                file_name = logs.fileName().split('_');
                if(file_name.size() == 2 && file_name[1] == "finished"){
                    QFile log(logs.absoluteFilePath());
                    QString error_message = get_error_messages(peer_dir, file_name[0]);
                    if(error_message.isEmpty()){
                        if(file_name[0] == "destroy"){
                            deleted_flag = true;
                        }
                        CNotificationObserver::Info(tr("Peer %1 has finished to \"%2\" successfully.")
                                                    .arg(peer_name, file_name[0]), DlgNotification::N_NO_ACTION);
                    }
                    else
                        CNotificationObserver::Info(tr("Peer %1 has finished to \"%2\" with following messages:\n %3")
                                                    .arg(peer_name, file_name[0], error_message), DlgNotification::N_NO_ACTION);
                    log.remove();
                }
            }
            if(deleted_flag){
                if(!peer_dir.removeRecursively())
                    CNotificationObserver::Error(tr("Failed to completely clear the peer's path while destroying the peer. "
                                                    "You may manually delete the folder that contains the peer data."), DlgNotification::N_NO_ACTION);
                finish_current_update();
                refresh();
            }
        }
    }
}

QString CPeerController::get_error_messages(QDir peer_dir, QString command){
    qDebug()
            <<"Get error messages of"<<peer_dir<<command;
    QString error_message;
    for (QFileInfo logs : peer_dir.entryInfoList()){
        if(logs.isDir())
            continue;
        QStringList file_name = logs.fileName().split('_');
        if(file_name.size() == 2 && file_name[1] == command){
            QFile log(logs.absoluteFilePath());
            if (log.open(QIODevice::ReadWrite) ){
                QTextStream stream(&log);
                error_message = QString(stream.readAll());
                log.close();
            }
            log.remove();
            break;
        }
    }
    return error_message;
}

void CPeerController::get_peer_info(const QFileInfo &fi, QDir dir){
   if(fi.fileName() == "." || fi.fileName() == "..")
       return;
   if(!fi.isDir())
       return;
   QString peer_name = parse_name(fi.fileName());
   static int status_type = 0;
   if(peer_name=="")
       return;
   dir.cd(fi.fileName());
   // get status of peer
   GetPeerInfo *thread_for_status = new GetPeerInfo(this);
   number_threads++;
   thread_for_status->init(dir.absolutePath(), status_type);
   thread_for_status->startWork();
   connect(thread_for_status, &GetPeerInfo::outputReceived, [dir, peer_name, this](int type, QString res){
      this->parse_peer_info(type, peer_name, dir.absolutePath(), res);
   });
   return;
}

QString CPeerController::parse_name(const QString &name){
    QString peer_name;
    QString prefix;
    bool flag = false;
    for (auto q_char : name){
        if(q_char == '_'){
            if(flag==false){
                if(prefix != "subutai-peer")
                    break;
                flag = true;
                continue;
            }
            else peer_name +=q_char;
        }
        else flag ? peer_name += q_char : prefix += q_char;
    }
    return peer_name;
}

void CPeerController::parse_peer_info(int type, const QString &name, const QString &dir, const QString &output){
    if(number_threads == 0)
        return;
    // get ip of peer
    if(type == 0){
        qDebug() << "Got status of "<<name<<"status:"<<output;
        GetPeerInfo *thread_for_ip = new GetPeerInfo(this);
        if(output != "running"){
            qCritical()<<"not working peer"<<name;
        }
        else{
            int ip_type = 1;
            thread_for_ip->init(dir, ip_type);
            number_threads++;
            thread_for_ip->startWork();
            connect(thread_for_ip, &GetPeerInfo::outputReceived, [dir, name, this](int type, QString res){
               this->parse_peer_info(type, name, dir, res);
            });
        }
    }
    else if(type == 1){
        qDebug() << "Got ip of "<<name<<"ip:"<<output;
        if(!output.isEmpty() && output != "undefined"){
            // get finger
            GetPeerInfo *thread_for_finger = new GetPeerInfo(this);
            int finger_type = 2;
            thread_for_finger->init(output, finger_type);
            number_threads++;
            thread_for_finger->startWork();
            connect(thread_for_finger, &GetPeerInfo::outputReceived, [dir, name, this](int type, QString res){
               this->parse_peer_info(type, name, dir, res);
            });
            // set password
            SetPasswordPeer *thread_for_pass = new SetPasswordPeer(this);
            static QString user_name = "admin";
            thread_for_pass->init(output, user_name,
                                  CSettingsManager::Instance().peer_pass(),
                                  CSettingsManager::Instance().peer_pass(name));
            thread_for_pass->startWork();
            // get update info
            GetPeerInfo *thread_for_update = new GetPeerInfo(this);
            int update_type = 3;
            thread_for_update->init(output, update_type);
            number_threads++;
            thread_for_update->startWork();
            connect(thread_for_update, &GetPeerInfo::outputReceived, [dir, name, this](int type, QString res){
               this->parse_peer_info(type, name, dir, res);
            });
        }
    }
    else if(type == 2){
      qDebug() << "Got finger of "<<name<<"finger:"<<output;
    }
    else if(type == 3){
      qDebug() << "Got update of "<<name<<"update:"<<output;
    }
    static QString undefined_string = "undefined";
    if(output.isEmpty())
        emit got_peer_info(type, name, dir, undefined_string);
    else emit got_peer_info(type, name, dir, output);
    return;
}
