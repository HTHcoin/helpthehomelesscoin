#include "homepage.h"
#include "ui_homepage.h"
#include "profilepage.h"
#include "qprocess.h"

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
#include "QString"
#include <QTextBrowser>
#include "QFontMetrics"
#include "QGroupBox"
#include "QScrollBar"
#include "mainwindow.h"
#include "addcomment.h"
#include "adminwindow.h"
#define POSTSNUMBER 100
#define COMMENTSNUMBER 3
#define POSTSATATIME 15
//_____
HomePage::HomePage(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::HomePage)
{
    ui->setupUi(this);
    shownPostsNumber = 0;
    ui->comboBox->setVisible(false);
    pagePosts = new QList<Post>;
    QWidget::setWindowIcon(QIcon(":/icons/about.png"));
    this->showMaximized();
    this->setWindowTitle("Social Network");

}

void HomePage::setCurrentSessionUser_Ptr(user *currentSessionUser_ptr)
{
    this->currentSessionUser = currentSessionUser_ptr;
    randomPosts(currentSessionUser_ptr->userName,POSTSATATIME);
    QString domain = currentSessionUser_ptr->userName.mid(currentSessionUser_ptr->userName.length()-9,9);
    if( domain!= "admin.com")
        ui->StatisticsWindow_btn->setVisible(false);
    viewPosts();
}


void HomePage::viewPosts()
{
    unsigned int j,k;
    static QVBoxLayout *ParentVerticalLayout = new QVBoxLayout;
    qDebug("now");
    QList<QObject*> oldPosts = ui->scrollAreaWidgetContents->children();
    for(int i = 1; i < oldPosts.size(); i++)
        delete oldPosts[i];
    for(k = shownPostsNumber; k < pagePosts->size() && shownPostsNumber < k + POSTSATATIME; k++,shownPostsNumber++)
    {
        QGroupBox *postframe = new QGroupBox;
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
            if(currentSessionUser->userName == (*likesOwnersVector_Ptr)[cnt])
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
    QScrollBar *verticalScrollBar = ui->PostsArea->verticalScrollBar();
    connect(verticalScrollBar,SIGNAL(valueChanged(int)),this,SLOT(viewMorePosts(int)));
}
void HomePage::on_LikeButton_clicked()
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
    currentSessionUser->userFileManipulator.addLikeByPostDate(labelString.mid(0,labelString.indexOf(":")),
                                                              labelString.mid(labelString.indexOf(":") + 1) , currentSessionUser->userName);
    int postsNumber,likesNumber;
    currentSessionUser->userFileManipulator.getActivity(currentSessionUser->userName,postsNumber,likesNumber);
    currentSessionUser->userFileManipulator.updateActivity(currentSessionUser->userName,++likesNumber,0);
}
void HomePage::on_CommentButton_clicked()
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


void HomePage::on_likesOwners_clicked()
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

void HomePage::viewMorePosts(int i)
{
    QScrollBar *bar = qobject_cast<QScrollBar*> (QObject::sender());
    int max = bar ->maximum();
    if (i > .9 * max && shownPostsNumber != (*pagePosts).size())
    {
        viewPosts();
    }
}


HomePage::~HomePage()
{
    delete ui;
}

void HomePage::on_pushButton_clicked()
{
    QList<QObject*> oldPosts = ui->scrollAreaWidgetContents->children();
    for(int i = 1; i < oldPosts.size(); i++)
        delete oldPosts[i];
    ProfilePage * profilePageWindow=new ProfilePage;
    profilePageWindow->show();
    this->hide();
    profilePageWindow->setCurrentSessionUser_Ptr(currentSessionUser);
}

void HomePage::on_Post_btn_clicked()
{
    QString postText = ui->newPost_txtEdit->toPlainText();
    Date Now;
    Post recentlyAddedPost = Post(currentSessionUser->userName,postText,Now.getDateNow());
    pagePosts->push_front(recentlyAddedPost);
    shownPostsNumber = 0;
    viewPosts();
    currentSessionUser->userFileManipulator.addPost_new(postText,Now.getDateNow(),currentSessionUser->userName);

}

void HomePage::on_StatisticsWindow_btn_clicked()
{
    AdminWindow *adminWindow = new AdminWindow;
    adminWindow->show();;
    this->hide();
}


void HomePage::swap(QString *a,QString *b)
{

    QString temp=*a;
    *a=*b;
    *b=temp;
}

void HomePage::siftUp(QList<QString>&myList,int index) //O(log(n)
{

    int  parentIndex= (index%2==0)?(index-2)/2 : (index-1)/2;
     while(  index!=0 && myList[parentIndex][0]<myList[index][0] )
       {
           swap(&myList[parentIndex],&myList[index]);
           index=parentIndex;
            parentIndex= (index%2==0)?(index-2)/2 : (index-1)/2;
       }
}

void HomePage::siftDown(QList<QString>&myList,int size)  //O(log(n)
{   int index=0;
    int leftChild= (2*index)+1; int rightChild=(2*index)+2;
    while ((leftChild<size|| rightChild<size)&& (myList[index]< myList[leftChild] || myList[index]< myList[rightChild] ) )
    {
        if(rightChild<size)
        {
            if(leftChild<size)
           {
              bool right= ( myList[leftChild]< myList[rightChild] )?true:false;
             if(right  )
             {
               swap(&myList[index],&myList[rightChild]);
               index=rightChild;
               leftChild= (2*index)+1;
               rightChild=(2*index)+2;
              }
             else
              {
                swap(&myList[index],&myList[leftChild]);
                index=leftChild;
                leftChild= (2*index)+1;
               rightChild=(2*index)+2;
              }
             }
            else  //right child
            {
              swap(&myList[index],&myList[rightChild]);
              index=rightChild;
              leftChild= (2*index)+1;
              rightChild=(2*index)+2;
             }
        }
        else  // swap with left child
        {
            swap(&myList[index],&myList[leftChild]);
            index=leftChild;
            leftChild= (2*index)+1;
            rightChild=(2*index)+2;
        }


    }
}
void HomePage::sort(QList<QString>& myList)  //O(n log(n))
{

    int size= myList.count();
    while(size>1)
    {
        swap(&myList[0],&myList[size-1]);
        size--;
        siftDown(myList,size);

    }

}



void HomePage::internalSearch (QString x,QList<QString> &myList,int begin,int end,QList<QString> &temp)
{

    if(begin>end)  {temp.append("\0");return  ;} //not found
    int middle= (begin+end)/2; bool indicator=false;
    //qDebug()<<test<<endl;
    while(myList.size() != 0 && middle < myList.size()&& myList[middle].mid(0,x.length())==x)
    {

        if (middle!=0&& indicator==false &&myList[middle-1].mid(0,x.length())==x ) temp.append(myList[middle-1]);
        temp.append(myList[middle]);
        myList.removeAt(middle);
        indicator=true;

    }
    if(indicator) return ;
    if(myList[middle][0]> x[0])
    {

        internalSearch ( x, myList,begin,middle-1,temp);
    }
   else
        internalSearch ( x, myList,middle+1,end,temp);



}


QList<QString> HomePage::search (QString x,QList<QString> & myList)
{
    for(int i=1;i<myList.count();i++) // heapify the list O( n log(n) )
     {
         siftUp(myList,i);
     }

      sort(myList);
      QList<QString> temp;
      internalSearch(x,myList,0,myList.count()-1,temp);
      return temp;
}


void HomePage::on_friendSearch_textChanged(const QString &arg1)
{
    QString name= ui->friendSearch->text();
    if(name=="")return;
    ui->comboBox->setVisible(true);
    QList<QString> list= currentSessionUser->userFileManipulator.readEmails();           //yomna's list of mails
    QList<QString >optionsList=search(name,list);
    ui->comboBox->addItem("Search Results");
    for(int i=0;i<optionsList.size();i++)
    {
        ui->comboBox->addItem(optionsList[i]);
    }

}

void HomePage::on_comboBox_currentIndexChanged(int index)
{
    QList<QObject*> items = ui->comboBox->children();
    if(index == -1)
        return ;
    QString mail = ui->comboBox->itemText(index);
    if(mail == "Search Results")
        return;
    QList<QObject*> oldPosts = ui->scrollAreaWidgetContents->children();
    for(int i = 1; i < oldPosts.size(); i++)
        delete oldPosts[i];
    ProfilePage * profilePageWindow=new ProfilePage;
    profilePageWindow->show();
    this->hide();
    profilePageWindow->setHomePageOwnerMail(currentSessionUser->userName);
    currentSessionUser->userName = mail;
    profilePageWindow->setCurrentSessionUser_Ptr(currentSessionUser);

}


QList<QString> *HomePage::randomPosts(QString mail,int number)
{
   fileman A;
  QList<QString> friendsMail=A.getFriends( mail);
  friendsMail.push_front(currentSessionUser->userName);
  friendsMail.push_front(currentSessionUser->userName);
  friendsMail.push_front(currentSessionUser->userName);
  for(int i=1;i<friendsMail.count() - 1;i = i + 3)
  {

      QList<Post> *friendsPosts=currentSessionUser->userFileManipulator.getPosts_new(friendsMail[i]);
      for(int j=0; j<number ;j++)
      {
         if(j < friendsPosts->count())
          pagePosts->push_back((*friendsPosts)[j]);
      }
  }
}

void HomePage::on_actionLog_Out_triggered()
{
    qApp->quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}
