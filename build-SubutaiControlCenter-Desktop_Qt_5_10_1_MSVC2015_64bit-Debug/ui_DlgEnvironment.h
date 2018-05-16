/********************************************************************************
** Form generated from reading UI file 'DlgEnvironment.ui'
**
** Created by: Qt User Interface Compiler version 5.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGENVIRONMENT_H
#define UI_DLGENVIRONMENT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DlgEnvironment
{
public:
    QGridLayout *gridLayout;
    QCheckBox *select_all;
    QVBoxLayout *cont_name;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_3;
    QPushButton *btn_ssh_selected;
    QPushButton *btn_desktop_selected;
    QPushButton *btn_upload_selected;
    QVBoxLayout *cont_rhip;
    QCheckBox *cb_details;
    QGroupBox *gr_details;
    QGridLayout *gridLayout_2;
    QLabel *label_2;
    QLabel *label_4;
    QLineEdit *le_env_status;
    QLineEdit *le_env_id;
    QLineEdit *le_env_key;
    QLineEdit *le_env_hash;
    QLabel *label;
    QLabel *label_3;
    QPushButton *btn_open_hub;
    QFrame *line;
    QVBoxLayout *cont_select;
    QFrame *line_2;
    QFrame *line_6;
    QLabel *label_9;
    QFrame *line_9;
    QLabel *label_5;
    QVBoxLayout *cont_desktop_info;
    QLabel *label_8;

    void setupUi(QWidget *DlgEnvironment)
    {
        if (DlgEnvironment->objectName().isEmpty())
            DlgEnvironment->setObjectName(QStringLiteral("DlgEnvironment"));
        DlgEnvironment->resize(616, 361);
        gridLayout = new QGridLayout(DlgEnvironment);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        select_all = new QCheckBox(DlgEnvironment);
        select_all->setObjectName(QStringLiteral("select_all"));

        gridLayout->addWidget(select_all, 0, 3, 1, 1, Qt::AlignHCenter);

        cont_name = new QVBoxLayout();
        cont_name->setObjectName(QStringLiteral("cont_name"));

        gridLayout->addLayout(cont_name, 2, 0, 1, 1);

        groupBox = new QGroupBox(DlgEnvironment);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setAlignment(Qt::AlignCenter);
        gridLayout_3 = new QGridLayout(groupBox);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        btn_ssh_selected = new QPushButton(groupBox);
        btn_ssh_selected->setObjectName(QStringLiteral("btn_ssh_selected"));

        gridLayout_3->addWidget(btn_ssh_selected, 0, 0, 1, 1);

        btn_desktop_selected = new QPushButton(groupBox);
        btn_desktop_selected->setObjectName(QStringLiteral("btn_desktop_selected"));

        gridLayout_3->addWidget(btn_desktop_selected, 0, 2, 1, 1);

        btn_upload_selected = new QPushButton(groupBox);
        btn_upload_selected->setObjectName(QStringLiteral("btn_upload_selected"));

        gridLayout_3->addWidget(btn_upload_selected, 0, 3, 1, 1);


        gridLayout->addWidget(groupBox, 5, 0, 1, 4);

        cont_rhip = new QVBoxLayout();
        cont_rhip->setObjectName(QStringLiteral("cont_rhip"));

        gridLayout->addLayout(cont_rhip, 2, 1, 1, 1);

        cb_details = new QCheckBox(DlgEnvironment);
        cb_details->setObjectName(QStringLiteral("cb_details"));
        cb_details->setLayoutDirection(Qt::LeftToRight);

        gridLayout->addWidget(cb_details, 6, 0, 1, 4, Qt::AlignHCenter);

        gr_details = new QGroupBox(DlgEnvironment);
        gr_details->setObjectName(QStringLiteral("gr_details"));
        gridLayout_2 = new QGridLayout(gr_details);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        label_2 = new QLabel(gr_details);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout_2->addWidget(label_2, 1, 0, 1, 1);

        label_4 = new QLabel(gr_details);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout_2->addWidget(label_4, 3, 0, 1, 1);

        le_env_status = new QLineEdit(gr_details);
        le_env_status->setObjectName(QStringLiteral("le_env_status"));

        gridLayout_2->addWidget(le_env_status, 0, 1, 1, 1);

        le_env_id = new QLineEdit(gr_details);
        le_env_id->setObjectName(QStringLiteral("le_env_id"));

        gridLayout_2->addWidget(le_env_id, 3, 1, 1, 1);

        le_env_key = new QLineEdit(gr_details);
        le_env_key->setObjectName(QStringLiteral("le_env_key"));

        gridLayout_2->addWidget(le_env_key, 2, 1, 1, 1);

        le_env_hash = new QLineEdit(gr_details);
        le_env_hash->setObjectName(QStringLiteral("le_env_hash"));

        gridLayout_2->addWidget(le_env_hash, 1, 1, 1, 1);

        label = new QLabel(gr_details);
        label->setObjectName(QStringLiteral("label"));

        gridLayout_2->addWidget(label, 0, 0, 1, 1);

        label_3 = new QLabel(gr_details);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout_2->addWidget(label_3, 2, 0, 1, 1);

        btn_open_hub = new QPushButton(gr_details);
        btn_open_hub->setObjectName(QStringLiteral("btn_open_hub"));

        gridLayout_2->addWidget(btn_open_hub, 4, 1, 1, 1);


        gridLayout->addWidget(gr_details, 8, 0, 1, 4);

        line = new QFrame(DlgEnvironment);
        line->setObjectName(QStringLiteral("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line, 1, 0, 1, 1);

        cont_select = new QVBoxLayout();
        cont_select->setObjectName(QStringLiteral("cont_select"));

        gridLayout->addLayout(cont_select, 2, 3, 1, 1);

        line_2 = new QFrame(DlgEnvironment);
        line_2->setObjectName(QStringLiteral("line_2"));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line_2, 1, 1, 1, 1);

        line_6 = new QFrame(DlgEnvironment);
        line_6->setObjectName(QStringLiteral("line_6"));
        line_6->setFrameShape(QFrame::HLine);
        line_6->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line_6, 1, 3, 1, 1);

        label_9 = new QLabel(DlgEnvironment);
        label_9->setObjectName(QStringLiteral("label_9"));
        label_9->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_9, 0, 0, 1, 1);

        line_9 = new QFrame(DlgEnvironment);
        line_9->setObjectName(QStringLiteral("line_9"));
        line_9->setFrameShape(QFrame::HLine);
        line_9->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line_9, 1, 2, 1, 1);

        label_5 = new QLabel(DlgEnvironment);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_5, 0, 2, 1, 1);

        cont_desktop_info = new QVBoxLayout();
        cont_desktop_info->setObjectName(QStringLiteral("cont_desktop_info"));

        gridLayout->addLayout(cont_desktop_info, 2, 2, 1, 1);

        label_8 = new QLabel(DlgEnvironment);
        label_8->setObjectName(QStringLiteral("label_8"));
        label_8->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_8, 0, 1, 1, 1);


        retranslateUi(DlgEnvironment);

        QMetaObject::connectSlotsByName(DlgEnvironment);
    } // setupUi

    void retranslateUi(QWidget *DlgEnvironment)
    {
        DlgEnvironment->setWindowTitle(QApplication::translate("DlgEnvironment", "Form", nullptr));
        select_all->setText(QApplication::translate("DlgEnvironment", "Select All", nullptr));
        groupBox->setTitle(QApplication::translate("DlgEnvironment", "Operations on selected containers", nullptr));
        btn_ssh_selected->setText(QApplication::translate("DlgEnvironment", "SSH ", nullptr));
        btn_desktop_selected->setText(QApplication::translate("DlgEnvironment", "Remote Desktop", nullptr));
        btn_upload_selected->setText(QApplication::translate("DlgEnvironment", "Transfer File", nullptr));
        cb_details->setText(QApplication::translate("DlgEnvironment", "More details", nullptr));
        gr_details->setTitle(QString());
        label_2->setText(QApplication::translate("DlgEnvironment", "Hash", nullptr));
        label_4->setText(QApplication::translate("DlgEnvironment", "ID", nullptr));
        label->setText(QApplication::translate("DlgEnvironment", "Status", nullptr));
        label_3->setText(QApplication::translate("DlgEnvironment", "Key", nullptr));
        btn_open_hub->setText(QApplication::translate("DlgEnvironment", "Open Bazaar", nullptr));
        label_9->setText(QApplication::translate("DlgEnvironment", "Container", nullptr));
        label_5->setText(QApplication::translate("DlgEnvironment", "Desktop info", nullptr));
        label_8->setText(QApplication::translate("DlgEnvironment", "Resource Host", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DlgEnvironment: public Ui_DlgEnvironment {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGENVIRONMENT_H
