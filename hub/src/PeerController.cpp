#include "PeerController.h"
#include "SettingsManager.h"
#include "SystemCallWrapper.h"
#include <QPushButton>
#include <QMessageBox>
#include "NotificationObserver.h"
#include "RestContainers.h"
#include "QStandardPaths"


CPeerController::CPeerController(QObject *parent) :
    QObject(parent){
        m_refresh_timer.setInterval(6*1000); // each 6 seconds update peer list
        m_logs_timer.setInterval(3*1000); // 2 seconds update peer list
        number_threads = 0;
        connect(&m_refresh_timer, &QTimer::timeout,
              this, &CPeerController::refresh_timer_timeout);
        connect(&m_logs_timer, &QTimer::timeout,
                this, &CPeerController::check_logs);
        m_refresh_timer.start();
        m_logs_timer.start();
}

CPeerController::~CPeerController() {
}

void CPeerController::init() {
    refresh();
}

void CPeerController::refresh() {
    if(number_threads != 0)
        return;
    search_local();
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
                        CNotificationObserver::Info(tr("Peer %1 is finished to \"%2\" succesfully")
                                                    .arg(peer_name, file_name[0]), DlgNotification::N_NO_ACTION);
                    }
                    else
                        CNotificationObserver::Info(tr("Peer %1 is finished to \"%2\" with following error:\n %3")
                                                    .arg(peer_name, file_name[0], error_message), DlgNotification::N_NO_ACTION);
                    log.remove();
                }
            }
            if(deleted_flag){
                if(!peers_dir.removeRecursively())
                    CNotificationObserver::Error(tr("Failed to clean peer path"), DlgNotification::N_NO_ACTION);
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
   number_threads++;
   dir.cd(fi.fileName());
   emit got_peer_info(3, peer_name, dir.absolutePath(), QString(""));
   // get status of peer
   GetPeerInfo *thread_for_status = new GetPeerInfo(this);
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
    // const QString &name, const QString &ip, const QString &fingerprint, const QString &status, const QString &dir
    // wait mazafaka
    // get ip of peer
    if(type == 0){
        qDebug() << "Got status of "<<name<<"status:"<<output;
        GetPeerInfo *thread_for_ip = new GetPeerInfo(this);
        if(output == "poweroff" || output == "broken"){
            qCritical()<<"not working peer"<<name;
        }
        else{
            int ip_type = 1;
            thread_for_ip->init(dir, ip_type);
            thread_for_ip->startWork();
            connect(thread_for_ip, &GetPeerInfo::outputReceived, [dir, name, this](int type, QString res){
               this->parse_peer_info(type, name, dir, res);
            });
        }
    }
    else if(type == 1){
        qDebug() << "Got ip of "<<name<<"ip:"<<output;
        if(!output.isEmpty()){
            GetPeerInfo *thread_for_finger = new GetPeerInfo(this);
            int finger_type = 2;
            thread_for_finger->init(output, finger_type);
            thread_for_finger->startWork();
            connect(thread_for_finger, &GetPeerInfo::outputReceived, [dir, name, this](int type, QString res){
               this->parse_peer_info(type, name, dir, res);
            });
        }
    }
    else if(type == 2){
        qDebug() << "Got finger of "<<name<<"finger:"<<output;
    }
    static QString undefined_string = "undefined";
    if(output.isEmpty())
        emit got_peer_info(type, name, dir, undefined_string);
    else emit got_peer_info(type, name, dir, output);
    return;
}
