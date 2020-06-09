#include "loginpage.h"
#include "ui_loginpage.h"
#include <QMessageBox>
#include "mainwindow.h"



LoginPage::LoginPage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginPage)
{
    ui->setupUi(this);
}

LoginPage::~LoginPage()
{
    delete ui;
}

// method to close the window
void LoginPage::on_pushButton_back_clicked()
{
    close();
    MainWindow* welcomePage = new MainWindow(this);
    welcomePage->show();
}

// method for controlling the operation to log in
// validates that the username and password entered match with our data inside our table
void LoginPage::on_pushButton_login_clicked()
{
    // opening a connection to our database in this window using the mainwindow object
    MainWindow database;
    database.openDatabase();

    // get the username and password from the input lines
    QString username = ui->lineEdit_username->text();
    QString password = ui->lineEdit_password->text();

    QSqlQuery filter;

    // condition to check if the username and passwords match any of our data
    if (filter.exec("select * from User where Username='"+username+"' and Password='"+password+"'"))
    {
        int count = 0;

        while (filter.next())
        {
            count++;
        }

        // checking if the input was correct
        if (count == 1)
        {
            // closing the database
            database.closeDatabase();

            // entering their account page
            this->close();
            userPage = new homepage(this);
            userPage->setUserName(username);
            userPage->show();
        }
        else
        {
            QMessageBox::warning(this, "Warning","Invalid username or password. Please try again.");
        }
    }
    else
    {
        qDebug() << "Failed to log into account";
    }
}
