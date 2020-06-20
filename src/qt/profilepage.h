#ifndef PROFILEPAGE_H
#define PROFILEPAGE_H

#include <QMainWindow>
#include "posts.h"
#include "user.h"
#include "homepage.h"
namespace Ui {
class ProfilePage;
}

class ProfilePage : public QWidget
{
    Q_OBJECT

public:
    explicit ProfilePage(QWidget *parent = 0);

    ~ProfilePage();

    ProfilePage(int userID,QString userFile);

private Q_SLOTS:
    void on_pushButton_2_clicked();

    void viewPosts();

    void on_LikeButton_clicked();

    void viewMorePosts(int i);

    void on_CommentButton_clicked();

    void on_addFriendBtn_clicked();

    void on_actionLog_out_triggered();


public Q_SLOTS:
    void setCurrentSessionUser_Ptr(user *currentSessionUser_ptr);

    void on_Post_btn_clicked();

    void setHomePageOwnerMail(QString owner);

    void on_likesOwners_clicked();


private:
    Ui::ProfilePage *ui;

    unsigned int shownPostsNumber;

    QList<Post> *pagePosts;

    user *currentSessionUser;

    QString homePageOwnerMail;
};

#endif // PROFILEPAGE_H
