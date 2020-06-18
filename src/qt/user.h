#ifndef USER_H
#define USER_H
#include "posts.h"
#include "fileman.h"
#include<QString>
#include<QList>
class user
{
public:
    QString userName;
    int id;
    Post userPost;
    fileman userFileManipulator;
    Date creationDate;
    user();
    user(QString passWord, QString email, QString name,int userID);
    void setUsersList( user A );
    QString getUserName(int id);
    bool usersEmpty();
    int getUsersSize();

private:
    QList <user> usersList;
};

#endif // USER_H
