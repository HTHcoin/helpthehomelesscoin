#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "newaccount.h"
#include  <QDebug>
#include <QtXml/QtXml>
#include <QFile>
#include "homepage.h"
#include "QMessageBox"
#include <iostream>

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

void MainWindow::on_loginButton_clicked()
{
        int count;
        QString user,pass,u,p;
        system("cls");
        std::cout<<"please enter the following details"<<endl;
        std::cout<<"USERNAME :";
        cin>>user;
        std::cout<<"PASSWORD :";
        cin>>pass;

        ifstream input("database.txt");
        while(input>>u>>p)
        {
                if(u==user && p==pass)

                {
                        count=1;
                        system("cls");
                }
        }
        input.close();
        if(count==1)
        {
                std::cout<<"\nHello"<<user<<"\nLOGIN SUCESS\nWe're glad that you're here.\nThanks for logging in\n";
                cin.get();
                cin.get();
                main();
        }
        else
        {
                std::cout<<"\nLOGIN ERROR\nPlease check your username and password\n";
                main();
        }
}
void MainWindow::on_registrButton_clicked()
{

        QString reguser,regpass,ru,rp;
        system("cls");
        std::cout<<"Enter the username :";
        cin>>reguser;
        std::cout<<"\nEnter the password :";
        cin>>regpass;

        ofstream reg("database.txt",ios::app);
        reg<<reguser<<' '<<regpass<<endl;
        system("cls");
        std::cout<<"\nRegistration Sucessful\n";
        main();


}

void MainWindow::on_forgotButton_clicked()
{
        int ch;
        system("cls");
        std::cout<<"Forgotten ? We're here for help\n";
        std::cout<<"1.Search your id by username"<<endl;
        std::cout<<"2.Search your id by password"<<endl;
        std::cout<<"3.Main menu"<<endl;
        std::cout<<"Enter your choice :";
        cin>>ch;
        switch(ch)
        {
                case 1:
                {
                        int count=0;
                        QString searchuser,su,sp;
                        std::cout<<"\nEnter your remembered username :";
                        cin>>searchuser;

                        ifstream searchu("database.txt");
                        while(searchu>>su>>sp)
                        {
                                if(su==searchuser)
                                {
                                        count=1;
                                }
                        }
                        searchu.close();
                        if(count==1)
                        {
                                std::cout<<"\n\nHurray, account found\n";
                                std::cout<<"\nYour password is "<<sp;
                                cin.get();
                                cin.get();
                                system("cls");
                                main();
                        }
                        else
                        {
                                std::cout<<"\nSorry, Your userID is not found in our database\n";
                                std::cout<<"\nPlease kindly contact your system administrator for more details \n";
                                cin.get();
                                cin.get();
                                main();
                        }
                        break;
                }
                case 2:
                {
                        int count=0;
                        QString searchpass,su2,sp2;
                        std::cout<<"\nEnter the remembered password :";
                        cin>>searchpass;

                        ifstream searchp("database.txt");
                        while(searchp>>su2>>sp2)
                        {
                                if(sp2==searchpass)
                                {
                                        count=1;
                                }
                        }
                        searchp.close();
                        if(count==1)
                        {
                                std::cout<<"\nYour password is found in the database \n";
                                std::cout<<"\nYour Id is : "<<su2;
                                cin.get();
                                cin.get();
                                system("cls");
                                main();
                        }
                        else
                        {
                                std::cout<<"Sorry, We cannot found your password in our database \n";
                                std::cout<<"\nkindly contact your administrator for more information\n";
                                cin.get();
                                cin.get();
                                main();
                        }

                        break;
                }

                case 3:
                {
                        cin.get();
                        main();
                }
                default:
                        std::cout<<"Sorry, You entered wrong choice. Kindly try again"<<endl;
                        forgot();
        }
}
main()
{
        int choice;
        std::cout<<"***********************************************************************\n\n";
        std::cout<<"                      Welcome to login page                               \n\n";
        std::cout<<"*******************        MENU        ********************************\n\n";
        std::cout<<"1.LOGIN\n";
        std::cout<<"2.REGISTER\n";
        std::cout<<"3.FORGOT PASSWORD (or) USERNAME\n";
        std::cout<<"4.Exit\n";
        std::cout<<"\nEnter your choice : ";
        cin>>choice;
        std::cout<<endl;
        switch(choice)
        {
                case 1:
                        login();
                        break;
                case 2:
                        registr();
                        break;
                case 3:
                        forgot();
                        break;
                case 4:

                        cout<<"Thanks for using this program\nThis program is created by @Thestral9\n\n";
                        break;
                default:
                        system("cls");
                        std::cout<<"Wrong Choice Intered\n"<<endl;
                        main();
        }


    HomePage *homePageWindow = new HomePage();
    homePageWindow->setCurrentSessionUser_Ptr(currentSessionUser);
    homePageWindow->show();
    this->hide();

}
