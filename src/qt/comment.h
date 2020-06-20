#ifndef COMMENT_H
#define COMMENT_H

#include <QString>
#include <QDate>
#include <QTime>
class Date : public QDate
{
    QTime timeIsNow;
public:
    QString getDateNow(){return this->currentDate().toString("yyyy.MM.dd") +" @ " +timeIsNow.currentTime().toString("HH:mm:ss");}
};
class Comment
{
    QString commentOwner;
    QString commentText;
    QString commentDate;
public:
    Comment();

    Comment(QString, QString, QString);

    void setCommentOwner(QString);

    QString getCommentOwner();

    void setCommentText(QString);

    QString getCommentText();

    void setCommentDate(QString);

    QString getCommentDate();
};

#endif // COMMENT_H
