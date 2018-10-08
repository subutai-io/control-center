#include "DlgCreateFolder.h"
#include "ui_DlgCreateFolder.h"
#include "OsBranchConsts.h"
#include <QRegularExpression>

DlgCreateFolder::DlgCreateFolder(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgCreateFolder),
  existing_files(QStringList()) {
  ui->setupUi(this);

  QIcon create_folder_icon(":/hub/create_folder_small.png");
  QPixmap create_folder_pixmap =
      create_folder_icon.pixmap(ui->lbl_icon->width(), ui->lbl_icon->height());
  ui->lbl_icon->setPixmap(create_folder_pixmap);

  ui->btn_create->setToolTip(tr("Create new folder"));
  ui->btn_cancel->setToolTip(tr("Cancel"));

  connect(ui->btn_create, &QPushButton::clicked,
          this, &DlgCreateFolder::btn_create_released);
  connect(ui->btn_cancel, &QPushButton::clicked,
          this, &DlgCreateFolder::btn_cancel_released);
}

DlgCreateFolder::~DlgCreateFolder() {
  delete ui;
}

void DlgCreateFolder::set_directory(const QString &type, const QString &dir) {
  ui->tb_text->acceptRichText();
  ui->tb_text->setOpenExternalLinks(true);
  ui->tb_text->setTextInteractionFlags(Qt::TextBrowserInteraction);
  ui->tb_text->setText(
        tr("You are going to create new folder in<br>\"%2\", %1 system").arg(type, dir));
  ui->tb_text->setStyleSheet("* { background-color: rgba(0, 0, 0, 0); border: none }");
  ui->tb_text->setAlignment(Qt::AlignLeft);
  ui->tb_text->setAlignment(Qt::AlignTop);
}

void DlgCreateFolder::add_existing_files(const QString &file) {
  if (file.at(file.size() - 1) == '/') {
    existing_files.append(file.right(file.size() - 1));
  } else {
    existing_files.append(file);
  }
}

void DlgCreateFolder::btn_create_released() {
  QString name = ui->le_name->text();
  QRegularExpression re(restricted_chars());

  if (existing_files.contains(name)) {
    ui->lbl_err->setVisible(true);
    ui->lbl_err->setStyleSheet("QLabel {color : red}");
    ui->lbl_err->setText(tr("File or folder with name %1 already exists.").arg(name));
  } else if (name.isEmpty()) {
    ui->lbl_err->setVisible(true);
    ui->lbl_err->setStyleSheet("QLabel {color : red}");
    ui->lbl_err->setText(tr("Folder name can not be empty"));
  } else if (CURRENT_OS == OS_WIN && (name.at(name.size() - 1) == ' ' || name.at(name.size() - 1) == '.')) {
    ui->lbl_err->setVisible(true);
    ui->lbl_err->setStyleSheet("QLabel {color : red}");
    ui->lbl_err->setText(tr("Folder name can not end with whitespaces"));
  } else if (name.contains(re)) {
    ui->lbl_err->setVisible(true);
    ui->lbl_err->setStyleSheet("QLabel {color : red}");
    ui->lbl_err->setText(
          tr("Folder name can not contain these character(s): %1")
          .arg(restricted_chars()));
  } else {
    emit got_new_folder_name(true, name);
  }
}

void DlgCreateFolder::btn_cancel_released() {
  emit got_new_folder_name(false, QString());
}
