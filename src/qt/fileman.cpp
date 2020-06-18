#include "fileman.h"
#include<user.h>

int numberOfUsers=0;
QList <QString> emailss;
fileman::fileman()
{
  //name=username;
  path="";

}
void fileman:: TESTTEST()
{
    QString line;
    QFile in(path+name);
    if(!in.open(QFile::ReadOnly| QFile::Text))
        qDebug()<<"COULD NOT OPEN YOMNA SAN";
    QTextStream innn(&in);
      line=innn.readLine();
      qDebug()<<line;
    in.close();
}


// THE GET POSTS FUNCTIONS SHOULD BE EDITED SO THAT THE POSTS ARE NOT QSTRINGS, BUT OBJECTS OF COMMENTS AND STUFF
QString fileman:: getPostByDate(QString Date){
  QString line;
  QFile in(path+name);
  QTextStream innn(&in);
    line=innn.readLine();
 //   qDebug()<<line;
  QString result="";


  //bool begin_tag = false;
  if(!in.open(QFile::ReadOnly| QFile::Text))
//qDebug()<<"COULD NOT OPEN YA BENTY";
  line=innn.readLine();
  while (line!=NULL)
   {
     if(line=="<postdate>")
     {

       line=innn.readLine();
       if(line==Date){
           line=innn.readLine();
         while(line!=NULL){

           if(line=="<posttext>"){
             line=innn.readLine();
             while(line!="</posttext>")
             {
               // cout<<line<<endl;
                // qDebug()<<line;
               result+=line;
               line=innn.readLine();

             }
           }

           line=innn.readLine();
         }

       }
     }
     line=innn.readLine();
   }
   return result;
 }


QList<Post>* fileman:: getPosts(QString email){
  QString line;
  QFile in(path+"Users/"+email+".xml");
  QTextStream innn(&in);
  QList <Post> *posts = new QList <Post>;
  QString temp="";
  if(!in.open(QFile::ReadOnly| QFile::Text))
      qDebug()<<"COULD NOT OPEN YA BENTY";
  //bool begin_tag = false;
  line=innn.readLine();
  while (line!=NULL)
  {

    if(line=="<posttext>"){
      temp="";
      line=innn.readLine();
      while(!(line=="</posttext>")  )
      {
      //cout<<line<<endl;
      temp+=line;
      line=innn.readLine();
      if(line==NULL) break;

      }
      QString now;
      Post tempPost = Post(name,temp,now);
      posts->append(tempPost);

    }
    line=innn.readLine();
}
  return posts;
}

 QString fileman:: getUserNameByEmail(QString email)
  {
     QFile in(path+"Users/"+email+".xml");
     QTextStream f (&in);
     if(!in.open(QFile::ReadOnly| QFile::Text))
         qDebug()<<"COULD NOT OPEN the file ";
     QString line=f.readLine();
    while(1)
    {
        if(line=="<profilename>")
        {
            line=f.readLine();
           // qDebug()<<line;
            return line;
        }



         line=f.readLine();
         if(line=="</profilename>") break;


    }



  }

void fileman:: createFile( QString passWord,QString email,QString userName,QString date )
{
    emailss.push_back(email);
    numberOfUsers++;
    QString newFileStamp="<profilename>\n"+userName+"\n</profilename>\n"+"<email>\n"+email+"\n</email>\n<date>\n"+date+"\n</date>\n<password>\n"+passWord+"\n</password>\n<friends>\n</friends>\n<numberOfLikes>\n0\n</numberOfLikes>\n<numberOfComments>\n0\n</numberOfComments>";
    QFile file(path+"Users/"+email+".xml");
    if ( file.open(QIODevice::WriteOnly | QIODevice::Text) )
    {
        QTextStream stream( &file );
        stream << newFileStamp;
    }

    addEmail(email);

}

void fileman:: addPost(QString userPost, QString Date, QString name)
{
   // QString newPost=readFile("addPost.txt");

    QFile i(path+name+".txt");
    QTextStream inn(&i);

    if(!i.open(QFile::ReadOnly| QFile::Text))
        qDebug()<<"COULD NOT OPEN YA BENTY";
    QString userFile=inn.readAll(); //This file is with norhaaaan doko kana wakaranai
    i.close();



    QFile in(path+"addPost.txt");
    QTextStream innn(&in);

    if(!in.open(QFile::ReadOnly| QFile::Text))
        qDebug()<<"COULD NOT OPEN YA BENTY";
    QString newPost=innn.readAll(); //This file is with norhaaaan doko kana wakaranai
    in.close();

    int indexText= newPost.indexOf("<posttext>");  //+10
        newPost.insert(indexText+11,userPost);

    int indexDate=newPost.indexOf("<postdate>");
    newPost.insert(indexDate+11,Date);
        indexText= userFile.lastIndexOf("</post>");
        userFile.insert(indexText+8,newPost);
        QFile file(path+name+".txt");
           if ( file.open(QIODevice::ReadWrite) )
           {
               QTextStream stream( &file );
               stream << userFile << endl;
           }
           else
               qDebug()<< "couldn't open file";

            file.close();

}


void fileman:: addComment(QString userComment){

    QString commentStamp="\n<comment>\n<commenttext>\n"+userComment+"\n</commenttext>\n<commentname>\n</commentname>\n</comment>\n";
    //now lets open the user file to insert el bta3 da

    QFile i(path+"addUser.txt");
    QTextStream inn(&i);

    if(!i.open(QFile::ReadOnly| QFile::Text))
        qDebug()<<"COULD NOT OPEN YA BENTY";
    QString userFile=inn.readAll(); //This file is with norhaaaan doko kana wakaranai
    i.close();
    //Now we have all the user file content in userFile
    int index=userFile.indexOf("</posttext>");
    userFile.insert(index,commentStamp);
   // qDebug()<<userFile;


    //now rewrite to the file
    QFile file(path+name);
       if ( file.open(QIODevice::ReadWrite) )
       {
           QTextStream stream( &file );
           stream << userFile << endl;
       }
       else
          // qDebug()<< "couldn't open file";

        file.close();


}

void fileman:: addCommentByPostDate(QString userComment, QString Date, QString nameComment,QString email)
{
    QString commentStamp="<comment>\n<commenttext>\n"+userComment+"\n</commenttext>\n<commentname>\n"+nameComment+"\n</commentname>\n</comment>\n";
    //now lets open the user file to insert el bta3 da

    QFile i(path+"Users\\"+email+".xml");
    QTextStream inn(&i);

    if(!i.open(QFile::ReadOnly| QFile::Text))
        qDebug()<<"COULD NOT OPEN YA BENTY";
    QString userFile=inn.readAll(); //This file is with norhaaaan doko kana wakaranai
    i.close();
//now insert fe el userFile how is that :)
    QString copy=userFile;
int from=0;
while(from<=userFile.count()){
   int indexOfPostDate=userFile.indexOf("<postdate>",from);//qDebug()<<"hiiiiiiiiii";
QString line="";int j=11;
line=userFile.mid(indexOfPostDate+11,Date.count());
//userFile.insert(indexOfPostDate+j,commentStamp);

//qDebug()<<line;

if(line==Date){
    qDebug()<<"I FOUND YOUR DATE"+line;
    j=13;
    userFile.insert(indexOfPostDate+Date.count()+j+11,commentStamp);
   // qDebug()<<userFile;
    break;
}
else{
   // qDebug()<<"I COULDN'T FIND SORRYYYYY"+line;
}
from=from+indexOfPostDate+11;
}
QFile file(path+"Users/"+email+".xml");
   if ( file.open(QIODevice::ReadWrite) )
   {
       QTextStream stream( &file );
       stream << userFile << endl;
   }
   else
      // qDebug()<< "couldn't open file";

    file.close();


}

void fileman:: addFriend(QString friendName, QString name){}

void fileman:: addPost_new(QString userPost, QString Date, QString email){


     QFile i(path+"Users/"+email+".xml");
     QTextStream inn(&i);

     if(!i.open(QFile::ReadOnly| QFile::Text))
         qDebug()<<"COULD NOT OPEN YA BENTY";
     QString userFile=inn.readAll(); //This file is with norhaaaan doko kana wakaranai
     i.close();

     QString postStamp="\n<post>\n<postdate>\n"+Date+"\n</postdate>\n<posttext>\n"+userPost+"\n</posttext>\n"+"<like>\n</like>\n"+"</post>";

    int indexText= userFile.indexOf("</password>");
        userFile.insert(indexText+11,postStamp);
         QFile file(path+"Users/"+email+".xml");
            if ( file.open(QIODevice::ReadWrite) )
            {
                QTextStream stream( &file );
                stream << userFile << endl;
            }
            else
               // qDebug()<< "couldn't open file";

             file.close();

}
//QList <Comment>* fileman::  getCommentsByPostDate(QString email,QString date)

void fileman::  getCommentsByPostDate(QString email,QString date){

    QString line;
    QFile in(path+"Users/"+email+".xml");
    QTextStream inn(&in);
    if(!in.open(QFile::ReadOnly| QFile::Text))
        qDebug()<<"COULD NOT OPEN YA BENTY";
    QString userFile=inn.readAll();
qDebug()<<userFile;
    QList <Comment> *comments = new QList <Comment>;
    QString temp="";
    if(!in.open(QFile::ReadOnly| QFile::Text))

     qDebug()<<"COULD NOT OPEN YA BENTY";
    int from=0;
    //while(from<=userFile.count()){
       int indexOfPostDate=userFile.indexOf("<postdate>",from);//qDebug()<<"hiiiiiiiiii";
    //QString line;
       int j=11+1+1;
    int indexOfCommentTag;
    line=userFile.mid(indexOfPostDate+j,date.count());
    //userFile.insert(indexOfPostDate+date.count()+j+11,"hiiiiii i am here");
   // qDebug()<<userFile;

    qDebug()<<line;

    if(line==date){
        qDebug()<<"I FOUND YOUR DATE111"+line;
        j=13;
        indexOfCommentTag=indexOfPostDate+date.count()+j+11;
        //break;
    }
    else{
        qDebug()<<"I COULDN'T FIND SORRYYYYY"+line;
    }
    from=from+indexOfPostDate+11;
    //}
    line=inn.readLine();
    qDebug()<<"HORAA"<<line;


}

void fileman:: addFriends(QString email,QString friendName){
    QString friendStamp="\n<friendName>\n"+friendName+"\n</friendName>";
    QFile i(path+"Users\\"+email+".xml");
    QTextStream inn(&i);
    if(!i.open(QFile::ReadOnly| QFile::Text))
        qDebug()<<"COULD NOT OPEN YA BENTY";
    QString userFile=inn.readAll(); //This file is with norhaaaan doko kana wakaranai
    i.close();
   int indexOfFriend=userFile.indexOf("<friends>");
    userFile.insert(indexOfFriend+9,friendStamp);
QFile file(path+"Users/"+email+".xml");
   if ( file.open(QIODevice::ReadWrite) )
   {
       QTextStream stream( &file );
       stream << userFile << endl;
   }
   else
      qDebug()<< "couldn't open file";

    file.close();

}
void fileman:: updateActivity(QString email, int numberOfLikes , int numberOfComments){
    QFile i(path+"Users\\"+email+".xml");
    QTextStream inn(&i);
    if(!i.open(QFile::ReadOnly| QFile::Text))
        qDebug()<<"COULD NOT OPEN YA BENTY";
    QString userFile=inn.readAll(); //This file is with norhaaaan doko kana wakaranai
    i.close();
    int indexOfLikes=userFile.indexOf("<numberOfLikes>");
  /*  int ii=16;
    QChar x=userFile[indexOfLikes+ii];

    while(x!='\n'){
        userFile[indexOfLikes+ii]='NULL';
        ii++;
        x=userFile[indexOfLikes+ii];

    }
*/
    userFile.insert(indexOfLikes+15,"\n"+QString::number(numberOfLikes));
    int indexOfComments=userFile.indexOf("<numberOfComments>");
    userFile.insert(indexOfComments+18,"\n"+QString::number(numberOfComments));
    QFile file(path+"Users/"+email+".xml");
       if ( file.open(QIODevice::ReadWrite) )
       {
           QTextStream stream( &file );
           stream << userFile << endl;
       }
       else
          qDebug()<< "couldn't open file";

        file.close();
}


void fileman:: networkFile(){
     QString newFileStamp="<numberOfUsers>\n"+QString::number(numberOfUsers)+"\n</numberOfUsers>\n";
    QFile file(path+"Users/"+"network"+".xml");
    if ( file.open(QIODevice::WriteOnly | QIODevice::Text) )
    {
        QTextStream stream( &file );
        stream << newFileStamp;
    }


}

void fileman::addUsers(){
    QFile i(path+"Users/network.xml");
    QTextStream inn(&i);
    int id=0;
    if(!i.open(QFile::ReadOnly| QFile::Text))
        qDebug()<<"COULD NOT OPEN YA BENTY";
    QString userFile=inn.readAll(); //This file is with norhaaaan doko kana wakaranai
    i.close();
    QFile file(path+"Users/network.xml");
    fileman x;
    QList <QString> emails=x.readEmails();
for (int k = 0; k <emailss.count(); k++) {
   QString emailStamp="\n<email>\n"+emails[k]+"\n</email>\n<id>\n"+QString::number(id)+"\n</id>";
   id++;
   int indexText= userFile.indexOf("</numberOfUsers>");
       userFile.insert(indexText+16,emailStamp);
           if ( file.open(QIODevice::ReadWrite) )
           {
               QTextStream stream( &file );
               stream << userFile << endl;
           }
           else
               qDebug()<< "couldn't open file";

           file.close();

    }

}

void fileman::createEmailFile()
{
    QString stamp="<emails>\n</emails>";
    QFile f(path+"Users/"+"email"+".xml");
    if ( f.open(QIODevice::WriteOnly | QIODevice::Text) )
    {
        QTextStream stream( &f );
        stream << stamp;
    }
}
void fileman::addEmail(QString email){
    QFile i(path+"Users/"+"email"+".xml");
    QTextStream inn(&i);
    if(!i.open(QFile::ReadOnly| QFile::Text))
        qDebug()<<"COULD NOT OPEN YA BENTY";
    QString userFile=inn.readAll();
    i.close();
   int indexText= userFile.indexOf("<emails>");
       userFile.insert(indexText+8,"\n"+email);
        QFile file(path+"Users/"+"email"+".xml");
           if ( file.open(QIODevice::ReadWrite) )
           {
               QTextStream stream( &file );
               stream << userFile << endl;
           }
           else
               qDebug()<< "couldn't open file";

            file.close();

}
QList <QString> fileman:: readEmails(){
    QString line;
    QFile in(path+"Users/"+"email"+".xml");
    QTextStream innn(&in);
//    QList <QString> emails = new QList <QString>;
    QList <QString> emails;
    QString temp="";
    if(!in.open(QFile::ReadOnly| QFile::Text))
        qDebug()<<"COULD NOT OPEN YA BENTY";
    line=innn.readLine();

      if(line=="<emails>"){
        line=innn.readLine();
        while(1)
        {
            emails.append(line);
           // qDebug()<<line;
        line=innn.readLine();
        if(line=="</emails>") break;

       // if(line==NULL) break;

        }
      }

    return emails;

}

QList <QString> fileman:: getFriends(QString email)
{
    QList <QString> result;
    QFile in(path+"Users/"+email+".xml");
    QTextStream innn(&in);
    if(!in.open(QFile::ReadOnly| QFile::Text))
        qDebug()<<"COULD NOT OPEN YA BENTY";
    QString line=innn.readLine();
    while(1){
        if(line=="<friends>")
        {
            while(1)
            {
            line=innn.readLine();

            if(line=="</friends>") break;
            result.append(line);
            if(line=="</friendName>"||line=="<friendName>") continue;
            //qDebug()<<line;
            }

        }
        line=innn.readLine();
        if(line ==NULL) break;
    }


    return result;
}

void fileman::getActivity(QString email, int &numberOfPosts, int &numberOfLikes)
{
    QFile i(path+"Users\\"+email+".xml");
    QTextStream inn(&i);
    if(!i.open(QFile::ReadOnly| QFile::Text))
        qDebug()<<"COULD NOT OPEN YA BENTY";
    QString line =inn.readLine();
    while(1)
    {
        if(line=="<numberOfLikes>")
        {
            line=inn.readLine();
            numberOfLikes=line.toInt();
            break;
        }
        if(line=="<numberOfComments>")
        {
            line=inn.readLine();
            numberOfPosts=line.toInt();
            break;
        }
        line=inn.readLine();
        if(line=="</numberOfComments>")
            break;

    }
}

QList <Post> *fileman::getPosts_new(QString email)
{
    QList <Post> *result = new QList<Post>;



    QFile i(path+"Users\\"+email+".xml");
    QTextStream inn(&i);
    if(!i.open(QFile::ReadOnly| QFile::Text))
        qDebug()<<"COULD NOT OPEN YA BENTY";
    QString line=inn.readLine();
    while(1)
    {
        Post temp; temp.setPostText("");
        if(line=="<post>")
        {
            line=inn.readLine();
            while(1)
            {

                if(line=="<postdate>")
                {
                    line=inn.readLine();
                    temp.setPostDate(line);
                   // qDebug()<<temp.getPostDate();

                }
                if(line=="<posttext>")
                {
                    temp.setPostText("");
                    line=inn.readLine();
                    while(1)
                    {
                        if(line=="</posttext>")
                        {
                            break;
                        }
                        temp.setPostText(temp.getPostText()+line);
                      //  qDebug()<<temp.getPostText();
                        line=inn.readLine();


                    }

                }
                if(line=="<comment>")
                {

                    Comment tempCom;
                    line=inn.readLine();
                    while(1)
                    {
                        if(line=="<commenttext>")
                        {
                            tempCom.setCommentText("");
                            line=inn.readLine();
                            while(1)
                            {
                                tempCom.setCommentText(tempCom.getCommentText()+line);
                                line=inn.readLine();
                                if(line=="</commenttext>")break;
                            }
                        }
                        if(line=="<commentname>")
                        {
                            tempCom.setCommentOwner(inn.readLine());
                        }
                        line=inn.readLine();
                        if(line=="</comment>")break;

                    }
                    //qDebug()<<tempCom.getCommentOwner()<<tempCom.getCommentText();
                    temp.addComment(tempCom);

                }
                if(line=="<like>")
                {
                    line=inn.readLine();
                    while(line!="</like>")
                    {
                        if(line=="</like>") break;
                        if(line=="<name>"|| line=="</name>")
                        {
                            line=inn.readLine();
                            continue;
                        }

                        temp.getPostLikesOwnersVectorPtr()->push_back(line);

                      //  qDebug()<<line;
                        line=inn.readLine();
                       // qDebug()<<"LOOOOOOOP";


                    }


                }

                line=inn.readLine();
                if(line=="</post>")
                {
                    fileman x;
                    temp.setPostOwner(email);
                    result->append(temp);
                    break;
                }

            }
        }
        line=inn.readLine();
        if(line=="<friends>")break;
    }
return result;
}



void fileman ::addLikeByPostDate(QString email, QString Date, QString likePerson)
{
    QString likeStamp="\n<name>\n"+likePerson+"\n</name>";
    QFile i(path+"Users/"+email+".xml");
    QTextStream inn(&i);
    if(!i.open(QFile::ReadOnly| QFile::Text))
        qDebug()<<"COULD NOT OPEN YA BENTY";
    QString userFile=inn.readAll(); //This file is with norhaaaan doko kana wakaranai
    i.close();

    int from=0; int k=0;
    while(from<=userFile.count()){
       int indexOfPostDate=userFile.indexOf("<postdate>",from);//qDebug()<<"hiiiiiiiiii";
    QString line="";int j=11;

    line=userFile.mid(indexOfPostDate+11,Date.count());
//    userFile.insert(indexOfPostDate+j,likeStamp);

    //qDebug()<<line;

    if(line==Date){
        qDebug()<<"I FOUND YOUR DATE"+line;
        j=13;
    int endOfPostIndex= userFile.indexOf("</post>",from);
    int likeTagIndex= userFile.indexOf("<like>",from);

        userFile.insert(likeTagIndex+6,likeStamp);
        k++;

        qDebug()<<userFile;
        break;

    }
    else{
       // qDebug()<<"I COULDN'T FIND SORRYYYYY"+line;
    }
    from=from+indexOfPostDate+11;
    }
    QFile file(path+"Users/"+email+".xml");
       if ( file.open(QIODevice::ReadWrite) )
       {
           QTextStream stream( &file );
           stream << userFile << endl;
       }
       else
          // qDebug()<< "couldn't open file";

        file.close();
}

QString fileman::getPassword(QString email)
{
    QFile in(path+"Users/"+email+".xml");
    QTextStream f (&in);
    if(!in.open(QFile::ReadOnly| QFile::Text))
        qDebug()<<"COULD NOT OPEN the file ";
    QString line=f.readLine();
   while(1)
   {
       if(line=="<password>")
       {
           line=f.readLine();
          // qDebug()<<line;
           return line;
       }



        line=f.readLine();
        if(line=="</password>") break;


   }
}
