#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QDialog>
#include "homepage.h"

namespace Ui {
class LoginPage;
}

class LoginPage : public QDialog
{
    Q_OBJECT

public:
    explicit LoginPage(QWidget *parent = 0);
    ~LoginPage();

private Q_SLOTS:
    void on_pushButton_back_clicked();

    void on_pushButton_login_clicked();

private:
    Ui::LoginPage *ui;
    homepage* userPage;
};

#endif // LOGINPAGE_H
