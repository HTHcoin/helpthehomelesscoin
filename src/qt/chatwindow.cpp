/*
* Copyright (C) 2016 Joe 'raizor' McChristmas
* Copyright (C) 2009 Cleriot Simon
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA*/

#include "chatwindow.h"
#include "ui_chatwindow.h"

#include <QLabel>
#include <QListWidget>
#include <QStackedWidget>
#include <QLineEdit>
#include <QPushButton>

ChatTabHolder::ChatTabHolder(QString nickname, ChatWindow* win, QWidget *parent) : QTabWidget(parent)
{
    nick = nickname;
    serverName = "chat.freenode.net";
    this->setTabsClosable(true);
    parentWindow = win;
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(CloseTab(int)));
}

void ChatTabHolder::CloseTab(int index)
{
    ChatTab* chatTab = (ChatTab*)this->widget(index);
    //chatTab->ChannelTabClosed();
    servers[serverName]->sendData(servers[serverName]->parseCommande("/part "+chatTab->tabName, true) );
    if (chatTab->isServer)
    {
        parentWindow->disconnectFromServer();
    }
    this->removeTab(index);
}

void ChatTab::UpdateUserList()
{
    if (this->userView)
    {
        users.sort();
        delete modelUsers;
        modelUsers = new QStandardItemModel();

        for(int i=0; i<users.count(); i++)
        {            
            QString username = users[i];

            //if (username.startsWith("@") || username.startsWith("+"))
            //    username.remove(0, 1);

            QString col = ircServer->nickColorHex(username);

            QStandardItem *colorItem = new QStandardItem(username);
            colorItem->setData(QBrush(QColor(col)),Qt::ForegroundRole);
            modelUsers->appendRow(colorItem);

            //QModelIndex vIndex = chanTab->modelUsers->index(i, 0);
            //chanTab->modelUsers->setData(vIndex, QBrush(Qt::red), Qt::ForegroundRole);
        }
        userView->setModel(this->modelUsers);
        userView->update();
    }
}

// find nick in userlist (with +v, @ decoration)
QString ChatTab::FormatNick(QString nick)
{
    foreach(QString s, users)
    {
        if (s.length() == nick.length() + 1 && s.endsWith(nick))
            return s;
    }
    return nick;
}

ChatTab::ChatTab(ChatTabHolder* tabHolder, QString tabName, QWidget *parent, IrcServer* server)
    : QWidget(parent)
{
    modelUsers = 0;
    updateUsers = true; // always update new tab
    this->tabName = tabName;
    QWidget* holder = this;
    QGridLayout* gridLayout = new QGridLayout();

    // create vertical layout to hold hlayout (editor/userlist) on top and line edit chat box below
    QVBoxLayout* vlayout = new QVBoxLayout();

    // add horizontal split with channel edit on left and user list on right
    QHBoxLayout* hlayout = new QHBoxLayout();

    // add text edit to left and make it fill spare width
    textEdit=new QTextEdit();
    textEdit->setReadOnly(true);
    QSizePolicy spLeft(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spLeft.setHorizontalStretch(1);
    textEdit->setSizePolicy(spLeft);
    hlayout->addWidget(textEdit);

    isServer = !server;

    if (server)
    {
        // add userlist on right. will be auto-constrained by sizing of text edit on left
        userView = new QListView();
        QFont f(userView->font());
        f.setBold(true);
        userView->setFont(f);
        hlayout->addWidget(userView);
    }

    // add the horizontal layout to vertical layout
    vlayout->addLayout(hlayout);
    gridLayout->addLayout(vlayout, 0, 0, 1, 1);

    // and add the edit line to bottom of vertical layout
    lineEdit = new QLineEdit();
    lineEdit->setObjectName(QLatin1String("lineEdit"));
    lineEdit->setGeometry(QRect(90, 230, 701, 30));
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(1);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(lineEdit->sizePolicy().hasHeightForWidth());
    lineEdit->setSizePolicy(sizePolicy);
    lineEdit->setMaximumSize(QSize(16777215, 30));
    vlayout->addWidget(lineEdit);

    holder->setLayout(gridLayout);

    if (!server)
    {
        // create server
        server=new IrcServer();
        server->pseudo=tabHolder->nick;
        server->serverName=tabHolder->serverName;
        server->port=6667;
        server->editor=textEdit;
        server->tab=tabHolder;
        server->parent=tabHolder;

        tabHolder->servers.insert(tabHolder->serverName, server);
        // connect event signals
        connect(server, SIGNAL(joinTab()),this, SLOT(tabJoined() ));
        connect(server, SIGNAL(tabJoined()),this, SLOT(tabJoining() ));
        server->connectToHost(tabHolder->serverName, 6667);        

        // add server status tab to conversations
        server->conversations.insert(server->serverName, this);
    }

    connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(sendCommand()));

    ircServer = server;
    holder = tabHolder;
}

ChatTab* ChatTabHolder::ChannelTab(QString channelName)
{
    for (int i=0; i<this->count(); i++)
    {
        ChatTab* tab = (ChatTab*)this->widget(i);
        if (QString::compare(tab->tabName, channelName, Qt::CaseInsensitive) == 0)
            return tab;
    }
    // return status tab
    return (ChatTab*)this->widget(0);
}

ChatTab::~ChatTab()
{
    // todo
}

ChatTabHolder::~ChatTabHolder()
{
    // todo
}


ChatWindow::ChatWindow(QWidget *parent)
    : QWidget(parent), ui(new Ui::ChatWindowClass)
{
    ui->setupUi(this);
    tabsChannels = 0;
    connectOptionsLayout = 0;
    showConnectOptions();
}

void ChatWindow::showConnectOptions()
{
    this->connectOptionsLayout = new QWidget();

    // main label
    QLabel* label_2 = new QLabel();
    label_2->setObjectName(QLatin1String("label_2"));
    label_2->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    label_2->setText(QApplication::translate("ChatWindowOldClass", "<html><head/><body><p>A few IRC commands :</p><ul style=\"margin-top: 0px; margin-bottom: 0px; margin-left: 0px; margin-right: 0px; -qt-list-indent: 1;\"><li style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">/JOIN #channel <span style=\" color:#9a9a9a;\">Join a channel</span></li><li style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">/NICK nickname <span style=\" color:#9a9a9a;\">Change nickname</span></li><li style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">/PRIVMSG nickname message <span style=\" color:#9a9a9a;\">Send PM </span></li><li style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">If userlist gets buggy, switch tabs to fix</li></ul></body></html>", 0));
    QSizePolicy spLeft(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spLeft.setHorizontalStretch(1);
    label_2->setSizePolicy(spLeft);

    editPseudo = new QLineEdit();
    editPseudo->setObjectName(QLatin1String("editPseudo"));
    //editPseudo->setGeometry(QRect(470, 250, 151, 22));

    QLabel* label_5 = new QLabel();
    label_5->setObjectName(QLatin1String("label_5"));
    //label_5->setGeometry(QRect(370, 250, 61, 21));
    label_5->setText(QApplication::translate("ChatWindowOldClass", "Nickname", 0));

    QPushButton* buttonConnect = new QPushButton();
    buttonConnect->setObjectName(QLatin1String("buttonConnect"));
    buttonConnect->setGeometry(QRect(470, 300, 151, 41));
    // buttonConnect->setStyleSheet(QLatin1String("border-radius:40px;border:1px solid grey"));
    buttonConnect->setText(QApplication::translate("ChatWindowOldClass", "Connect to IRC", 0));

    QLabel* label_10 = new QLabel();
    label_10->setObjectName(QLatin1String("label_10"));
    //label_10->setGeometry(QRect(370, 200, 161, 21));
    label_10->setText(QApplication::translate("ChatWindowOldClass", "<html><head/><body><p><span style=\" font-size:12pt;\">Connect to IRC :</span></p></body></html>", 0));
    label_10->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    label_10->setMinimumHeight(30);

    // add horizontal split with channel edit on left and user list on right
    QHBoxLayout* hlayout = new QHBoxLayout();

    QWidget*  gridHolder= new QWidget();
    gridHolder->setMaximumWidth(250);

    QGridLayout* glayout = new QGridLayout();
    glayout->addWidget(label_10, 0, 0, 2, 2);
    glayout->addWidget(label_5, 1, 0, 1, 1);
    glayout->addWidget(editPseudo, 1, 1, 1, 1);
    glayout->addWidget(buttonConnect, 2, 0, 2, 2);
    hlayout->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);

    // add left spacer so contents are centered
    QWidget* leftSpacer = new QWidget();
    leftSpacer->setSizePolicy(spLeft);
    hlayout->addWidget(leftSpacer);

    hlayout->addWidget(label_2);

    gridHolder->setLayout(glayout);
    hlayout->addWidget(gridHolder);

    // add right spacer so contents are centered
    QWidget* rightSpacer = new QWidget();
    rightSpacer->setSizePolicy(spLeft);
    hlayout->addWidget(rightSpacer);


    connectOptionsLayout->setLayout(hlayout);
    ui->verticalLayout->addWidget(connectOptionsLayout);

    connect(buttonConnect, SIGNAL(clicked()),this, SLOT(on_buttonConnect_clicked()));
}

void ChatTab::sendCommand()
{
    if(isServer) {
        ircServer->sendData(ircServer->parseCommande(lineEdit->text(), true) );
    }
    else {
        ircServer->sendData(ircServer->parseCommande(lineEdit->text()) );
    }

    lineEdit->clear();
    lineEdit->setFocus();
}

void ChatWindow::tabChanged(int index)
{
   // if(index != 0 && joining == false)
   //     currentTab()->updateUsersList(ui->tab->tabText(index));
}

void ChatWindow::tabClosing(int index)
{
    //currentTab()->leave(ui->tab->tabText(index));
}
/*void ChatWindow::tabRemoved(int index)
{
    currentTab()->leave(ui->tab->tabText(index));
}*/

void ChatWindow::disconnectFromServer()
{
    QMapIterator<QString, IrcServer*> i(this->tabsChannels->servers);
    while(i.hasNext())
    {
        i.next();
        QMapIterator<QString, ChatTab*> i2(i.value()->conversations);

        while(i2.hasNext()) {
            i2.next();
            i.value()->sendData("QUIT " + i2.key() + " ");
        }
    }

    // remove the tab holder
    ui->verticalLayout->removeWidget(tabsChannels);

    // recreate connection options
    showConnectOptions();

    //ui->splitter->hide();
    //ui->hide3->show();

}

IrcServer *ChatWindow::getServer()
{
    //QString tooltip=ui->tab->tabToolTip(ui->tab->currentIndex());
    return this->tabsChannels->servers[0];
    //return ui->tab->currentWidget()->findChild<Serveur *>();
}

void ChatWindow::closeTab()
{
    QString tooltip=this->tabsChannels->tabToolTip(this->tabsChannels->currentIndex());
    QString txt=this->tabsChannels->tabText(this->tabsChannels->currentIndex());
    //QString tooltip=ui->tab->tabToolTip(ui->tab->currentIndex());
    //QString txt=ui->tab->tabText(ui->tab->currentIndex());
/*
    if(txt==tooltip) {
        QMapIterator<QString, QTextEdit*> i(servers[tooltip]->conversations);
        int count=ui->tab->currentIndex()+1;
        while(i.hasNext()) {
            i.next();
            ui->tab->removeTab(count);
        }
        currentTab()->abort();
        ui->tab->removeTab(ui->tab->currentIndex());
    }
    else {
        ui->tab->removeTab(ui->tab->currentIndex());
        currentTab()->conversations.remove(txt);
    }*/
}


void ChatWindow::tabJoined()
{
    joining = true;
}

void ChatWindow::tabJoining()
{
    joining = false;
}

void ChatWindow::connectToServer()
{
    if (!this->tabsChannels)
    {
        this->tabsChannels = new ChatTabHolder(editPseudo->text(), this);
    }

    // remove connections options
    if (connectOptionsLayout)
    {
        /*
        QLayoutItem *item;
        while ((item = ui->verticalLayout->takeAt(0)) != 0)
            delete item;
        delete connectOptionsLayout;
        */
        connectOptionsLayout->hide();
    }

    ui->verticalLayout->addWidget(tabsChannels);

    QString chan = tabsChannels->serverName;
    ChatTab* tab = new ChatTab(tabsChannels, chan);
    int idx = tabsChannels->currentIndex()+1;
    int index=tabsChannels->insertTab(idx, tab, chan);
    tabsChannels->setCurrentIndex(index);
}

void ChatWindow::closeEvent(QCloseEvent *event)
{
    (void) event;

    QMapIterator<QString, IrcServer*> i(tabsChannels->servers);

    while(i.hasNext())
    {
        i.next();
        QMapIterator<QString, ChatTab*> i2(i.value()->conversations);

        while(i2.hasNext()) {
            i2.next();
            i.value()->sendData("QUIT " + i2.key() + " ");
        }
    }
}

void ChatWindow ::setModel(ClientModel *model)
{
    this->model = model;
}

ChatWindow::~ChatWindow()
{
    delete ui;
    return;
    /*
    QMapIterator<QString, IrcServer*> i(tabsChannels->servers);
    while(i.hasNext())
    {
        i.next();
        QMapIterator<QString, ChatTab*> i2(i.value()->conversations);
        while(i2.hasNext()) {
            i2.next();
            i.value()->sendData("QUIT " + i2.key() + " ");
        }
    }
    */
}

void ChatWindow::on_buttonConnect_clicked()
{
    this->connectToServer();
}
