#ifndef POSTS_H
#define POSTS_H
#include "comment.h"
#include <QString>
#include <vector>
class Post
{
  QString postOwner;
  QString postText;
  unsigned int postLikesNumber;
  std::vector<QString> postLikesOwners;
  std::vector<Comment> postComments;
  QString postDate;
public:

  /**
   * @brief <sets postowner, and text to empty strings, and post date to the creation date of the class>
   *
   * @param <void>
   *
   * @return <void>
   */
  Post();

  /**
   * @brief <sets the postOwner and text and the date of the creation of the post>
   *
   * @param <string postOwner, postText, postDate>
   *
   * @example <foo@yahoo,this is an example,dateOfthePost>
   *
   * @return <void>
   */
  Post(QString,QString,QString);

  /**
   * @brief <sets the ID of the postOwner>
   *
   * @param <string postOwner> <postOwner ID>
   *
   * @example <foo@yahoo>
   *
   * @return <void>
   */
  void setPostOwner(QString);

  /**
   * @brief <returns the ID of the postOwner>
   *
   * @param <void>
   *
   * @example <foo@yahoo>
   *
   * @return <string postOwner>
   */
  QString getPostOwner();

  /**
   * @brief <sets the post text, and maybe used to edit a post>
   *
   * @<Example> <this is a post.>
   *
   * @param <QString PostText>
   *
   * @return <void>
   */
  void setPostText(QString);

  /**
   * @brief <returns the post text. Maybe used to view the post>
   *
   * @example <this is the post you want to view!!.>
   *
   * @param <void>
   *
   * @return <QString postText>
   */
  QString getPostText();

  /**
   * @brief <used to add a like to the post.>
   *
   * @example <a user pressed on the like button.>
   *
   * @param <QString userID>
   *
   * @return <void>
   */
  void addPostLike(QString);

  /**
   * @brief <used to remove a like from a post>
   *
   * @example <a user who already has a like on the post, pressed again on the like button to remove the like.>
   *
   * @param <QString userID>
   *
   * @return <void>
   */
  void removePostLike(QString);

  /**
   * @brief <returns the likesNumber of a post>
   *
   * @example <35 likes or so. Maybe used to view the post likes number in a label or something.>
   *
   * @param <void>
   *
   * @return <unsigned int that indicates the likes number>
   */
  unsigned int getPostLikesNumber();

  /**
   * @brief <returns a pointer the likes users vector. Maybe used to view who liked the post>
   *
   * @example <a pointer to a vector of QStrings like {nourhan@yahoo,Youmna@yahoo,Mahmoud@yahoo....}>
   *
   * @param <void>
   *
   * @return <std::vector<QString>* postLikesOwners>
   */
  std::vector<QString>* getPostLikesOwnersVectorPtr();

  /**
   * @brief <add a comment to the post>
   *
   * @example <a user pressed on a comment button and entered a text.>
   *
   * @param <Comment postComment>
   *
   * @return <void>
   */
  void addComment(Comment);

  /**
   * @brief <used to delete an existing comment on the post.>
   *
   * @example <a user who has a commment on the post want to delete it.>
   *
   * @param <Comment theCommentToBeDeleted>
   *
   * @return <void>
   */
  void deleteComment(Comment);

  /**
   * @brief <returns a pointer the post comments. Maybe used to view the comments>
   *
   * @example <a pointer to a vector of Comments like {comment1,comment2,....}>
   *
   * @param <void>
   *
   * @return <std::vector<Comment>*  commentsVector>
   */
  std::vector<Comment>* getPostCommentsVectorPtr();

  /**
   * @brief <set the date of a post. Maybe used when a post is modified to modify the date>
   *
   * @example: <the user made a post or edited a post.>
   *
   * @param <Date postCreationDate>
   *
   * @return <void>
   */
  void setPostDate(QString);

  /**
   * @brief <returns the post creation/last modification Date. Maybe used to view a post Date>
   *
   * @example <it's needed to view a post date>
   *
   * @param <void>
   *
   * @return <Date postDate>
   */
  QString getPostDate();
};
#endif // POSTS_H
