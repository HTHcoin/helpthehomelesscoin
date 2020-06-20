#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "newaccount.h"
#include "homepage.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->signUpLabel->setText("<font color='red'>No account? Create one!</font>");

}

MainWindow::~MainWindow()
{
    delete ui;
}

/*void MainWindow::on_signUpButton_clicked()
{
    newAccount *newAccountWindow = new newAccount;
    newAccountWindow->show();
    this->hide();
    newAccountWindow->setLoginPtr(this);

} */

void MainWindow::on_logInButton_clicked()
{
    this->hide();
    HomePage *homePageWindow = new HomePage;
    homePageWindow->show();
}
