#ifndef NEWACCOUNT_H
#define NEWACCOUNT_H

#include <QMainWindow>
#include "mainwindow.h"
namespace Ui {
class newAccount;
}

class newAccount : public QMainWindow
{
    Q_OBJECT

public:
    explicit newAccount(QWidget *parent = 0);
    ~newAccount();
    int id;


public slots:
    void setLoginPtr(MainWindow *ptr);


private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();


private:
    Ui::newAccount *ui;

    MainWindow *mainWindowPtr;
};

#endif // NEWACCOUNT_H
