#include "comment.h"

Comment::Comment()
{
    commentOwner = "";
    commentText = "";
}

Comment::Comment(QString owner, QString text, QString creationDate)
{

    commentOwner = owner;
    commentText = text;
    commentDate = creationDate;
}

void Comment::setCommentOwner(QString owner)
{
    commentOwner = owner;
}

QString Comment::getCommentOwner()
{
    return commentOwner;
}

void Comment::setCommentText(QString text)
{
    commentText = text;
}

QString Comment::getCommentText()
{
    return commentText;
}

void Comment::setCommentDate(QString creationDate)
{
    commentDate = creationDate;
}

QString Comment::getCommentDate()
{
    return commentDate;
}

