/********************************************************************************
** Form generated from reading UI file 'DlgCreatePeer.ui'
**
** Created by: Qt User Interface Compiler version 5.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGCREATEPEER_H
#define UI_DLGCREATEPEER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_DlgCreatePeer
{
public:
    QGridLayout *gridLayout;
    QLabel *lbl_pass_confirm;
    QLabel *lbl_cpu;
    QPushButton *btn_cancel;
    QComboBox *cmb_cpu;
    QLabel *lbl_err_disk;
    QLabel *lbl_ram;
    QLabel *lbl_pass;
    QLabel *lbl_bridge;
    QLabel *lbl_err_ram;
    QLineEdit *le_name;
    QLineEdit *le_ram;
    QLineEdit *le_pass;
    QLineEdit *le_pass_confirm;
    QLabel *lbl_os;
    QLabel *lbl_err_pass;
    QLabel *lbl_disk;
    QLabel *lbl_err_os;
    QLabel *lbl_err_name;
    QLineEdit *le_disk;
    QComboBox *cmb_os;
    QPushButton *btn_create;
    QComboBox *cmb_bridge;
    QLabel *lbl_err_cpu;
    QLabel *lbl_name;
    QLabel *lbl_err_ifs;

    void setupUi(QDialog *DlgCreatePeer)
    {
        if (DlgCreatePeer->objectName().isEmpty())
            DlgCreatePeer->setObjectName(QStringLiteral("DlgCreatePeer"));
        DlgCreatePeer->resize(358, 475);
        gridLayout = new QGridLayout(DlgCreatePeer);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        lbl_pass_confirm = new QLabel(DlgCreatePeer);
        lbl_pass_confirm->setObjectName(QStringLiteral("lbl_pass_confirm"));

        gridLayout->addWidget(lbl_pass_confirm, 3, 0, 1, 1);

        lbl_cpu = new QLabel(DlgCreatePeer);
        lbl_cpu->setObjectName(QStringLiteral("lbl_cpu"));

        gridLayout->addWidget(lbl_cpu, 11, 0, 1, 1);

        btn_cancel = new QPushButton(DlgCreatePeer);
        btn_cancel->setObjectName(QStringLiteral("btn_cancel"));
        btn_cancel->setFocusPolicy(Qt::NoFocus);

        gridLayout->addWidget(btn_cancel, 15, 0, 1, 1);

        cmb_cpu = new QComboBox(DlgCreatePeer);
        cmb_cpu->addItem(QString());
        cmb_cpu->addItem(QString());
        cmb_cpu->addItem(QString());
        cmb_cpu->addItem(QString());
        cmb_cpu->addItem(QString());
        cmb_cpu->addItem(QString());
        cmb_cpu->addItem(QString());
        cmb_cpu->addItem(QString());
        cmb_cpu->setObjectName(QStringLiteral("cmb_cpu"));

        gridLayout->addWidget(cmb_cpu, 11, 1, 1, 1);

        lbl_err_disk = new QLabel(DlgCreatePeer);
        lbl_err_disk->setObjectName(QStringLiteral("lbl_err_disk"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(lbl_err_disk->sizePolicy().hasHeightForWidth());
        lbl_err_disk->setSizePolicy(sizePolicy);
        lbl_err_disk->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(lbl_err_disk, 8, 0, 1, 2);

        lbl_ram = new QLabel(DlgCreatePeer);
        lbl_ram->setObjectName(QStringLiteral("lbl_ram"));

        gridLayout->addWidget(lbl_ram, 5, 0, 1, 1);

        lbl_pass = new QLabel(DlgCreatePeer);
        lbl_pass->setObjectName(QStringLiteral("lbl_pass"));

        gridLayout->addWidget(lbl_pass, 2, 0, 1, 1);

        lbl_bridge = new QLabel(DlgCreatePeer);
        lbl_bridge->setObjectName(QStringLiteral("lbl_bridge"));

        gridLayout->addWidget(lbl_bridge, 9, 0, 1, 1);

        lbl_err_ram = new QLabel(DlgCreatePeer);
        lbl_err_ram->setObjectName(QStringLiteral("lbl_err_ram"));
        sizePolicy.setHeightForWidth(lbl_err_ram->sizePolicy().hasHeightForWidth());
        lbl_err_ram->setSizePolicy(sizePolicy);
        lbl_err_ram->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(lbl_err_ram, 6, 0, 1, 2);

        le_name = new QLineEdit(DlgCreatePeer);
        le_name->setObjectName(QStringLiteral("le_name"));

        gridLayout->addWidget(le_name, 0, 1, 1, 1);

        le_ram = new QLineEdit(DlgCreatePeer);
        le_ram->setObjectName(QStringLiteral("le_ram"));

        gridLayout->addWidget(le_ram, 5, 1, 1, 1);

        le_pass = new QLineEdit(DlgCreatePeer);
        le_pass->setObjectName(QStringLiteral("le_pass"));

        gridLayout->addWidget(le_pass, 2, 1, 1, 1);

        le_pass_confirm = new QLineEdit(DlgCreatePeer);
        le_pass_confirm->setObjectName(QStringLiteral("le_pass_confirm"));

        gridLayout->addWidget(le_pass_confirm, 3, 1, 1, 1);

        lbl_os = new QLabel(DlgCreatePeer);
        lbl_os->setObjectName(QStringLiteral("lbl_os"));

        gridLayout->addWidget(lbl_os, 13, 0, 1, 1);

        lbl_err_pass = new QLabel(DlgCreatePeer);
        lbl_err_pass->setObjectName(QStringLiteral("lbl_err_pass"));
        sizePolicy.setHeightForWidth(lbl_err_pass->sizePolicy().hasHeightForWidth());
        lbl_err_pass->setSizePolicy(sizePolicy);
        lbl_err_pass->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(lbl_err_pass, 4, 0, 1, 2);

        lbl_disk = new QLabel(DlgCreatePeer);
        lbl_disk->setObjectName(QStringLiteral("lbl_disk"));

        gridLayout->addWidget(lbl_disk, 7, 0, 1, 1);

        lbl_err_os = new QLabel(DlgCreatePeer);
        lbl_err_os->setObjectName(QStringLiteral("lbl_err_os"));
        sizePolicy.setHeightForWidth(lbl_err_os->sizePolicy().hasHeightForWidth());
        lbl_err_os->setSizePolicy(sizePolicy);
        lbl_err_os->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(lbl_err_os, 14, 0, 1, 2);

        lbl_err_name = new QLabel(DlgCreatePeer);
        lbl_err_name->setObjectName(QStringLiteral("lbl_err_name"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(lbl_err_name->sizePolicy().hasHeightForWidth());
        lbl_err_name->setSizePolicy(sizePolicy1);
        lbl_err_name->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(lbl_err_name, 1, 0, 1, 2);

        le_disk = new QLineEdit(DlgCreatePeer);
        le_disk->setObjectName(QStringLiteral("le_disk"));

        gridLayout->addWidget(le_disk, 7, 1, 1, 1);

        cmb_os = new QComboBox(DlgCreatePeer);
        cmb_os->addItem(QString());
        cmb_os->addItem(QString());
        cmb_os->setObjectName(QStringLiteral("cmb_os"));

        gridLayout->addWidget(cmb_os, 13, 1, 1, 1);

        btn_create = new QPushButton(DlgCreatePeer);
        btn_create->setObjectName(QStringLiteral("btn_create"));

        gridLayout->addWidget(btn_create, 15, 1, 1, 1);

        cmb_bridge = new QComboBox(DlgCreatePeer);
        cmb_bridge->setObjectName(QStringLiteral("cmb_bridge"));
        sizePolicy1.setHeightForWidth(cmb_bridge->sizePolicy().hasHeightForWidth());
        cmb_bridge->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(cmb_bridge, 9, 1, 1, 1);

        lbl_err_cpu = new QLabel(DlgCreatePeer);
        lbl_err_cpu->setObjectName(QStringLiteral("lbl_err_cpu"));
        sizePolicy.setHeightForWidth(lbl_err_cpu->sizePolicy().hasHeightForWidth());
        lbl_err_cpu->setSizePolicy(sizePolicy);
        lbl_err_cpu->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(lbl_err_cpu, 12, 0, 1, 2);

        lbl_name = new QLabel(DlgCreatePeer);
        lbl_name->setObjectName(QStringLiteral("lbl_name"));

        gridLayout->addWidget(lbl_name, 0, 0, 1, 1);

        lbl_err_ifs = new QLabel(DlgCreatePeer);
        lbl_err_ifs->setObjectName(QStringLiteral("lbl_err_ifs"));
        sizePolicy.setHeightForWidth(lbl_err_ifs->sizePolicy().hasHeightForWidth());
        lbl_err_ifs->setSizePolicy(sizePolicy);
        lbl_err_ifs->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(lbl_err_ifs, 10, 0, 1, 2);

        QWidget::setTabOrder(le_name, le_pass);
        QWidget::setTabOrder(le_pass, le_pass_confirm);
        QWidget::setTabOrder(le_pass_confirm, le_ram);
        QWidget::setTabOrder(le_ram, le_disk);
        QWidget::setTabOrder(le_disk, cmb_bridge);
        QWidget::setTabOrder(cmb_bridge, cmb_cpu);
        QWidget::setTabOrder(cmb_cpu, cmb_os);
        QWidget::setTabOrder(cmb_os, btn_create);

        retranslateUi(DlgCreatePeer);

        cmb_cpu->setCurrentIndex(0);
        btn_create->setDefault(true);


        QMetaObject::connectSlotsByName(DlgCreatePeer);
    } // setupUi

    void retranslateUi(QDialog *DlgCreatePeer)
    {
        DlgCreatePeer->setWindowTitle(QApplication::translate("DlgCreatePeer", "Dialog", nullptr));
        lbl_pass_confirm->setText(QApplication::translate("DlgCreatePeer", "Confirm password", nullptr));
        lbl_cpu->setText(QApplication::translate("DlgCreatePeer", "CPU", nullptr));
        btn_cancel->setText(QApplication::translate("DlgCreatePeer", "Cancel", nullptr));
        cmb_cpu->setItemText(0, QApplication::translate("DlgCreatePeer", "1", nullptr));
        cmb_cpu->setItemText(1, QApplication::translate("DlgCreatePeer", "2", nullptr));
        cmb_cpu->setItemText(2, QApplication::translate("DlgCreatePeer", "3", nullptr));
        cmb_cpu->setItemText(3, QApplication::translate("DlgCreatePeer", "4", nullptr));
        cmb_cpu->setItemText(4, QApplication::translate("DlgCreatePeer", "5", nullptr));
        cmb_cpu->setItemText(5, QApplication::translate("DlgCreatePeer", "6", nullptr));
        cmb_cpu->setItemText(6, QApplication::translate("DlgCreatePeer", "7", nullptr));
        cmb_cpu->setItemText(7, QApplication::translate("DlgCreatePeer", "8", nullptr));

        cmb_cpu->setCurrentText(QApplication::translate("DlgCreatePeer", "1", nullptr));
        lbl_err_disk->setText(QString());
        lbl_ram->setText(QApplication::translate("DlgCreatePeer", "RAM MB", nullptr));
        lbl_pass->setText(QApplication::translate("DlgCreatePeer", "Set password", nullptr));
        lbl_bridge->setText(QApplication::translate("DlgCreatePeer", "Bridge Interface", nullptr));
        lbl_err_ram->setText(QString());
        le_ram->setText(QApplication::translate("DlgCreatePeer", "4096", nullptr));
        lbl_os->setText(QApplication::translate("DlgCreatePeer", "OS", nullptr));
        lbl_err_pass->setText(QString());
        lbl_disk->setText(QApplication::translate("DlgCreatePeer", "Disk GB", nullptr));
        lbl_err_os->setText(QString());
        lbl_err_name->setText(QString());
        cmb_os->setItemText(0, QApplication::translate("DlgCreatePeer", "Debian Stretch", nullptr));
        cmb_os->setItemText(1, QApplication::translate("DlgCreatePeer", "Ubuntu Xenial", nullptr));

        btn_create->setText(QApplication::translate("DlgCreatePeer", "Create", nullptr));
        lbl_err_cpu->setText(QString());
        lbl_name->setText(QApplication::translate("DlgCreatePeer", "Name", nullptr));
        lbl_err_ifs->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class DlgCreatePeer: public Ui_DlgCreatePeer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGCREATEPEER_H
