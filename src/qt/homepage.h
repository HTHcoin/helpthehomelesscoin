#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QDialog>
#include <QTextEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <QMessageBox>

namespace Ui {
class homepage;
}

class homepage : public QDialog
{
    Q_OBJECT

public:
    explicit homepage(QWidget *parent = 0);
    ~homepage();

    void setUserName(QString);
    void loadProfile();
    void loadNewsfeed();
    void loadNotifications();
    void loadFriends();

private slots:
    void on_pushButton_logout_clicked();

    void on_pushButton_post_clicked();

    void on_pushButton_saveChanges_clicked();

    void on_pushButton_removeAccount_clicked();

    void on_pushButton_search_clicked();

    void on_pushButton_sendFriend_clicked();

    void on_pushButton_acceptNotification_clicked();

    void on_pushButton_ignoreNotification_clicked();

    void on_pushButton_removeFriend_clicked();

private:
    Ui::homepage *ui;

    QString userId;
    QString firstName;
    QString lastName;
    QString username;
    QString password;
    QString gender;
    QString birthday;
    QString fullName;
    QString country;

    QVBoxLayout* mainLayout;
};

#endif // HOMEPAGE_H
