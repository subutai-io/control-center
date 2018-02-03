#ifndef DLGTRANSFERFILE_H
#define DLGTRANSFERFILE_H

#include <QDialog>
#include <QDragEnterEvent>
#include <QDebug>
#include <QListWidget>
#include <QListWidget>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDebug>

class CustomListWidget : public QListWidget
{
  Q_OBJECT
public:
  explicit CustomListWidget(QWidget *parent = Q_NULLPTR) :
    QListWidget(parent) {
    setAcceptDrops(true);
  }

 void dragEnterEvent(QDragEnterEvent *event) override {
    qDebug()
        << event->pos().x()
        << event->pos().y();

    setBackgroundRole(QPalette::Highlight);
    if (event->mimeData()->hasUrls()) {
      event->acceptProposedAction();
    }
  }
  void dragMoveEvent(QDragMoveEvent *event) override {

    event->acceptProposedAction();
  }

 void dropEvent(QDropEvent *event) override {
    qDebug() << "I am here maan";

    foreach (const QUrl &url, event->mimeData()->urls()) {
      QString fileName = url.toLocalFile();
      this->addItem(fileName);
      qDebug() << "Dropped file:" << fileName;
    }
  }
};

namespace Ui {
  class DlgTransferFile;
}
class DropFile : public QListWidget
{
public:
  DropFile(QWidget *p) : QListWidget(p){
  setAcceptDrops(true);
  }
  virtual void dragEnterEvent(QDragEnterEvent *event) {
    qDebug() << "DragEnterEvent"
             << event->pos().x()
             << event->pos().y();
  }

};
class DlgTransferFile : public QDialog
{
  Q_OBJECT

public:
  explicit DlgTransferFile(QWidget *parent = 0);
  ~DlgTransferFile();

private:
  void upload_files();

  void select_file();

  void addSSHKey(const QString &key);
  void addIPPort(const QString &ip, const QString &port);
  void addUser(const QString &user);



  Ui::DlgTransferFile *ui;
};

#endif // DLGTRANSFERFILE_H
