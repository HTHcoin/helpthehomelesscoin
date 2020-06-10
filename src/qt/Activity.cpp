#include "Activity.h"
#include <algorithm>
#include "posts.h"
#include"vector"


Activity::Activity()
{
    likesNumbers = 0;
    postsNumber = 0;
}

Activity::~Activity()
{

}
unsigned int Activity::getFriendsNumbers()

   {return friends.size();}

unsigned int Activity::getPostsNumbers()

{ return postsNumber;}

void Activity::addFriend(QString friendName)
{
    friends.push_back(friendName);
}

void Activity::addPost()
{
    postsNumber++;
}

unsigned int Activity::getLikesNumbers()
{
    return likesNumbers;
}

void Activity:: addLike()
{
    likesNumbers++;
}

QList<QString> *Activity::getFriendsList_Ptr()
{
    return &friends;
}
