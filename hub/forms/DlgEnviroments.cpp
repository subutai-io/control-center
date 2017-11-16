#include "DlgEnviroments.h"
#include "ui_DlgEnviroments.h"

DlgEnviroments::DlgEnviroments(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DlgEnviroments)
{
    ui->setupUi(this);

}

DlgEnviroments::~DlgEnviroments()
{
    delete ui;
}
