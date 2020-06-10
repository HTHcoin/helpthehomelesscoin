#include "profilepage.h"
#include "ui_profilepage.h"
#include "homepage.h"

//selim includes
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QHBoxLayout"
#include <QVBoxLayout>
#include "QLabel"
#include "QSpacerItem"
#include "QPushButton"
#include <vector>
#include "posts.h"
#include <QString>
#include <QTextBrowser>
#include "QFontMetrics"
#include "QGroupBox"
#include "QScrollBar"
#include "mainwindow.h"
#include "addcomment.h"
#define POSTSNUMBER 100
#define COMMENTSNUMBER 3
#define POSTSATATIME 15
//_____

#include "qprocess.h"

ProfilePage::ProfilePage(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ProfilePage)
{
    ui->setupUi(this);
    shownPostsNumber = 0;
    //pagePosts = makePosts();
   // viewPosts();
    QWidget::setWindowIcon(QIcon(":/icons/chat"));
    this->showMaximized();
    this->setWindowTitle("Social Network");
}

 ProfilePage ::ProfilePage(int userID,QString email):  ui(new Ui::ProfilePage)
 {

      ui->setupUi(this);
      user A;
     // ui->lblUserName->setText(A.getUserName(userID)); // dyyy :((((
      ui->lblMail->setText(email);//user file's name is the email of the user
      shownPostsNumber = 0;
      qDebug()<<"id is "<<userID;
      //pagePosts = makePosts();
      //viewPosts();
 }

ProfilePage::~ProfilePage()
{
    delete ui;
}

void ProfilePage::on_pushButton_2_clicked()
{
    if(homePageOwnerMail != "")
        currentSessionUser->userName = homePageOwnerMail;
    QList<QObject*> oldPosts = ui->scrollAreaWidgetContents->children();
    for(int i = 1; i < oldPosts.size(); i++)
        delete oldPosts[i];
    HomePage *homePageWindow = new HomePage();
    homePageWindow->setCurrentSessionUser_Ptr(currentSessionUser);
    homePageWindow->show();
    this->hide();
}


void ProfilePage::setCurrentSessionUser_Ptr(user *currentSessionUser_ptr)
{
    this->currentSessionUser = currentSessionUser_ptr;
    pagePosts = currentSessionUser_ptr->userFileManipulator.getPosts_new(currentSessionUser_ptr->userName);
    viewPosts();
    if(homePageOwnerMail != "")
    {
        fileman x;
        QList<QString> friendsList = x.getFriends(homePageOwnerMail);
        for(QList<QString>::iterator it = friendsList.begin(); it != friendsList.end(); it++)
        {
            if(*it == currentSessionUser_ptr->userName)
            {
                ui->addFriendBtn->setEnabled(false);
                break;
            }
        }
    }
}

//Selim functions
void ProfilePage::viewPosts()
{
    ui->lblUserName->setText(currentSessionUser->userFileManipulator.getUserNameByEmail(currentSessionUser->userName));
    ui->lblMail->setText(currentSessionUser->userName);
    ui->labelNumber->setText(QString::number(currentSessionUser->userFileManipulator.getFriends(currentSessionUser->userName).size()/3));
    QList<QString> friendsList = currentSessionUser->userFileManipulator.getFriends(currentSessionUser->userName);
    if(homePageOwnerMail == "")
        ui->addFriendBtn->setEnabled(false);
    int j,k;
    for(j = 1; j < friendsList.size() - 1; j = j + 3)
    {
        ui->friendsComboBox->addItem(friendsList[j]);
    }
    static QVBoxLayout *ParentVerticalLayout = new QVBoxLayout;
    qDebug("now");
    QList<QObject*> oldPosts = ui->scrollAreaWidgetContents->children();
    for(int i = 1; i < oldPosts.size(); i++)
        delete oldPosts[i];
    for(k = shownPostsNumber; k < pagePosts->size() && shownPostsNumber < k + POSTSATATIME; k++,shownPostsNumber++)
    {
        QGroupBox *postframe = new QGroupBox;
        //postframe->setObjectName("frame"+QString::number(k));
        QVBoxLayout *postLayout = new QVBoxLayout;
        postframe->setLayout(postLayout);
        postframe->setStyleSheet("background-color: rgb(255, 255, 255);");
        QLabel *postOwner = new QLabel((*pagePosts)[shownPostsNumber].getPostOwner()+":");
        QLabel *postDate = new QLabel((*pagePosts)[shownPostsNumber].getPostOwner()+":"+(*pagePosts)[shownPostsNumber].getPostDate());
        postLayout->addWidget(postDate);
        QTextBrowser *postBody = new QTextBrowser;
        postBody->setText((*pagePosts)[k].getPostText()); /////////// shownPostsNumber
        QFontMetrics font_metrics(postBody->font());
        int font_height = font_metrics.height();
        // Get the height by multiplying number of lines by font height, Maybe add to this a bit for a slight margin? -Added
        int height = font_height * 6;
        postBody->setMinimumHeight(height);
        postBody->setMaximumHeight(height);
        postLayout->addWidget(postBody);
        ParentVerticalLayout->addWidget(postframe);
        QHBoxLayout *likeAndComment = new QHBoxLayout;
        QPushButton *like = new QPushButton("Like");
        like->setObjectName("LikeButton"+QString::number(shownPostsNumber));
        connect(like,SIGNAL(clicked(bool)),this,SLOT(on_LikeButton_clicked()));
        like->setStyleSheet("background-color: rgb(c0,c6,c8);");
        QPushButton *comment = new QPushButton("Comment",postframe);
        comment->setObjectName("CommentButton"+QString::number(shownPostsNumber));
        connect(comment,SIGNAL(clicked(bool)),this,SLOT(on_CommentButton_clicked()));
        comment->setStyleSheet("background-color: rgb(c0,c6,c8);");
        likeAndComment->addWidget(like);
        likeAndComment->addWidget(comment);
        postLayout->addLayout(likeAndComment);
        std::vector<QString>* likesOwnersVector_Ptr = (*pagePosts)[shownPostsNumber].getPostLikesOwnersVectorPtr();
        for(int cnt = 0; cnt < likesOwnersVector_Ptr->size(); cnt++)
        {
            if((currentSessionUser->userName == (*likesOwnersVector_Ptr)[cnt] && homePageOwnerMail == "") || (homePageOwnerMail == (*likesOwnersVector_Ptr)[cnt]))
            {
                like->setEnabled(false);
                break;
            }
        }
        QPushButton *likesOwners = new QPushButton(QString::number((*pagePosts)[shownPostsNumber].getPostLikesOwnersVectorPtr()->size())
                                                    +" people are liking this!");
        likesOwners->setObjectName("likesOwnersButton");
        connect(likesOwners,SIGNAL(clicked(bool)),this,SLOT(on_likesOwners_clicked()));
        QHBoxLayout *likesOwnersLayout = new QHBoxLayout;
        likesOwnersLayout->addWidget(likesOwners);
        postLayout->addLayout(likesOwnersLayout);
        for(j = 0; j < (*pagePosts)[shownPostsNumber].getPostCommentsVectorPtr()->size(); j++)
        {
            QHBoxLayout *commentLayout = new QHBoxLayout;
            QLabel *commentOwner = new QLabel ((*((*pagePosts)[shownPostsNumber].getPostCommentsVectorPtr()))[j].getCommentOwner());
            commentLayout->addWidget(commentOwner);
            QTextBrowser *commentBody = new QTextBrowser;
            commentBody->setText((*((*pagePosts)[shownPostsNumber].getPostCommentsVectorPtr()))[j].getCommentText());
            commentBody->setMinimumHeight(height);
            commentBody->setMaximumHeight(height);
            commentLayout->addWidget(commentBody);
            postLayout->addLayout(commentLayout);
        }
        QFrame* line = new QFrame();
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        ParentVerticalLayout->addWidget(line);
    }
    ParentVerticalLayout->addSpacerItem(new QSpacerItem(20,40,QSizePolicy::Fixed,QSizePolicy::Expanding));
    ui->scrollAreaWidgetContents->setLayout(ParentVerticalLayout);
    //connect(ui->PostsArea,SIGNAL(ui->PostsArea->scroll(50,50)),this,SLOT(close()));
    QScrollBar *verticalScrollBar = ui->PostsArea->verticalScrollBar();
    connect(verticalScrollBar,SIGNAL(valueChanged(int)),this,SLOT(viewMorePosts(int)));
}
void ProfilePage::on_LikeButton_clicked()
{
    QVBoxLayout *postLayout;
    QPushButton *button = qobject_cast<QPushButton*>( QObject::sender() );
    button->setEnabled(false);
    QString name = button->objectName();
    qDebug(name.toLatin1());
    QGroupBox *postframe = qobject_cast<QGroupBox*> (QObject::sender()->parent());
    postLayout = qobject_cast<QVBoxLayout*> (postframe->layout());
    QObject* labelObject = (postframe->children()[1]);
    QLabel* commentLabel = qobject_cast<QLabel*> (labelObject);
    QString labelString  = commentLabel->text();
    if(homePageOwnerMail == "")
        currentSessionUser->userFileManipulator.addLikeByPostDate(labelString.mid(0,labelString.indexOf(":")),
                                                              labelString.mid(labelString.indexOf(":") + 1) , currentSessionUser->userName);
    else
        currentSessionUser->userFileManipulator.addLikeByPostDate(labelString.mid(0,labelString.indexOf(":")),
                                                              labelString.mid(labelString.indexOf(":") + 1) , homePageOwnerMail);
    int postsNumber,likesNumber;
    currentSessionUser->userFileManipulator.getActivity(currentSessionUser->userName,postsNumber,likesNumber);
    currentSessionUser->userFileManipulator.updateActivity(currentSessionUser->userName,++likesNumber,0);

}
void ProfilePage::on_CommentButton_clicked()
{
    QString commentBody;
    QVBoxLayout *postLayout;
    AddComment commentWindow;
    commentWindow.setModal(true);
    commentWindow.exec();
    commentBody = commentWindow.getCommentBody();
    qDebug(commentBody.toLatin1());
    QGroupBox *postframe = qobject_cast<QGroupBox*> (QObject::sender()->parent());
    postLayout = qobject_cast<QVBoxLayout*> (postframe->layout());
    QHBoxLayout *commentLayout = new QHBoxLayout;
    QString commentOwnerName = currentSessionUser->userFileManipulator.getUserNameByEmail(currentSessionUser->userName);
    QLabel *commentOwner = new QLabel (commentOwnerName);
    commentLayout->addWidget(commentOwner);
    QTextBrowser *commentBodyTextBrowser = new QTextBrowser;
    commentBodyTextBrowser->setText(commentBody);
    QFontMetrics font_metrics(commentBodyTextBrowser->font());
    int font_height = font_metrics.height();
    // Get the height by multiplying number of lines by font height, Maybe add to this a bit for a slight margin?
    int height = font_height * 2;
    commentBodyTextBrowser->setMinimumHeight(height);
    commentBodyTextBrowser->setMaximumHeight(height);
    commentLayout->addWidget(commentBodyTextBrowser);
    postLayout->addLayout(commentLayout);
    /*QLabel labelInfo = *(qobject_cast<QLabel*>*/QObject* labelObject = (postframe->children()[1]);
    QLabel* commentLabel = qobject_cast<QLabel*> (labelObject);
    QString labelString  = commentLabel->text();
    qDebug(labelString.toLatin1());
    currentSessionUser->userFileManipulator.addCommentByPostDate(commentBody,labelString.mid(labelString.indexOf(":") + 1,labelString.length())
                                                                 ,currentSessionUser->userName,labelString.mid(0,labelString.indexOf(":")));
}

void ProfilePage::on_likesOwners_clicked()
{
    QVBoxLayout *postLayout;
    QPushButton *button = qobject_cast<QPushButton*>( QObject::sender() );
    QString name = button->objectName();
    qDebug(name.toLatin1());
    QGroupBox *postframe = qobject_cast<QGroupBox*> (QObject::sender()->parent());
    postLayout = qobject_cast<QVBoxLayout*> (postframe->layout());
    QObject* labelObject = (postframe->children()[1]);
    QLabel* commentLabel = qobject_cast<QLabel*> (labelObject);
    QString labelString  = commentLabel->text();
    QList<Post> *userPosts = currentSessionUser->userFileManipulator.getPosts_new(labelString.mid(0,labelString.indexOf(":")));
    QScrollArea* likesOwners = new QScrollArea;
    QVBoxLayout* scrollingAreaLayout = new QVBoxLayout;
    likesOwners->setLayout(scrollingAreaLayout);
    for(QList<Post>::iterator it = userPosts->begin(); it != userPosts->end(); it++)
    {
        if(it->getPostDate() == labelString.mid(labelString.indexOf(":") + 1))
        {
            std::vector<QString>* likesVector = it->getPostLikesOwnersVectorPtr();
            for(int i = 0; i < likesVector->size(); i++)
            {
                QLabel* userLabel = new QLabel((*likesVector)[i]);
                scrollingAreaLayout->addWidget(userLabel);
            }
        }
    }
    likesOwners->show();
}

void ProfilePage::viewMorePosts(int i)
{
    QScrollBar *bar = qobject_cast<QScrollBar*> (QObject::sender());
    int max = bar ->maximum();
    if (i > .9 * max && shownPostsNumber != (*pagePosts).size())
    {
        viewPosts();
    }
}



void ProfilePage::on_Post_btn_clicked()
{
    QString postText = ui->newPost_txtEdit->toPlainText();
//    currentSessionUser.userFileManipulator.addPost(postText);
    Date Now;
    Post recentlyAddedPost = Post(currentSessionUser->userName,postText,Now.getDateNow());
    pagePosts->push_front(recentlyAddedPost);
    shownPostsNumber = 0;
    // clear the post area
    //ui->PostsArea->
    viewPosts();
    currentSessionUser->userFileManipulator.addPost_new(postText,Now.getDateNow(),currentSessionUser->userName);
}

void ProfilePage::setHomePageOwnerMail(QString owner)
{
    homePageOwnerMail = owner;
    ui->newPost_txtEdit->setVisible(false);
    ui->Post_btn->setVisible(false);
}


void ProfilePage::on_addFriendBtn_clicked()
{
    currentSessionUser->userFileManipulator.addFriends(homePageOwnerMail,ui->lblMail->text());
    ui->addFriendBtn->setEnabled(false);
}

void ProfilePage::on_actionLog_out_triggered()
{
    qApp->quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}
