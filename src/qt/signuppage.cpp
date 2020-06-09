#include "signuppage.h"
#include "ui_signuppage.h"
#include <QMessageBox>
#include "mainwindow.h"

const int monthSize = 12;
const int countrySize = 8;
QString monthList[monthSize] = {"January", "February", "March", "April", "May", "June","July", "August","September", "October", "November", "December"};
QString countryList[countrySize] = {"United States", "Canada", "Mexico", "Russia", "France", "India", "China", "Germany"};

SignUpPage::SignUpPage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SignUpPage)
{
    ui->setupUi(this);

    // adding the elements to the month drop down menu
    for (int x = 0; x < monthSize; x++)
    {
        ui->comboBox_months->addItem(monthList[x]);
    }

    // addinf the elements to the country drop down menu
    for (int x = 0; x < countrySize; x++)
    {
        ui->comboBox_country->addItem(countryList[x]);
    }
}

SignUpPage::~SignUpPage()
{
    delete ui;
}

// method to close the window
void SignUpPage::on_pushButton_back_clicked()
{
    close();
}

void SignUpPage::on_pushButton_createAccount_clicked()
{
    // opening a connection to our database
    MainWindow database;
    database.openDatabase();
    QSqlQuery filter;

    // getting the user input from the sign up page
    QString name = ui->lineEdit_firstname->text();
    QString lastname = ui->lineEdit_lastname->text();
    QString username = ui->lineEdit_username->text();
    QString password = ui->lineEdit_password->text();
    QString month = ui->comboBox_months->currentText();
    QString date = ui->lineEdit_date->text();
    QString year = ui->lineEdit_year->text();
    QString country = ui->comboBox_country->currentText();

    QString fullname = name + " " + lastname;
    QString birthday = month + " " + date + ", " + year;

    // getting the gender by checking which of the radio buttons has been checked
    QString gender;
    if (ui->radioButton_female->isChecked())
    {
        gender = "Female";
    }
    else if (ui->radioButton_male->isChecked())
    {
        gender = "Male";
    }

    // condition to assure that all the field are not empty
    if (name.isEmpty() || lastname.isEmpty() || username.isEmpty() || password.isEmpty() || month.isEmpty() || date.isEmpty() || year.isEmpty() || gender.isEmpty() || country.isEmpty())
    {
        // we will send message to the user to fill out all of the data
        QMessageBox::warning(this,"Warning", "Please fill out all of the information");
    }
    else
    {
        // section to check that the password is correct
        if (password.length() < 6)
        {
            QMessageBox::warning(this,"Warning", "Password must be atleast 6 characters long");
        }
        else
        {
            // analyzing the password input
            bool hasUppercase = false;
            bool hasNumber = false;
            for (int x = 0; x < password.length(); x++)
            {
                if (password[x] >= 'A' && password[x] < 'Z')
                {
                    hasUppercase = true;
                }

                if (password[x] >= '0' && password[x] < '9')
                {
                    hasNumber = true;
                }
            }

            // condition to check if password met the 2 requirements for including an uppercase letter and a number
            if (!hasNumber || !hasUppercase)
            {
                QMessageBox::warning(this,"Warning", "Password must include atleast 1 uppercase letter and atleast 1 number");
            }
            else
            {
                // before we create the new user account we validate that the username entered is not similar to one already found in our database
                // if the username entered is not unique we let the user know and we do not let them proceed
                QSqlQuery check;
                if (check.exec("select * from User where Username='"+username+"'"))
                {
                    int count = 0;

                    while (check.next())
                    {
                        count++;
                    }

                    if (count > 0)
                    {
                        QMessageBox::warning(this,"Warning","That username is taken. Try another");
                        return;
                    }
                }

                // adding the new record to our database table
                if (filter.exec("insert into User (FirstName,LastName,FullName,Username,Password,Birthday,Country,Gender) values ('"+name+"','"+lastname+"','"+fullname+"','"+username+"','"+password+"','"+birthday+"','"+country+"','"+gender+"')"))
                {
                   // closing the database and navigating to their account page
                   database.closeDatabase();
                   this->close();
                   userPage = new homepage(this);
                   userPage->setUserName(username);
                   userPage->show();
                }
                else
                {
                   qDebug() << "Failed to create a new account ";
                }
            }
        }
    }
}
