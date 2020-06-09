#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// method to open a connection to our database
void MainWindow::openDatabase()
{
    // making the connection to our database file
    usersDataBase = QSqlDatabase::addDatabase("QSQLITE");
    usersDataBase.setDatabaseName("C:/Users/carlo/Documents/Qt Projects/SocialMediaApp/socialMediaUsers.db");

    if (usersDataBase.open())
    {
        qDebug() << "We are connected" << endl;
    }
    else
    {
        qDebug() << "Connection failed" << endl;
    }
}

// method to close our database connection
void MainWindow::closeDatabase()
{
    usersDataBase.close();
    usersDataBase.removeDatabase(QSqlDatabase::defaultConnection);
    qDebug() << "We have closed the database" << endl;
}

// method to take the user to the signup page
void MainWindow::on_pushButton_signup_clicked()
{
    signup = new SignUpPage(this);
    signup->show();
}

// method to take the user to the login page
void MainWindow::on_pushButton_login_clicked()
{
    login = new LoginPage(this);
    login->show();
}
