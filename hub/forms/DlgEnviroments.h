#ifndef DLGENVIROMENTS_H
#define DLGENVIROMENTS_H

#include <QWidget>

namespace Ui {
class DlgEnviroments;
}

class DlgEnviroments : public QWidget
{
    Q_OBJECT

public:
    explicit DlgEnviroments(QWidget *parent = 0);
    ~DlgEnviroments();

private:
    Ui::DlgEnviroments *ui;
};

#endif // DLGENVIROMENTS_H
