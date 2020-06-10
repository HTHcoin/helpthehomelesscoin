#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "newaccount.h"
#include<QDebug>
#include "homepage.h"
#include "QMessageBox"

MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::MainWindow)
{
ui->setupUi(this);
ui->signUpLabel->setText("<font color='red'>No account? Create one!</font>");
QWidget::setWindowIcon(QIcon(":/icons/chat"));
this->setWindowTitle("Social Network");

}

MainWindow::MainWindow(int userID) :
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->signUpLabel->setText("<font color='red'>No account? Create one!</font>");
    id=userID;
    qDebug()<< "id is "<<id;

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_signUpButton_clicked()
{
    newAccount *newAccountWindow = new newAccount;
    newAccountWindow->show();
    this->hide();


}

void MainWindow::on_logInButton_clicked()
{
    QString email=ui->txtUserMail->text();
    QFile userFile("Users/"+email+".xml");
    if(!userFile.open(QFile::ReadOnly))
    {
        QMessageBox::information(this,"Error","Email doesn't exist. Please sign up!");
        return;
    }
    user *currentSessionUser = new user();
    currentSessionUser->userName = email;
    currentSessionUser->userFileManipulator.name = email;
    QString password = currentSessionUser->userFileManipulator.getPassword(email);
    if(password != ui->txtPassword->text())
    {
        QMessageBox::information(this,"Error","Wrong Password. Please try again!");
        return;
    }
    HomePage *homePageWindow = new HomePage();
    homePageWindow->setCurrentSessionUser_Ptr(currentSessionUser);
    homePageWindow->show();
    this->hide();

}
