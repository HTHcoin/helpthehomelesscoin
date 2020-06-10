#include "newaccount.h"
#include "ui_newaccount.h"
#include "QMessageBox"
#include<user.h>
#include <QWidget>

NewAccount::NewAccount(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::NewAccount)
{
    ui->setupUi(this);
    user check;
    id=check.getUsersSize();
    QWidget::setWindowIcon(QIcon(":/icons/chat"));
   this->setWindowTitle("Social Network");

}

NewAccount::~NewAccount()
{
    delete ui;
}

void NewAccount::on_pushButton_clicked()
{
    QString password =ui->lineEdit_2->text();
    QString confirm =ui->lineEdit_3->text();
    QString mail=ui->txtUserMail->text();
    if(password!=confirm)
    {
        QMessageBox :: information(this,"confirmation","your password isn't confirmed try again");
    }
    else
    {
        user temp (password,mail,ui->txtUserName->text(),id);
        temp.setUsersList(temp);
        MainWindow* newWindow= new MainWindow(id);
        newWindow->show();
        this->hide();
//        temp.userFileManipulator.updateActivity(mail,0,0);
    }

}

void NewAccount::setLoginPtr(MainWindow *ptr)
{
    this->mainWindowPtr = ptr;
}

void NewAccount::on_pushButton_2_clicked()
{
    MainWindow* newWindow= new MainWindow(id);
    newWindow->show();
   // mainWindowPtr->show();
    this->hide();

}
