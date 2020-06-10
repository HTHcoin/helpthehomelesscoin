#ifndef FILEMAN_H
#define FILEMAN_H
#include <iostream>
#include <QString>
#include <QList>
#include<QFile>
#include<QDebug>
#include <QTextStream>
#include "posts.h"
#include "comment.h"


class fileman{
public:
  QString path;
  QString name;
  QList <QString> posts;

  fileman();
  QString getPostByDate(QString Date);
  QList <Post>* getPosts(QString email);
  QString getUserNameByEmail(QString email);
  void addPost(QString userPost, QString Date, QString name);
  void addComment(QString userComment);
  void addCommentByPostDate(QString userComment, QString Date, QString nameComment,QString nameFile);
  void createFile( QString passWord,QString email,QString userName,QString date );
  void addFriend (QString friendName, QString name);
  void addPost_new(QString userPost, QString Date, QString name);
  void getCommentsByPostDate(QString email,QString date);
  void addFriends(QString email,QString friendName);
  void updateActivity(QString email, int numberOfLikes , int numberOfComments);
  friend class user;
  void networkFile();
  void addUsers();
  void TESTTEST();
  void createEmailFile();
  void addEmail(QString email);
  QList <QString> readEmails();
  QList <QString> getFriends(QString email);
  void getActivity(QString email, int &numberOfPosts, int &numberOfLikes);
  QList <Post>* getPosts_new(QString email);

public Q_SLOTS:
  void addLikeByPostDate(QString email, QString Date, QString likePerson);
  QString getPassword(QString email);
};

#endif
