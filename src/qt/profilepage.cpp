#include "profilepage.h"
#include "ui_profilepage.h"

profilepage::profilepage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::profilepage)
{
    ui->setupUi(this);
}

profilepage::~profilepage()
{
    delete ui;
}
