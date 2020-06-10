#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSql>
#include <QDebug>
#include <QFileInfo>
#include <QSqlDatabase>

#include "signuppage.h"
#include "loginpage.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QSqlDatabase usersDataBase;
    void openDatabase();
    void closeDatabase();

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private Q_SLOTS:
    void on_pushButton_signup_clicked();

    void on_pushButton_login_clicked();

private:
    Ui::MainWindow *ui;
    LoginPage* login;
    SignUpPage* signup;
};

#endif // MAINWINDOW_H
