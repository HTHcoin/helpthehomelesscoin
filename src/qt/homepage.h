#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QMainWindow>
#include "profilepage.h"
#include "posts.h"
#include "user.h"
#include "QList"
#include "QVBoxLayout"
namespace Ui {
class HomePage;
}

class HomePage : public QMainWindow
{
    Q_OBJECT

public:
    explicit HomePage(QWidget *parent = 0);

    void setCurrentSessionUser_Ptr(user *currentSessionUser_ptr);

    friend class ProfilePage;

    QList<QString> search(char x, QList<QString> &myList);

    ~HomePage();


public Q_SLOTS:
    QList<QString> search(QString x, QList<QString>& myList);

    void internalSearch(QString x, QList<QString> &myList, int begin, int end, QList<QString> &temp);

    void sort(QList<QString> &myList);

    void siftDown(QList<QString> &myList, int size);

    void siftUp(QList<QString> &myList, int index);

    void swap(QString *a, QString *b);

    QList<QString> *randomPosts(QString mail, int number);

    void on_likesOwners_clicked();


private:

    Ui::HomePage *ui;

    user *currentSessionUser;

private Q_SLOTS:

    void viewPosts();

    void on_LikeButton_clicked();

    void viewMorePosts(int i);

    void on_CommentButton_clicked();

    void on_Post_btn_clicked();

    void on_pushButton_clicked();

    void on_StatisticsWindow_btn_clicked();

    void on_friendSearch_textChanged(const QString &arg1);

    void on_comboBox_currentIndexChanged(int index);

    void on_actionLog_Out_triggered();


private:
    unsigned int shownPostsNumber;

    QList<Post> *pagePosts;

};

#endif // HOMEPAGE_H
