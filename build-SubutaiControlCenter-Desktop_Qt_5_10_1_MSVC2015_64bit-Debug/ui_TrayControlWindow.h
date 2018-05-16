/********************************************************************************
** Form generated from reading UI file 'TrayControlWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TRAYCONTROLWINDOW_H
#define UI_TRAYCONTROLWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TrayControlWindow
{
public:
    QWidget *centralwidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *TrayControlWindow)
    {
        if (TrayControlWindow->objectName().isEmpty())
            TrayControlWindow->setObjectName(QStringLiteral("TrayControlWindow"));
        TrayControlWindow->resize(114, 63);
        QIcon icon;
        icon.addFile(QStringLiteral(":/hub/Hub-07.png"), QSize(), QIcon::Normal, QIcon::Off);
        TrayControlWindow->setWindowIcon(icon);
        centralwidget = new QWidget(TrayControlWindow);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        TrayControlWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(TrayControlWindow);
        menubar->setObjectName(QStringLiteral("menubar"));
        menubar->setGeometry(QRect(0, 0, 114, 22));
        TrayControlWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(TrayControlWindow);
        statusbar->setObjectName(QStringLiteral("statusbar"));
        TrayControlWindow->setStatusBar(statusbar);

        retranslateUi(TrayControlWindow);

        QMetaObject::connectSlotsByName(TrayControlWindow);
    } // setupUi

    void retranslateUi(QMainWindow *TrayControlWindow)
    {
        TrayControlWindow->setWindowTitle(QApplication::translate("TrayControlWindow", "MainWindow", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TrayControlWindow: public Ui_TrayControlWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TRAYCONTROLWINDOW_H
