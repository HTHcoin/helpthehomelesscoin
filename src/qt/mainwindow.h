#ifndef _MainWindow_H
#define	_MainWindow_H

#include <QtGui>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QDialog>
#include <QLineEdit>

class MainWindow : public QDialog
{
    Q_OBJECT
public:
    MainWindow();
    QPushButton *loginbtn;
    QPushButton *quitbtn;
    QLineEdit *usernametext;
    QLineEdit *passtext;
    QLabel *password;
    QLabel *username;
    void setupUi(QDialog *MainWindow);
    void retranslateUi(QDialog *MainWindow);
    virtual ~MainWindow();
public Q_SLOTS:
    void loginclick();
private:
    QString user;
    QString pass;
};

#endif	/* _MainWindow_H */
