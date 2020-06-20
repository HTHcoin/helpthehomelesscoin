#ifndef STATISTICS_H
#define STATISTICS_H
#include <QString>

class Statistics
{
public:
    Statistics();

    void setLikesNumber(unsigned int number);

    void setFriendsNumber(unsigned int number);

    void setPostsNumber(unsigned int number);

    void setUserID(unsigned int number);

    unsigned int getLikesNumber();

    unsigned int getPostsNumber();

    unsigned int getFriendsNumber();

    unsigned int getUserID();


private:
    unsigned int userID;

    unsigned int likesNumber;

    unsigned int friendsNumber;

    unsigned int postsNumber;
};

#endif // STATISTICS_H
