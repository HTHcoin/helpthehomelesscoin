#ifndef NEWACCOUNT_H
#define NEWACCOUNT_H

#include <QMainWindow>
#include "mainwindow.h"
namespace Ui {
class NewAccount;
}

class NewAccount : public QMainWindow
{
    Q_OBJECT

public:
    explicit NewAccount(QWidget *parent = 0);
    ~NewAccount();
    int id;


public Q_SLOTS:
    void setLoginPtr(MainWindow *ptr);


private Q_SLOTS:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();


private:
    Ui::NewAccount *ui;

    MainWindow *mainWindowPtr;
};

#endif // NEWACCOUNT_H
