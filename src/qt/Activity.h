#ifndef ACTIVITY_H
#define ACTIVITY_H
#include <QString>
#include <algorithm>
#include "vector"
#include "posts.h"
#include "QList"
class Activity
{
    unsigned int likesNumbers;

    unsigned int postsNumber;

    QList<QString> friends;


public:

    Activity();

    ~Activity();

    unsigned int getLikesNumbers();

    unsigned int getFriendsNumbers();

    unsigned int getPostsNumbers();

    void addLike();

    void addFriend(QString);

    void addPost();

    QList<QString> *getFriendsList_Ptr();
};

#endif // ACTIVITY_H
