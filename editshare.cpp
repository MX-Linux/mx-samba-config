#include "editshare.h"
#include "ui_editshare.h"

editshare::editshare(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::editshare)
{
    ui->setupUi(this);
}

editshare::~editshare()
{
    delete ui;
}
