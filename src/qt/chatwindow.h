/*Copyright (C) 2009 Cleriot Simon
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

#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QtGui>
#include <QtNetwork>
#include "clientmodel.h"
#include "serveur.h"

namespace Ui
{
    class ChatWindowClass;
}

enum ChatWindowTabType
{
    CHATWINDOW_UNKNOWN         = 0,
    CHATWINDOW_SERVER          = 1,
    CHATWINDOW_CHANNEL         = 2
};

class IrcServer;
class ChatTab;
class QHBoxLayout;
class ChatWindow;

class ChatTabHolder : public QTabWidget
{
    Q_OBJECT
    public:
        ChatTabHolder(QString nickname, ChatWindow* win = 0, QWidget *parent = 0);
        ~ChatTabHolder();

        ChatTab* ChannelTab(QString channelName); // #channelname

        QMap<QString, IrcServer *> servers;
        QString nick;
        QString serverName;
        ChatWindow* parentWindow;

    public Q_SLOTS:
        void CloseTab(int index);

};

class ChatTab : public QWidget
{
    Q_OBJECT
    public:
        ChatTab(ChatTabHolder* tabHolder, QString tabName, QWidget *parent = 0, IrcServer* server = 0);
        ~ChatTab();

        void UpdateUserList();
        QString FormatNick(QString nick);

        QListView *userView;
        QLineEdit* lineEdit;
        QTextEdit *textEdit;
        IrcServer* ircServer;
        ChatTabHolder* holder;
        QStringList users;
        QStandardItemModel* modelUsers;
        QString tabName;
        bool updateUsers;
        bool isServer;


   public Q_SLOTS:
        void sendCommand();
};

class ChatWindow : public QWidget
{
    Q_OBJECT

public:
    ChatWindow(QWidget *parent = 0);
    ~ChatWindow();
    void setModel(ClientModel *model);
    void addNewServerTab();
    IrcServer * getServer();
    ChatTabHolder* tabsChannels;
    QLineEdit* editPseudo;
    QWidget* connectOptionsLayout;

Q_SIGNALS:
    void changeTab();

public Q_SLOTS:
    void connectToServer();
    void showConnectOptions();
    void closeTab();
    void tabChanged(int index);
    void tabJoined();
    void tabJoining();
    void disconnectFromServer();
    void tabClosing(int index);

private Q_SLOTS:
    void on_buttonConnect_clicked();

private:
    Ui::ChatWindowClass *ui;
    ClientModel *model;
    bool joining;
    void closeEvent(QCloseEvent *event);
};

#endif // CHATWINDOW_H
