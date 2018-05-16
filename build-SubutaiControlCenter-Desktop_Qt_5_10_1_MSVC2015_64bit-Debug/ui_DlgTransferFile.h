/********************************************************************************
** Form generated from reading UI file 'DlgTransferFile.ui'
**
** Created by: Qt User Interface Compiler version 5.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGTRANSFERFILE_H
#define UI_DLGTRANSFERFILE_H

#include <DlgTransferFile.h>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_DlgTransferFile
{
public:
    QGridLayout *gridLayout;
    QPushButton *btn_clear_files;
    QPushButton *btn_upload_file;
    FileSystemTableWidget *remote_file_system;
    QPushButton *btn_download_file;
    QFrame *line;
    QLineEdit *le_remote;
    QPushButton *btn_local_back;
    QLineEdit *le_local;
    QCheckBox *more_info;
    QPushButton *btn_refresh_remote;
    DropFileTableWidget *tw_transfer_file;
    QPushButton *btn_refresh_local;
    QPushButton *btn_start_transfer;
    QLabel *lbl_local_files;
    QLabel *lbl_remote_files;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_2;
    QLabel *label_3;
    QLineEdit *remote_ip;
    QLineEdit *remote_port;
    QLabel *label_5;
    QLabel *label_4;
    QLineEdit *remote_user;
    QLineEdit *remote_ssh_key_path;
    QLabel *label_7;
    QPushButton *btn_remote_back;
    FileSystemTableWidget *local_file_system;

    void setupUi(QDialog *DlgTransferFile)
    {
        if (DlgTransferFile->objectName().isEmpty())
            DlgTransferFile->setObjectName(QStringLiteral("DlgTransferFile"));
        DlgTransferFile->resize(898, 639);
        gridLayout = new QGridLayout(DlgTransferFile);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        btn_clear_files = new QPushButton(DlgTransferFile);
        btn_clear_files->setObjectName(QStringLiteral("btn_clear_files"));

        gridLayout->addWidget(btn_clear_files, 5, 8, 1, 1);

        btn_upload_file = new QPushButton(DlgTransferFile);
        btn_upload_file->setObjectName(QStringLiteral("btn_upload_file"));
        QIcon icon;
        icon.addFile(QStringLiteral(":/hub/upload-0.png"), QSize(), QIcon::Normal, QIcon::Off);
        btn_upload_file->setIcon(icon);

        gridLayout->addWidget(btn_upload_file, 2, 0, 1, 1);

        remote_file_system = new FileSystemTableWidget(DlgTransferFile);
        remote_file_system->setObjectName(QStringLiteral("remote_file_system"));

        gridLayout->addWidget(remote_file_system, 1, 5, 1, 4);

        btn_download_file = new QPushButton(DlgTransferFile);
        btn_download_file->setObjectName(QStringLiteral("btn_download_file"));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/hub/download.png"), QSize(), QIcon::Normal, QIcon::Off);
        btn_download_file->setIcon(icon1);

        gridLayout->addWidget(btn_download_file, 2, 5, 1, 1);

        line = new QFrame(DlgTransferFile);
        line->setObjectName(QStringLiteral("line"));
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line, 0, 4, 3, 1);

        le_remote = new QLineEdit(DlgTransferFile);
        le_remote->setObjectName(QStringLiteral("le_remote"));

        gridLayout->addWidget(le_remote, 2, 8, 1, 1);

        btn_local_back = new QPushButton(DlgTransferFile);
        btn_local_back->setObjectName(QStringLiteral("btn_local_back"));
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/hub/back.png"), QSize(), QIcon::Normal, QIcon::Off);
        btn_local_back->setIcon(icon2);

        gridLayout->addWidget(btn_local_back, 2, 1, 1, 1);

        le_local = new QLineEdit(DlgTransferFile);
        le_local->setObjectName(QStringLiteral("le_local"));

        gridLayout->addWidget(le_local, 2, 3, 1, 1);

        more_info = new QCheckBox(DlgTransferFile);
        more_info->setObjectName(QStringLiteral("more_info"));
        more_info->setLayoutDirection(Qt::LeftToRight);

        gridLayout->addWidget(more_info, 6, 8, 1, 1);

        btn_refresh_remote = new QPushButton(DlgTransferFile);
        btn_refresh_remote->setObjectName(QStringLiteral("btn_refresh_remote"));
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/hub/refresh.png"), QSize(), QIcon::Normal, QIcon::Off);
        btn_refresh_remote->setIcon(icon3);

        gridLayout->addWidget(btn_refresh_remote, 2, 7, 1, 1);

        tw_transfer_file = new DropFileTableWidget(DlgTransferFile);
        tw_transfer_file->setObjectName(QStringLiteral("tw_transfer_file"));

        gridLayout->addWidget(tw_transfer_file, 3, 0, 1, 9);

        btn_refresh_local = new QPushButton(DlgTransferFile);
        btn_refresh_local->setObjectName(QStringLiteral("btn_refresh_local"));
        btn_refresh_local->setIcon(icon3);

        gridLayout->addWidget(btn_refresh_local, 2, 2, 1, 1);

        btn_start_transfer = new QPushButton(DlgTransferFile);
        btn_start_transfer->setObjectName(QStringLiteral("btn_start_transfer"));

        gridLayout->addWidget(btn_start_transfer, 4, 8, 1, 1);

        lbl_local_files = new QLabel(DlgTransferFile);
        lbl_local_files->setObjectName(QStringLiteral("lbl_local_files"));
        lbl_local_files->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(lbl_local_files, 0, 0, 1, 4);

        lbl_remote_files = new QLabel(DlgTransferFile);
        lbl_remote_files->setObjectName(QStringLiteral("lbl_remote_files"));
        lbl_remote_files->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(lbl_remote_files, 0, 5, 1, 4);

        groupBox = new QGroupBox(DlgTransferFile);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        gridLayout_2 = new QGridLayout(groupBox);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout_2->addWidget(label_3, 0, 0, 1, 1);

        remote_ip = new QLineEdit(groupBox);
        remote_ip->setObjectName(QStringLiteral("remote_ip"));

        gridLayout_2->addWidget(remote_ip, 0, 1, 1, 1);

        remote_port = new QLineEdit(groupBox);
        remote_port->setObjectName(QStringLiteral("remote_port"));

        gridLayout_2->addWidget(remote_port, 1, 1, 1, 1);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QStringLiteral("label_5"));

        gridLayout_2->addWidget(label_5, 0, 2, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout_2->addWidget(label_4, 1, 0, 1, 1);

        remote_user = new QLineEdit(groupBox);
        remote_user->setObjectName(QStringLiteral("remote_user"));

        gridLayout_2->addWidget(remote_user, 0, 3, 1, 1);

        remote_ssh_key_path = new QLineEdit(groupBox);
        remote_ssh_key_path->setObjectName(QStringLiteral("remote_ssh_key_path"));

        gridLayout_2->addWidget(remote_ssh_key_path, 1, 3, 1, 1);

        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QStringLiteral("label_7"));

        gridLayout_2->addWidget(label_7, 1, 2, 1, 1);


        gridLayout->addWidget(groupBox, 8, 0, 1, 9);

        btn_remote_back = new QPushButton(DlgTransferFile);
        btn_remote_back->setObjectName(QStringLiteral("btn_remote_back"));
        btn_remote_back->setIcon(icon2);

        gridLayout->addWidget(btn_remote_back, 2, 6, 1, 1);

        local_file_system = new FileSystemTableWidget(DlgTransferFile);
        local_file_system->setObjectName(QStringLiteral("local_file_system"));

        gridLayout->addWidget(local_file_system, 1, 0, 1, 4);


        retranslateUi(DlgTransferFile);

        QMetaObject::connectSlotsByName(DlgTransferFile);
    } // setupUi

    void retranslateUi(QDialog *DlgTransferFile)
    {
        DlgTransferFile->setWindowTitle(QApplication::translate("DlgTransferFile", "Dialog", nullptr));
        btn_clear_files->setText(QApplication::translate("DlgTransferFile", "Clear files", nullptr));
        btn_upload_file->setText(QString());
        btn_download_file->setText(QString());
        btn_local_back->setText(QString());
        more_info->setText(QApplication::translate("DlgTransferFile", "More Info about remote machine", nullptr));
        btn_refresh_remote->setText(QString());
        btn_refresh_local->setText(QString());
        btn_start_transfer->setText(QApplication::translate("DlgTransferFile", "Start to Transfer Files", nullptr));
        lbl_local_files->setText(QApplication::translate("DlgTransferFile", "Local", nullptr));
        lbl_remote_files->setText(QApplication::translate("DlgTransferFile", "Remote", nullptr));
        groupBox->setTitle(QApplication::translate("DlgTransferFile", "Remote Machine Parameters", nullptr));
        label_3->setText(QApplication::translate("DlgTransferFile", "IP", nullptr));
        label_5->setText(QApplication::translate("DlgTransferFile", "User", nullptr));
        label_4->setText(QApplication::translate("DlgTransferFile", "Port", nullptr));
        label_7->setText(QApplication::translate("DlgTransferFile", "Key ", nullptr));
        btn_remote_back->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class DlgTransferFile: public Ui_DlgTransferFile {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGTRANSFERFILE_H
