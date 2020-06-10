#include "user.h"

user::user()
{
    userName="";
    id=0;
}

user::user(QString passWord, QString email,QString name,int userID)
{
    userName=name;
    userFileManipulator.name=name;
    userFileManipulator.createFile(passWord,email,name,creationDate.getDateNow());
    id=userID;
}

void user :: setUsersList(user A)
{
    usersList.append(A);
}

QString user:: getUserName(int id)
{
    return usersList[id].userName;
}

 bool user:: usersEmpty()
 {
     return (usersList.size()==0);
 }

int user:: getUsersSize()
{
    return usersList.count();
}
