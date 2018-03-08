#ifndef DLGCREATEPEER_H
#define DLGCREATEPEER_H

#include <QDialog>
#include <QObject>
#include <QDebug>
#include "SystemCallWrapper.h"

#include <QtConcurrent/QtConcurrent>

namespace Ui {
  class DlgCreatePeer;
}

class DlgCreatePeer : public QDialog
{
  Q_OBJECT

public:
  explicit DlgCreatePeer(QWidget *parent = 0);
  ~DlgCreatePeer();
  QString create_dir(const QString &name);
  void init_completed(system_call_wrapper_error_t res, QString dir, QString ram, QString cpu);

private:
  Ui::DlgCreatePeer *ui;

public slots:
  void create_button_pressed();
};

// init peer  executer
class InitPeer : public QObject{
    Q_OBJECT
    QString directory;
    QString OS;
public:
    InitPeer(QObject *parent = nullptr) : QObject(parent){

    }
    void init (const QString &directory, const QString &OS){
        this->directory = directory;
        this->OS = OS;
    }
    void startWork() {
        QThread* thread = new QThread();
        connect(thread, &QThread::started,
                this, &InitPeer::execute_remote_command);
        connect(this, &InitPeer::outputReceived,
                thread, &QThread::quit);
        connect(thread, &QThread::finished,
                this, &InitPeer::deleteLater);
        connect(thread, &QThread::finished,
                thread, &QThread::deleteLater);
        this->moveToThread(thread);
        thread->start();
    }

    void execute_remote_command() {
        QFutureWatcher<system_call_wrapper_error_t> *watcher
            = new QFutureWatcher<system_call_wrapper_error_t>(this);
        QFuture< system_call_wrapper_error_t >  res =
            QtConcurrent::run(CSystemCallWrapper::vagrant_init, directory, OS);
        watcher->setFuture(res);
        connect(watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished, [this, res](){
          emit this->outputReceived(res.result());
        });
    }
signals:
    void outputReceived(system_call_wrapper_error_t res);
};
#endif // DLGCREATEPEER_H
