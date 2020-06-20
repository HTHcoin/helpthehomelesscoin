#include "newaccount.h"
#include "ui_newaccount.h"
#include "QMessageBox"
newAccount::newAccount(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::newAccount)
{
    ui->setupUi(this);
}

newAccount::~newAccount()
{
    delete ui;
}

void newAccount::on_pushButton_clicked()
{
    QString password =ui->lineEdit_2->text();
    QString confirm =ui->lineEdit_3->text();
    if(password!=confirm)
    {
        QMessageBox :: information(this,"confirmation","your password isn't confirmed try again");
    }
}

void newAccount::setLoginPtr(MainWindow *ptr)
{
    this->mainWindowPtr = ptr;
}

void newAccount::on_pushButton_2_clicked()
{
    mainWindowPtr->show();
    this->hide();
}
