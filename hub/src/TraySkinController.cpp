#include <QCoreApplication>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>


#include "TraySkinController.h"

void TraySkinController::set_tray_skin(TRAY_SKINS current_skin) {
  qDebug() << "Current skin: " << tray_skin_to_str(current_skin);
  qDebug() << "Current skin's stylesheet: " << tray_skin_to_stylesheet(current_skin);
  if (current_skin == tray_skin) return;

  if (current_skin == DEFAULT_SKIN) {
    qApp->setStyleSheet(tray_skin_to_stylesheet(current_skin));
    tray_skin = current_skin;
  }
  else {
    QString stylesheet_file = tray_skin_to_stylesheet(current_skin);
    QString skin_name = tray_skin_to_stylesheet(current_skin);
    try {
      QFile f(stylesheet_file);

      if (!f.exists() || !f.open(QFile::ReadOnly | QFile::Text)) {
        throw 0;
      }

      QTextStream ts(&f);
      qApp->setStyleSheet(ts.readAll());
      tray_skin = current_skin;
    } catch(...) {
      qInfo() << "Couldn't set the skin: " << skin_name;
    }
  }
}
