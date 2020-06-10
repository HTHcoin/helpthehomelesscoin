#ifndef SIGNUPPAGE_H
#define SIGNUPPAGE_H

#include <QDialog>
#include "homepage.h"

namespace Ui {
class SignUpPage;
}

class SignUpPage : public QDialog
{
    Q_OBJECT

public:
    explicit SignUpPage(QWidget *parent = 0);
    ~SignUpPage();

private Q_SLOTS:
    void on_pushButton_back_clicked();

    void on_pushButton_createAccount_clicked();

private:
    Ui::SignUpPage *ui;
    homepage* userPage;
};

#endif // SIGNUPPAGE_H
