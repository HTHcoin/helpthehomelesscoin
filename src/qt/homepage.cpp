#include "homepage.h"
#include "ui_homepage.h"
#include "mainwindow.h"


homepage::homepage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::homepage)
{
    ui->setupUi(this);

    ui->scrollArea->setWidgetResizable(true);
}

homepage::~homepage()
{
    delete ui;
}

/*
method to set up the username
note that based on the username porvided this is how we extract all out information for the current user from our Database
since our username attribute is unique for everyone a valid input username will always present the correct profile requested
*/
void homepage::setUserName(QString input)
{
    username = input;

    // here we load all the required data for this user into our program
    loadProfile();
    loadNewsfeed();
    loadNotifications();
    loadFriends();
}

// method to load the profile information for the current user
void homepage::loadProfile()
{
    // connect to our database
    MainWindow database;
    database.openDatabase();

    // query to extract the information of the current user
    QSqlQuery query;

    if (query.exec("select * from User where Username='"+username+"'"))
    {
        while(query.next())
        {
            userId = query.value(0).toString();
            firstName = query.value(1).toString();
            lastName = query.value(2).toString();
            fullName = query.value(3).toString();
            username = query.value(4).toString();
            password = query.value(5).toString();
            birthday = query.value(6).toString();
            country = query.value(7).toString();
            gender = query.value(8).toString();
        }

        // we also update all the profile information to sync with the current user
        ui->lineEdit_Name->setText(firstName);
        ui->lineEdit_LastName->setText(lastName);
        ui->lineEdit_Username->setText(username);
        ui->lineEdit_Password->setText(password);
        ui->lineEdit_Birthday->setText(birthday);
        ui->lineEdit_Gender->setText(gender);
        ui->lineEdit_Country->setText(country);
        ui->label_welcome->setText("Welcome: " + fullName);

        database.closeDatabase();
    }
    else
    {
        qDebug() << "Error loading user data";
    }
}

// method to load the newsfeed of the current user
void homepage::loadNewsfeed()
{
    // connect to our database
    MainWindow database;
    database.openDatabase();

    // here we load up all the newsfeed history that corresponds to the current user
    // note that we also insert the post that corresponds to friends of this user
    // for example if fred is a friend of the current and fred made a post, his post will show in the newsfeed of current user
    // we also sort them from most recent post to latest
    mainLayout = new QVBoxLayout();

    QSqlQuery query;

    if (query.exec("select PostInfo from (select PostInfo,PostID from Post where UserId='"+userId+"' union select PostInfo,PostID from Post inner join FriendPair on UserID=User2ID and User1ID='"+userId+"' order by PostID desc)"))
    {
        while(query.next())
        {
            QTextEdit* label = new QTextEdit(query.value(0).toString());
            mainLayout->addWidget(label);
        }
    }
    else
    {
        qDebug() << "Error loading newsfeed";
    }

    // placing it inside our scroll area
    QWidget *mainWidget = new QWidget();
    mainWidget->setLayout(mainLayout);
    ui->scrollArea->setWidget(mainWidget);

    database.closeDatabase();
}

// method to load all the notifications for the current user
void homepage::loadNotifications()
{
    // connect to our database
    MainWindow database;
    database.openDatabase();

    // here we load all the notifications that correspond to this user
    QSqlQueryModel* result = new QSqlQueryModel();
    QSqlQuery* filter = new QSqlQuery();

    filter->exec("select Info from Notification where ReceiverID='"+userId+"'");

    result->setQuery(*filter);

    ui->listView_notifications->setModel(result);

    database.closeDatabase();
}

// method to load all the friends corresponding to this user
void homepage::loadFriends()
{
    // connect to our database
    MainWindow database;
    database.openDatabase();

    // here we load all the notifications that correspond to this user
    QSqlQueryModel* result = new QSqlQueryModel();
    QSqlQuery* filter = new QSqlQuery();

    filter->exec("select FullName from User inner join FriendPair on ID=User2ID where User1ID ='"+userId+"'");

    result->setQuery(*filter);

    ui->listView_friends->setModel(result);

    database.closeDatabase();
}

// method to exit the account page
void homepage::on_pushButton_logout_clicked()
{
    this->close();
}

// method operating the post feature
void homepage::on_pushButton_post_clicked()
{
    // we first get the input provided by the user
    QString postCreated = ui->textEdit_postArena->toPlainText();
    if (postCreated.isEmpty())
    {
        QMessageBox::warning(this,"Warning","Input field is empty, please create a post");
        return;
    }
    else
    {
        // editing the post so it includes the author at the start
        postCreated = fullName + " : " + postCreated;
    }

    // connect to our database
    MainWindow database;
    database.openDatabase();

    // we use a query to insert the new post in the corresponding table
    QSqlQuery query;

    if (query.exec("insert into Post (PostInfo,UserID) values ('"+postCreated+"','"+userId+"')"))
    {
        QTextEdit* newPost = new QTextEdit(postCreated);
        mainLayout->insertWidget(0, newPost);

        database.closeDatabase();
    }
    else
    {
        qDebug() << "Error inserting post";
    }
}

// method that allows the user to update either their password or country
void homepage::on_pushButton_saveChanges_clicked()
{
    // storing the inputs from the lines to later compare if changes where made
    QString name2 = ui->lineEdit_Name->text();
    QString lastName2 = ui->lineEdit_LastName->text();
    QString username2 = ui->lineEdit_Username->text();
    QString birthday2 = ui->lineEdit_Birthday->text();
    QString gender2 = ui->lineEdit_Gender->text();

    // getting the inputs from the password and country fields
    QString newPassword = ui->lineEdit_Password->text();
    QString newCountry = ui->lineEdit_Country->text();


    // condition to validate that the other fields were not changed
    if (firstName != name2 || lastName != lastName2 || username != username2 || birthday != birthday2 || gender != gender2)
    {
        // we reset the fields back to their defeault and prompt an error message
        ui->lineEdit_Name->setText(firstName);
        ui->lineEdit_LastName->setText(lastName);
        ui->lineEdit_Username->setText(username);
        ui->lineEdit_Birthday->setText(birthday);
        ui->lineEdit_Gender->setText(gender);
        ui->lineEdit_Password->setText(password);
        ui->lineEdit_Country->setText(country);

        QMessageBox::warning(this,"Warning", "Unable to make changes, you can only change your password or country");
    }
    else if (password != newPassword || country != newCountry)
    {
        // first we validate the password in the case it was changed
        // analyzing the password input
        if (newPassword.length() < 6)
        {
            QMessageBox::warning(this,"Warning", "Password must be atleast 6 characters long");
            ui->lineEdit_Password->setText(password);
            return;
        }

        bool hasUppercase = false;
        bool hasNumber = false;
        for (int x = 0; x < newPassword.length(); x++)
        {
            if ( newPassword[x] >= 'A' &&  newPassword[x] < 'Z')
            {
                hasUppercase = true;
            }

            if ( newPassword[x] >= '0' &&  newPassword[x] < '9')
            {
                hasNumber = true;
            }
        }

        // condition to check if password met the 2 requirements for including an uppercase letter and a number
        if (!hasNumber || !hasUppercase)
        {
            QMessageBox::warning(this,"Warning", "Password must include atleast 1 uppercase letter and atleast 1 number");
            ui->lineEdit_Password->setText(password);
        }
        else
        {
            // update their information
            // we also inform the user that they have made a change and update the record in the database
            password = newPassword;
            if (!newCountry.isEmpty())
            {
                country = newCountry;
            }

            ui->lineEdit_Password->setText(password);
            ui->lineEdit_Country->setText(country);

            // connect to our database
            MainWindow database;
            database.openDatabase();

            // query to update the country and/or password
            QSqlQuery query;

            if (query.exec("update User set Password='"+password+"',Country='"+country+"' where Username='"+username+"'"))
            {
                 QMessageBox::warning(this,"Warning", "Your account has been updated");
                 database.closeDatabase();
            }
            else
            {
                qDebug() << "Failed to make changes";
            }
        }
    }
}

// this method will remove their account from our database and prompt them back to the starting page
void homepage::on_pushButton_removeAccount_clicked()
{
    // connect to our database
    MainWindow database;
    database.openDatabase();

    // query to update the country and/or password
    QSqlQuery query;

    if (query.exec("delete from User where Username='"+username+"'"))
    {
        QMessageBox::warning(this,"Warning","Your account has been removed");

        // closing the database and returning back to the welcome window
        database.closeDatabase();
        this->close();
    }
    else
    {
        qDebug() << "Failed to remove account";
    }
}

// method to showcase people from our database based on the search input provided by the user
void homepage::on_pushButton_search_clicked()
{
    // first we get the string input from the users present in the search bar
    QString target = ui->lineEdit_search ->text();

    // condition to check that an input is present before attempting to use the search feature
    if (target.isEmpty())
    {
        QMessageBox::warning(this,"Warning", "Please enter either a name or country in the search field.");
        return;
    }
    else
    {
        // add the wildcard characters to aid in our search filter
        target.push_front('%');
        target.push_back('%');
    }

    // connect to our database
    MainWindow database;
    database.openDatabase();

    /*
    for this section we are passing the filtered data as an output for the user
    the search will find any name or country that has the a similarity to the input provided
    if no user from our database is found based on the input entered, the user will be informed that no results were found
    the search will only showcase users that are not currently friends with the current user that is logged in
    so if mike is already a friend of lucas, and the user enters that same lucas; lucas will not be showned
    */
    QSqlQueryModel* result = new QSqlQueryModel();
    QSqlQuery* filter = new QSqlQuery();

    filter->exec("select FullName from User where (FullName like '"+target+"' or Country like '"+target+"') and (Username!='"+username+"') and (ID not in (select User2ID from FriendPair where User1ID='"+userId+"'))");

    result->setQuery(*filter);

    ui->listView_searchResult->setModel(result);

    // condition to signal that no results where found based on their search input
    if (ui->listView_searchResult->model()->rowCount() == 0)
    {
        QMessageBox::warning(this,"Warning", "No Results Found "+target);
    }

    database.closeDatabase();
}

// method to send a friend request to another user in our network
void homepage::on_pushButton_sendFriend_clicked()
{
    // getting the selected user from the search results
    QModelIndex index = ui->listView_searchResult->currentIndex();
    QString targetUser = index.data(Qt::DisplayRole).toString();

    if (targetUser.isEmpty())
    {
        QMessageBox::warning(this,"Warning", "please select a user to send a request to");
        return;
    }

    // get the userID for the target user
    QString targetID;

    // connect to our database
    MainWindow database;
    database.openDatabase();
    QSqlQuery query;

    if (query.exec("select ID from User where FullName='"+targetUser+"'"))
    {
        while(query.next())
        {
            targetID = query.value(0).toString();
        }

        // creating the message being send
        QString request = fullName + " wants to be your friend";

        // here we now create a notification bridge between the 2 users
        // we also insert the information into our database so the target user can later extract the info from their notification tab
        if (query.exec("insert into Notification (Info,SenderID,ReceiverID) values ('"+request+"','"+userId+"','"+targetID+"')"))
        {
            QMessageBox::warning(this,"Warning", "Your friend request has been sent to " + targetUser);
        }
        else
        {
            qDebug() << "Error your request was not send";
        }
    }
    else
    {
        qDebug() << "Error failed to send friendRequest";
    }
}

// method for accepting a friendRequest
void homepage::on_pushButton_acceptNotification_clicked()
{
    // getting the selecting notification
    QModelIndex index = ui->listView_notifications->currentIndex();
    QString currentNotification = index.data(Qt::DisplayRole).toString();

    if (currentNotification.isEmpty())
    {
        QMessageBox::warning(this,"Warning", "please select a notification");
        return;
    }

    // get the userID and name from the person that sent you this request
    // we also extract the notification ID to remove it once the request has been answer
    QString senderID;
    QString notificationID;
    QString senderName;

    // connect to our database
    MainWindow database;
    database.openDatabase();
    QSqlQuery query;

    if (query.exec("select NotificationID,SenderID from Notification where Info='"+currentNotification+"' and ReceiverID='"+userId+"'"))
    {
        while(query.next())
        {
            notificationID = query.value(0).toString();
            senderID = query.value(1).toString();
        }

        // query to extract the name of the sender
        query.first();
        if (query.exec("select FullName from User where ID='"+senderID+"'"))
        {
            while(query.next())
            {
                senderName = query.value(0).toString();
            }
        }
        else
        {
            qDebug() << "Error failed to retrieve the sender name";
            return;
        }

        // we now create a connection between the 2 users
        // this officially makes them both friends
        if (query.exec("insert into FriendPair (User1ID,User2ID) values ('"+userId+"','"+senderID+"')"))
        {
            QMessageBox::warning(this,"Warning","You are now friends with "+ senderName);
        }
        else
        {
            qDebug() << "Error unable to accept the friend request";
            return;
        }

        // note since this is bi connection we create another record
        // reason for this is to make it more accesible to extract who is friends with who
        query.exec("insert into FriendPair (User1ID,User2ID) values ('"+senderID+"','"+userId+"')");

        // we remove the notification from our database
        if (query.exec("delete from Notification where NotificationID='"+notificationID+"'"))
        {
            // we update both our notifications and friends list after accepting the request
            loadNotifications();
            loadFriends();
        }
        else
        {
            qDebug() << "Error failed to remove the notifcation";
        }

        database.closeDatabase();
    }
    else
    {
        qDebug() << "Error failed to get the sender ID";
    }
}

// method for ignoring a notification which in the end will delete it from our database
void homepage::on_pushButton_ignoreNotification_clicked()
{
    // getting the current notification selected
    QModelIndex index = ui->listView_notifications->currentIndex();
    QString currentNotification = index.data(Qt::DisplayRole).toString();

    if (currentNotification.isEmpty())
    {
        QMessageBox::warning(this,"Warning", "please select a notification");
        return;
    }

    // get the notification ID to remove it
    QString senderID;
    QString notificationID;

    // connect to our database
    MainWindow database;
    database.openDatabase();
    QSqlQuery query;

    if (query.exec("select NotificationID from Notification where Info='"+currentNotification+"' and ReceiverID='"+userId+"'"))
    {
        while(query.next())
        {
            notificationID = query.value(0).toString();
        }

        // we remove the notification from our database
        if (query.exec("delete from Notification where NotificationID='"+notificationID+"'"))
        {
            // we update our notification view after deleting it
            loadNotifications();
        }
        else
        {
            qDebug() << "Error failed to remove the notifcation";
        }

        database.closeDatabase();
    }
    else
    {
        qDebug() << "Error failed to get the notification ID";
    }
}

// method to remove a friend from your list
void homepage::on_pushButton_removeFriend_clicked()
{
    // getting the current friend selected
    QModelIndex index = ui->listView_friends->currentIndex();
    QString currentFriend = index.data(Qt::DisplayRole).toString();

    if (currentFriend.isEmpty())
    {
        QMessageBox::warning(this,"Warning", "please select a friend from your list");
        return;
    }

    // get the friend ID to remove it
    QString friendID;

    // connect to our database
    MainWindow database;
    database.openDatabase();
    QSqlQuery query;

    if (query.exec("select ID from User where FullName='"+currentFriend+"'"))
    {
        while(query.next())
        {
            friendID = query.value(0).toString();
        }

        // we remove the pair from our database
        // once removed there is no connection between the 2 users
        if (query.exec("delete from FriendPair where User1ID='"+userId+"' and User2ID='"+friendID+"'"))
        {
            query.exec("delete from FriendPair where User1ID='"+friendID+"' and User2ID='"+userId+"'");

            QMessageBox::warning(this,"Warning",currentFriend + " has been removed from your list");

            // we update both our friend list and our newsfeed after removing that friend
            loadNewsfeed();
            loadFriends();
        }
        else
        {
            qDebug() << "Error failed to remove the friend";
        }

        database.closeDatabase();
    }
    else
    {
        qDebug() << "Error failed to get the friend ID";
    }
}
