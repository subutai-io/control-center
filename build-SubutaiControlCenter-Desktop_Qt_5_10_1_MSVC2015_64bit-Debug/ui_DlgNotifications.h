/********************************************************************************
** Form generated from reading UI file 'DlgNotifications.ui'
**
** Created by: Qt User Interface Compiler version 5.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGNOTIFICATIONS_H
#define UI_DLGNOTIFICATIONS_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DlgNotifications
{
public:
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QTableView *tv_notifications;
    QCheckBox *cb_full_info;

    void setupUi(QDialog *DlgNotifications)
    {
        if (DlgNotifications->objectName().isEmpty())
            DlgNotifications->setObjectName(QStringLiteral("DlgNotifications"));
        DlgNotifications->resize(745, 371);
        verticalLayout = new QVBoxLayout(DlgNotifications);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        tv_notifications = new QTableView(DlgNotifications);
        tv_notifications->setObjectName(QStringLiteral("tv_notifications"));

        gridLayout->addWidget(tv_notifications, 1, 0, 1, 2);

        cb_full_info = new QCheckBox(DlgNotifications);
        cb_full_info->setObjectName(QStringLiteral("cb_full_info"));

        gridLayout->addWidget(cb_full_info, 0, 0, 1, 1);


        verticalLayout->addLayout(gridLayout);


        retranslateUi(DlgNotifications);

        QMetaObject::connectSlotsByName(DlgNotifications);
    } // setupUi

    void retranslateUi(QDialog *DlgNotifications)
    {
        DlgNotifications->setWindowTitle(QApplication::translate("DlgNotifications", "Dialog", nullptr));
        cb_full_info->setText(QApplication::translate("DlgNotifications", "Full info", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DlgNotifications: public Ui_DlgNotifications {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGNOTIFICATIONS_H
