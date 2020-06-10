#include "statistics.h"

Statistics::Statistics()
{

}

void Statistics::setLikesNumber(unsigned int number)
{
    likesNumber = number;
}

void Statistics::setFriendsNumber(unsigned int number)
{
    friendsNumber = number;
}

void Statistics::setPostsNumber(unsigned int number)
{
    postsNumber = number;
}

void Statistics::setUserID(unsigned int number)
{
    userID = number;
}

unsigned int Statistics::getLikesNumber()
{
    return likesNumber;
}

unsigned int Statistics::getPostsNumber()
{
    return postsNumber;
}

unsigned int Statistics::getFriendsNumber()
{
    return friendsNumber;
}

unsigned int Statistics::getUserID()
{
    return userID;
}
