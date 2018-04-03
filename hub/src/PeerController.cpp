#include "PeerController.h"
#include "SettingsManager.h"
#include "SystemCallWrapper.h"
#include <QPushButton>
#include "NotificationObserver.h"
#include "RestContainers.h"
#include "QStandardPaths"


CPeerController::CPeerController(QObject *parent) :
    QObject(parent){
        m_refresh_timer.setInterval(60*1000);
        number_threads = 0;
        connect(&m_refresh_timer, &QTimer::timeout,
              this, &CPeerController::refresh_timer_timeout);
        m_refresh_timer.start();
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
