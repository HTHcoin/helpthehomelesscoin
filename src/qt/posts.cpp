#include "posts.h"

Post::Post()
{
    postOwner = "";
    postText = "";
}

Post::Post(QString owner, QString text, QString creationDate)
{
    postOwner = owner;
    postText = text;
    postDate = creationDate;
    postLikesNumber = 0;
}

void Post::setPostOwner(QString owner)
{
    postOwner = owner;
}

QString Post::getPostOwner()
{
    return postOwner;
}

void Post::setPostText(QString text)
{
    postText = text;
}

QString Post::getPostText()
{
    return postText;
}

void Post::addPostLike(QString likeOwner)
{
    postLikesNumber++;
    postLikesOwners.push_back(likeOwner);
}

void Post::removePostLike(QString owner)
{
    std::vector<QString>::iterator iterator;
    for(iterator = postLikesOwners.begin(); iterator != postLikesOwners.end(); iterator++)
    {
        if(*iterator == owner)
        {
            postLikesOwners.erase(iterator);
            break;
        }
    }
}

unsigned int Post::getPostLikesNumber()
{
    return postLikesNumber;
}

std::vector<QString> *Post::getPostLikesOwnersVectorPtr()
{
    return &postLikesOwners;
}

void Post::addComment(Comment newComment)
{
    postComments.push_back(newComment);
}

void Post::deleteComment(Comment toBeDeleted)
{
    std::vector<Comment>::iterator iterator;
    for(iterator = postComments.begin(); iterator != postComments.end(); iterator++)
    {
        if((*iterator).getCommentText() == toBeDeleted.getCommentText())
        {
            postComments.erase(iterator);
            break;
        }
    }
}

std::vector<Comment> *Post::getPostCommentsVectorPtr()
{
    return &postComments;
}

void Post::setPostDate(QString creationDate)
{
    postDate = creationDate;
}

QString Post::getPostDate()
{
    return postDate;
}















