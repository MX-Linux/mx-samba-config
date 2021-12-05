#ifndef EDITSHARE_H
#define EDITSHARE_H

#include <QDialog>

namespace Ui {
class editshare;
}

class editshare : public QDialog
{
    Q_OBJECT

public:
    explicit editshare(QWidget *parent = nullptr);
    ~editshare();

private:
    Ui::editshare *ui;
};

#endif // EDITSHARE_H
