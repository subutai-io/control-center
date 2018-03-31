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
    m_local_peers.clear();
    number_threads = 0;
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
   if(peer_name=="")
       return;
   number_threads++;
   dir.cd(fi.fileName());
   GetPeerInfo *thread_for = new GetPeerInfo(this);
   thread_for->init(dir.absolutePath());
   thread_for->startWork();
   connect(thread_for, &GetPeerInfo::outputReceived, [dir, peer_name, this](QStringList res){
      this->parse_peer_info(peer_name, dir.absolutePath(), res);
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

void CPeerController::parse_peer_info(const QString &name, const QString &dir, const QStringList &output){
    // const QString &name, const QString &ip, const QString &fingerprint, const QString &status, const QString &dir
    number_threads--;
    static QString empty_string = "";
    static QString running_status = "Running";
    static QString poweroff_status = "Poweroff";
    if(output.size() != 2 || output[0].contains("Try again") || output[1].contains("Try again")){
        m_local_peers.push_back(CLocalPeer(name, empty_string, empty_string, poweroff_status, dir));
    }
    else{
        QString finger = output[1];
        QString ip = output[0];
        if(finger[finger.size()-1] == '\r')
            finger.remove(finger.size()-1, 1);
        if(ip[ip.size()-1] == '\r')
            ip.remove(ip.size()-1, 1);
        CLocalPeer new_peer(name, ip, finger, running_status, dir);
        m_local_peers.push_back(new_peer);
    }
    if(number_threads == 0){
        emit local_peer_list_updated();
    }
}
