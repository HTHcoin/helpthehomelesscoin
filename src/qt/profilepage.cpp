#include "profilepage.h"
#include "ui_profilepage.h"
#include "homepage.h"

//selim includes
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QHBoxLayout"
#include "QVBoxLayout"
#include "QLabel"
#include "QSpacerItem"
#include "QPushButton"
#include <vector>
#include "posts.h"
#include "Qstring"
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



ProfilePage::ProfilePage(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ProfilePage)
{
    ui->setupUi(this);
    shownPostsNumber = 0;
    pagePosts = makePosts();
    viewPosts();
}

ProfilePage::~ProfilePage()
{
    delete ui;
}

void ProfilePage::on_pushButton_2_clicked()
{
    HomePage* HomePageWindow= new HomePage;
    this->hide();
    HomePageWindow->show();

}


//selim functions
void ProfilePage::viewPosts()
{
    unsigned int j,k;
    static QVBoxLayout *ParentVerticalLayout = new QVBoxLayout;
    qDebug("now");
    for(k = shownPostsNumber; shownPostsNumber < pagePosts->size() && shownPostsNumber < k + POSTSATATIME; shownPostsNumber++)
    {
        QGroupBox *postframe = new QGroupBox;
        QVBoxLayout *postLayout = new QVBoxLayout;
        postframe->setLayout(postLayout);
        postframe->setStyleSheet("background-color: rgb(255, 255, 255);");
        QLabel *postOwner = new QLabel((*pagePosts)[shownPostsNumber].owner+":");
        postLayout->addWidget(postOwner);
        QTextBrowser *postBody = new QTextBrowser;
        postBody->setText((*pagePosts)[shownPostsNumber].postBody);
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
        for(j = 0; j < (*pagePosts)[shownPostsNumber].postComments.size(); j++)
        {
            QHBoxLayout *commentLayout = new QHBoxLayout;
            QLabel *commentOwner = new QLabel ((*pagePosts)[shownPostsNumber].postComments[j].owner);
            commentLayout->addWidget(commentOwner);
            QTextBrowser *commentBody = new QTextBrowser;
            commentBody->setText((*pagePosts)[shownPostsNumber].postComments[j].commentBody);
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
    QPushButton *button = qobject_cast<QPushButton*>( QObject::sender() );
    QString name = button->objectName();
    qDebug(name.toLatin1());
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
    QLabel *commentOwner = new QLabel ("manuallyAddedComment");
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
std::vector<Post>* ProfilePage::makePosts()
{
    unsigned int i,j;
    std::vector<Post> *testPosts= new std::vector<Post>;
    for(i = 0; i < POSTSNUMBER; i++)
    {
        QString postOwner = "Owner" + QString::number(i);
        QString postBody = "this is Post number" + QString::number(i)+"\n"+
                "Again, This is Post number" + QString::number(i)+"\n"+
                "Again, This is Post number" + QString::number(i)+"\n"+
                "Again, This is Post number" + QString::number(i)+"\n"+
                "Again, This is Post number" + QString::number(i);
        std::vector<Comment> postComments;
        for(j = 0; j < COMMENTSNUMBER; j++)
        {
            Comment postComment;
            postComment.owner = "Owner" + QString::number(i);
            postComment.commentBody = "this is Comment number" + QString::number(i)+"\n"+
                    "Again, This is Comment number" + QString::number(i)+"\n"+
                    "Again, This is Comment number" + QString::number(i)+"\n"+
                    "Again, This is Comment number" + QString::number(i)+"\n"+
                    "Again, This is Comment number" + QString::number(i);
            postComments.push_back(postComment);
        }
        Post tempPost = {postOwner,postBody,postComments};
        testPosts->push_back(tempPost);
    }
    return testPosts;
}

//___



///////------- end of selim's functions
