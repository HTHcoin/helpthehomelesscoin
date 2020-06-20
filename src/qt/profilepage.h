#ifndef PROFILEPAGE_H
#define PROFILEPAGE_H

#include <QMainWindow>
#include "posts.h"
namespace Ui {
class ProfilePage;
}

class ProfilePage : public QMainWindow
{
    Q_OBJECT

public:
    explicit ProfilePage(QWidget *parent = 0);
    ~ProfilePage();

private Q_SLOTS:
    void on_pushButton_2_clicked();
    void viewPosts();
    std::vector<Post>* makePosts();
    void on_LikeButton_clicked();
    void viewMorePosts(int i);
    void on_CommentButton_clicked();

private:
    Ui::ProfilePage *ui;
    unsigned int shownPostsNumber;
    std::vector<Post> *pagePosts;
};

#endif // PROFILEPAGE_H
