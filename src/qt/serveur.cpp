/* Copyright (C) 2009 Cleriot Simon
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

#include "serveur.h"
#include "util.h"
#include "chatwindow.h"

#define IRC_STRIP_NICKS true
#define IRC_LOG_SERVER_INPUT true
#define IRC_LOG_SERVER_OUTPUT true

bool delist = true;

IrcServer::IrcServer()
{
    connect(this, SIGNAL(readyRead()), this, SLOT(readServeur()));
    connect(this, SIGNAL(connected()), this, SLOT(connected()));
    connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(errorSocket(QAbstractSocket::SocketError)));
}

void IrcServer::errorSocket(QAbstractSocket::SocketError error)
{
    switch(error)
    {
    case QAbstractSocket::HostNotFoundError:
        editor->append(tr("<em>ERROR : can't find freenode server.</em>"));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        editor->append(tr("<em>ERROR : server refused connection</em>"));
        break;
    case QAbstractSocket::RemoteHostClosedError:
        editor->append(tr("<em>ERROR : server cut connection</em>"));
        break;
    default:
        editor->append(tr("<em>ERROR : ") + this->errorString() + tr("</em>"));
    }
}

void IrcServer::connected()
{
    editor->append("Connecting...");
    sendData("USER "+pseudo+" localhost "+serverName+" :"+pseudo);
    sendData("NICK "+pseudo);
    editor->append("Connected to freenode.");
}

void IrcServer::joins()
{
    join("#cryptobullion");
}

void IrcServer::readServeur()
{
    QString message=QString::fromUtf8(this->readAll());
    QString currentChan=tab->tabText(tab->currentIndex());

    if(message.startsWith("PING :"))
    {
        QStringList liste=message.split(" ");
        QString msg="PONG "+liste.at(1);
        sendData(msg);
    }
    else if(message.contains("Nickname is already in use."))
    {
        pseudo=pseudo+"_2";
        pseudo.remove("\r\n");
        sendData("NICK "+pseudo);
        emit pseudoChanged(pseudo);
        writeToTab("-> Name changed to "+pseudo);
    }
    else if(conversations[currentChan]->updateUsers==true)
    {
        //updateUsersList("", message);
    }

    QStringList list=message.split("\r\n");
    foreach(QString msg,list)
        HandleServerMessage(msg);
    /*
    QStringList list=message.split("\r\n");
        foreach(QString msg,list)
        {
            if(msg.contains(QRegExp(":([a-zA-Z0-9\\D]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ PRIVMSG ([a-zA-Z0-9\\#]+) :(.+)")))
            {
                QRegExp reg(":([a-zA-Z0-9\\D]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ PRIVMSG ([a-zA-Z0-9\\#]+) :(.+)");
                QString msg2=msg;
                    ecrire(msg.replace(reg,"\\2 <b>&lt;\\1&gt;</b> \\3"),"",msg2.replace(reg,"\\2 <\\1> \\3"));
            }
            else if(msg.contains(QRegExp(":([a-zA-Z0-9\\D]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ JOIN ([a-zA-Z0-9\\#]+)")))
            {
                QRegExp reg(":([a-zA-Z0-9\\D]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ JOIN ([a-zA-Z0-9\\#]+)");
                QString msg2=msg;
                ecrire(msg.replace(reg,"\\2 <i>-> \\1 join \\2</i><br />"),"",msg2.replace(reg,"-> \\1 join \\2"));
                updateUsersList(msg.replace(reg,"\\2"));
            }
            else if(msg.contains(QRegExp(":([a-zA-Z0-9\\D]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ PART ([a-zA-Z0-9\\#]+)")))
            {
                QRegExp reg(":([a-zA-Z0-9\\D]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ PART ([a-zA-Z0-9\\#]+) :(.+)");
                QString msg2=msg;
                ecrire(msg.replace(reg,"\\2 <i>-> \\1 quit \\2 (\\3)</i><br />"),"",msg2.replace(reg,"-> \\1 quit \\2"));
                //updateUsersList(msg.replace(reg,"\\2"));
            }
            else if(msg.contains(QRegExp(":([a-zA-Z0-9\\D]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ QUIT (.+)")))
            {
                QRegExp reg(":([a-zA-Z0-9\\D]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ QUIT (.+)");
                QString msg2=msg;
                ecrire(msg.replace(reg,"\\2 <i>-> \\1 quit (\\2)</i><br />"),"",msg2.replace(reg,"-> \\1 left"));
                updateUsersList(msg.replace(reg,"\\2"));
            }
            else if(msg.contains(QRegExp(":([a-zA-Z0-9\\D]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ NICK :(.+)")))
            {
                QRegExp reg(":([a-zA-Z0-9\\D]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ NICK :(.+)");
                QString msg2=msg;
                ecrire(msg.replace(reg,"<i>\\1 changed nick to \\2</i><br />"),"",msg2.replace(reg,"-> \\1 is now called \\2"));
                updateUsersList(currentChan);
            }
            else if(msg.contains(QRegExp(":([a-zA-Z0-9\\D]+)\\![a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ KICK ([a-zA-Z0-9\\#]+) ([a-zA-Z0-9]+) :(.+)")))
            {
                QRegExp reg(":([a-zA-Z0-9\\D]+)\\!~[a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ KICK ([a-zA-Z0-9\\#]+) ([a-zA-Z0-9]+) :(.+)");
                QString msg2=msg;
                ecrire(msg.replace(reg,"\\2 <i>-> \\1 kicked \\3 (\\4)</i><br />"),"",msg2.replace(reg,"-> \\1 quit \\3"));
                updateUsersList(msg.replace(reg,"\\2"));
            }
            else if(msg.contains(QRegExp(":([a-zA-Z0-9\\D]+)\\![a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ NOTICE ([a-zA-Z0-9]+) :(.+)")))
            {
                if(conversations.contains(currentChan))
                {
                    QRegExp reg(":([a-zA-Z0-9\\D]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ NOTICE [a-zA-Z0-9]+ :(.+)");
                    ecrire(msg.replace(reg,"<b>[NOTICE] <i>\\1</i> : \\2 <br />"),currentChan);
                }
                else if(currentChan==serverName)
                {
                    QRegExp reg(":([a-zA-Z0-9\\D]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ NOTICE [a-zA-Z0-9]+ :(.+)");
                    ecrire(msg.replace(reg,"<b>[NOTICE] <i>\\1</i> : \\2 <br />"));
                }
            }
            else if(msg.contains("/MOTD command."))
            {
                joins();
            }else{
                HandleServerMessage(msg);
            }
        }
        //}
        */
}

QString IrcServer::nickColorHex(QString nick)
{
    if (nick.startsWith('@') || nick.startsWith("+"))
        nick = nick.remove(0, 1);

    if (!nickColors.contains(nick))
    {
        std::string s = nick.toStdString();
        uint256 hashName = Hash(s.begin(), s.end());
        std::string hex = hashName.GetHex();

        QString colorHexString = "#";

        for(int i=0; i<hex.length(); i++)
        {
            if (hex[i] > '7' || colorHexString.length()%2==0)
            {
                colorHexString.append(hex[i]);
            }

            if (colorHexString.length() == 7)
                break;
        }

        for(int i=0; i<6; i++)
        {            
            if (hex[i] < '7' && colorHexString.length()%2!=0)
                hex[i] = '7';

            if (colorHexString.length() < 7)
                colorHexString.append(hex[i]);
        }

        nickColors[nick] = colorHexString;
    }
    return nickColors[nick];

}

QString IrcServer::resolveTarget(const QString& sender, const QString& receiver) const
    {
        QString target = receiver;

        if (target.contains(QLatin1Char('*')) || target.contains(QLatin1Char('?')))
            target = nick;

        if (target == nick)
        {
            if (target == sender)
                target = this->serverName;
            else
                target = sender;
        }

        if (target.isEmpty() || target == QLatin1String("AUTH"))
            target = this->serverName;

        return target;
    }

/*!
    Parses and returns the nick part from \a target.
*/
QString IrcServer::nickFromTarget(const QString& target)
{
    int index = target.indexOf(QLatin1Char('!'));
    return target.left(index);
}

/*!
    Parses and returns the host part from \a target.
*/
QString IrcServer::hostFromTarget(const QString& target)
{
    int index = target.indexOf(QLatin1Char('!'));
    return target.mid(index + 1);
}

void IrcServer::HandleUserJoin(QString nick, QString channel)
{
    ChatTab* chanTab = conversations[channel];

    if (!chanTab)
    {
        printf("IRC unable to find chat window for %s", channel.toStdString().c_str());
        return;
    }

    if (!chanTab->users.contains(nick))
    {
        chanTab->users.append(nick);
        chanTab->users.sort();
        chanTab->UpdateUserList();
        writeToTab("<b><i>"+nick+" joined "+channel+"</i></b>", channel);
    }
}

void IrcServer::HandleUserPart(QString nick, QString channel)
{
    ChatTab* chanTab = conversations[channel];

    if (!chanTab)
    {
        printf("IRC unable to find chat window for %s", channel.toStdString().c_str());
        return;
    }

    if (chanTab->users.contains(nick))
    {
        chanTab->users.removeOne(nick);
        chanTab->users.sort();
        chanTab->UpdateUserList();
        writeToTab("<b><i>"+nick+" left "+channel+"</i></b>", channel);
    }
}

void IrcServer::HandleUserQuit(QString nick, QString reason)
{
    foreach(QString k, conversations.keys())
    {
        ChatTab* ct = conversations[k];
        if (ct && ct->users.contains(nick))
        {
            ct->users.removeOne(nick);
            writeToTab("<b><i>"+nick+" quit ("+reason+")</i></b>", k);
        }
    }
}

void IrcServer::HandleServerMessage(QString msg)
{
    if (msg.trimmed().length() == 0)
        return;

    // From RFC 1459:
    //  <message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
    //  <prefix>   ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
    //  <command>  ::= <letter> { <letter> } | <number> <number> <number>
    //  <SPACE>    ::= ' ' { ' ' }
    //  <params>   ::= <SPACE> [ ':' <trailing> | <middle> <params> ]
    //  <middle>   ::= <Any *non-empty* sequence of octets not including SPACE
    //                 or NUL or CR or LF, the first of which may not be ':'>
    //  <trailing> ::= <Any, possibly *empty*, sequence of octets not including
    //                   NUL or CR or LF>

    QString prefix, command;
    QStringList params;

    // format
    // :[server] [command] [recipient] [message]

    QStringList lines=msg.split("\r\n");
    for(int i=0; i<lines.count(); i++)
    {
        QString process = lines[i];
        // parse <prefix>
        if (process.startsWith(QLatin1Char(':')))
        {
            prefix = process.mid(1, process.indexOf(QLatin1Char(' ')) - 1);
            process.remove(0, prefix.length() + 2);

            /*
            if (options & Session::StripNicks)
            {
                int index = prefix.indexOf(QRegExp(QLatin1String("[@!]")));
                if (index != -1)
                    prefix.truncate(index);
            }
            */
        }

        // parse <command>
        command = process.mid(0, process.indexOf(QLatin1Char(' ')));
        process.remove(0, command.length() + 1);
        bool isNumeric = false;
        uint code = command.toInt(&isNumeric);

        // parse middle/params
        while (!process.isEmpty())
        {
            if (process.startsWith(QLatin1Char(':')))
            {
                process.remove(0, 1);
                params << process;
                process.clear();
            }
            else
            {
                QString param = process.mid(0, process.indexOf(QLatin1Char(' ')));
                process.remove(0, param.length() + 1);
                params << param;
            }
        }

        // and dump
        if (isNumeric)
        {
            switch (code)
            {
            case RPL_WELCOME:
            {
                /*
                Q_ASSERT(defaultBuffer);
                defaultBuffer->d_func()->setReceiver(prefix, false);
                emit q->welcomed();
                welcomed = true;
                if (!capabilitiesSupported && !wantedCapabilities.isEmpty())
                    emit q->capabilitiesNotAcked(wantedCapabilities);
                wantedCapabilities.clear();
                */
                break;
            }

            case RPL_TOPIC:
            {
                QString topic = params.value(2);
                QString target = resolveTarget(QString(), params.value(1));
                writeToTab(topic, target);
                break;
            }

            case RPL_NAMREPLY:
            {
                QStringList list = params;
                list.removeAll(QLatin1String("="));
                list.removeAll(QLatin1String("@"));
                list.removeAll(QLatin1String("*"));

                QString target = resolveTarget(QString(), list.value(1));
                ChatTab* chanTab = conversations[target];

                if (chanTab->updateUsers)
                {
                    //Buffer* buffer = createBuffer(target);
                    QStringList names = list.value(2).split(QLatin1String(" "), QString::SkipEmptyParts);

                    QStringList usersToRemove;
                    for(int i=0; i<chanTab->users.count(); i++)
                    {
                        if (!names.contains(chanTab->users[i]))
                            usersToRemove.append(chanTab->users[i]);
                    }

                    for(int i=0; i<usersToRemove.count(); i++)
                        chanTab->users.removeAll(usersToRemove[i]);

                    for(int i=0; i<names.count(); i++)
                    {
                        // add new?
                        if (!chanTab->users.contains(names[i]))
                            chanTab->users.append(names[i]);
                    }

                    chanTab->UpdateUserList();

                    if (chanTab->userView)
                    {
                        chanTab->userView->setModel(chanTab->modelUsers);
                        chanTab->userView->update();
                    }
                    chanTab->updateUsers = false;
                }else{
                    chanTab->updateUsers=true;
                    sendData("NAMES "+target);
                }
                break;
            }

            case RPL_MOTDSTART:
                motd.clear();
                break;

            case RPL_MOTD:
                motd.append(params.value(1) + QLatin1Char('\n'));
                break;

            case RPL_ENDOFMOTD:
                motd.clear();
                writeToTab(motd);
                joins();
                break;

            default:
                break;
            }

           if (code == RPL_TOPICSET && IRC_STRIP_NICKS)
           {
               QString user = params.value(2);
               int index = user.indexOf(QRegExp(QLatin1String("[@!]")));
               if (index != -1)
               {
                   user.truncate(index);
                   params.replace(2, user);
               }
           }

           //if (defaultBuffer)
           //    emit defaultBuffer->numericMessageReceived(prefix, code, params);
        }
        else
        {
            if (command == QLatin1String("NICK"))
            {
                QString oldNick = nickFromTarget(prefix);
                QString newNick = params.value(0);

                if (nick == oldNick)
                    nick = newNick;

                foreach(QString k, conversations.keys())
                {
                    ChatTab* ct = conversations[k];
                    for (int i=0; i<ct->users.count(); i++)
                        if (ct->users[i].compare(oldNick) == 0)
                            ct->users[i] = newNick;
                }

                /*
                foreach (Buffer* buffer, buffers)
                {
                   if (buffer->receiver() == oldNick)
                       buffer->d_func()->setReceiver(newNick);
                   if (buffer->names().contains(oldNick))
                   {
                       buffer->d_func()->removeName(oldNick);
                       buffer->d_func()->addName(newNick);
                       emit buffer->nickChanged(prefix, newNick);
                   }
               }
               */
            }
            else if (command == QLatin1String("QUIT"))
            {
                QString reason = params.value(0);
                QString nick = nickFromTarget(prefix);
                HandleUserQuit(nick, reason);
            }
            else if (command == QLatin1String("JOIN"))
            {
                QString channel = params.value(0);
                QString target = resolveTarget(prefix, channel);
                QString joinedNick = nickFromTarget(prefix);
                HandleUserJoin(joinedNick, channel);
            }
            else if (command == QLatin1String("PART"))
            {
                QString channel = params.value(0);
                QString target = nickFromTarget(prefix);

                HandleUserPart(target, channel);

                /*
                if (nick != nickFromTarget(prefix))
                {
                    QString message = params.value(1);
                    Buffer* buffer = createBuffer(target);
                    buffer->d_func()->removeName(Util::nickFromTarget(prefix));
                    emit buffer->parted(prefix, message);
                }
                else if (buffers.contains(target))
                {
                    Buffer* buffer = buffers.value(target);
                    removeBuffer(buffer);
                    buffer->deleteLater();
                }*/
            }
            else if (command == QLatin1String("MODE"))
            {
                QString receiver = params.value(0);
                QString mode = params.value(1);
                QString args = params.value(2);
                QString target = nickFromTarget(prefix);
                /*
                Buffer* buffer = createBuffer(target);
                buffer->d_func()->updateMode(args, mode);
                emit buffer->modeChanged(prefix, mode, args);*/
                bool haveChannel = conversations.keys().contains(receiver);
                QString channel = haveChannel ? receiver : "";
                writeToTab(target+" sets mode "+mode+" "+args, channel);

                if (haveChannel)
                {
                    ChatTab* tab = conversations[channel];
                    if (QString::compare(mode, "-o", Qt::CaseInsensitive) == 0 || QString::compare(mode, "-v", Qt::CaseInsensitive) == 0)
                    {
                        for(int i=0; i<tab->users.count(); i++)
                        {
                            QString targetName = QString::compare(mode, "-o", Qt::CaseInsensitive) == 0 ? "@"+args : "+"+args;
                            if (QString::compare(tab->users[i], targetName, Qt::CaseInsensitive) == 0)
                            {
                                QString newName = tab->users[i].remove(0, 1);
                                tab->users[i] = newName;
                            }
                        }
                    }else if (QString::compare(mode, "+o", Qt::CaseInsensitive) == 0 || QString::compare(mode, "+v", Qt::CaseInsensitive) == 0)
                    {
                        for(int i=0; i<tab->users.count(); i++)
                        {
                            if (QString::compare(tab->users[i], args, Qt::CaseInsensitive) == 0)
                            {
                                QString newName = tab->users[i].prepend(QString::compare(mode, "+o") == 0 ? "@" : "+");
                                tab->users[i] = newName;
                            }
                        }
                    }
                    tab->UpdateUserList();
                }
            }
            else if (command == QLatin1String("TOPIC"))
            {
                /*
                QString channel = params.value(0);
                QString topic = params.value(1);
                QString target = resolveTarget(prefix, channel);
                Buffer* buffer = createBuffer(target);
                buffer->d_func()->topic = topic;
                emit buffer->topicChanged(prefix, topic);
               */
            }
            else if (command == QLatin1String("INVITE"))
            {
                /*
                QString receiver = params.value(0);
                QString channel = params.value(1);
                if (defaultBuffer)
                   emit defaultBuffer->invited(prefix, receiver, channel);
                 */
            }
            else if (command == QLatin1String("KICK"))
            {
                /*
                QString channel = params.value(0);
                QString nick = params.value(1);
                QString message = params.value(2);
                QString target = resolveTarget(prefix, channel);
                Buffer* buffer = createBuffer(target);
                buffer->d_func()->removeName(nick);
                emit buffer->kicked(prefix, nick, message);
                */
            }
            else if (command == QLatin1String("PRIVMSG"))
            {
                QString message = params.value(1);

                //Irc::Buffer::MessageFlags flags = getMessageFlags(message);

                if (message.startsWith(QLatin1Char('\1')) && message.endsWith(QLatin1Char('\1')))
                {
                    message.remove(0, 1);
                    message.remove(message.length() - 1, 1);

                    if (message.startsWith(QLatin1String("ACTION ")))
                    {
                        QString receiver = params.value(0);
                       //QString target = resolveTarget(prefix, receiver);
                       //Buffer* buffer = createBuffer(target);
                       //emit buffer->ctcpActionReceived(prefix, message.mid(7), flags);
                    }
                    else
                    {
                       // TODO: check params
                       //if (defaultBuffer)
                       //    emit defaultBuffer->ctcpRequestReceived(prefix, message, flags);
                        writeToTab(prefix+" "+message);
                    }
                }
                else
                {
                    QString receiver = params.value(0);
                    QString target = resolveTarget(prefix, receiver);
                    //Buffer* buffer = createBuffer(target);
                    //emit buffer->messageReceived(prefix, message, flags);

                    QString nick = nickFromTarget(prefix);

                    if (conversations.contains(receiver))
                        nick = conversations[receiver]->FormatNick(nick);

                    writeToTab(receiver+" <b><span style='color: "+this->nickColorHex(nick)+";'>&lt;"+nick+"&gt;</span></b> "+message);
                }
            }
            else if (command == QLatin1String("NOTICE"))
            {
               //if (!welcomed)
               //{
                 //  Q_ASSERT(defaultBuffer);
                 //  defaultBuffer->d_func()->setReceiver(prefix, false);
               //}

                QString receiver = params.value(0);
                QString message = params.value(1);

              // Irc::Buffer::MessageFlags flags = getMessageFlags(message);

                if (message.startsWith(QLatin1Char('\1')) && message.endsWith(QLatin1Char('\1')))
                {
                    message.remove(0, 1);
                    message.remove(message.length() - 1, 1);

                   // TODO: check params
                   //if (defaultBuffer)
                   //    emit defaultBuffer->ctcpReplyReceived(prefix, /*receiver,*/ message, flags);
                }
                else
                {
                   //QString target = resolveTarget(prefix, receiver);
                   //Buffer* buffer = createBuffer(target);
                   //emit buffer->noticeReceived(prefix, message, flags);
                    writeToTab(prefix+" <b>"+message+"<b>");
                }
            }
            else if (command == QLatin1String("KILL"))
            {
                // ignore this event - not all servers generate this
            }
            else if (command == QLatin1String("CAP"))
            {

            }
            else
            {

            }
        }
        if (serverName.length())
        {
            //ecrire(message);
        }
    }
}

/*
void IrcServer::HandleServerMessage(QString msg)
{
    if (msg.trimmed().length() == 0)
        return;
    // From RFC 1459:
    //  <message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
    //  <prefix>   ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
    //  <command>  ::= <letter> { <letter> } | <number> <number> <number>
    //  <SPACE>    ::= ' ' { ' ' }
    //  <params>   ::= <SPACE> [ ':' <trailing> | <middle> <params> ]
    //  <middle>   ::= <Any *non-empty* sequence of octets not including SPACE
    //                 or NUL or CR or LF, the first of which may not be ':'>
    //  <trailing> ::= <Any, possibly *empty*, sequence of octets not including
    //                   NUL or CR or LF>
    QString serverName = "";
    QString commandNum = "";
    QString user = "";
    QString message = "";
    // format
    // :[server] [command] [recipient] [message]
    QStringList lines=msg.split("\r\n");
    for(int i=0; i<lines.count(); i++)
    {
        QString line = lines[i];
        if (line.indexOf("End of /NAMES list") >-1)
            continue;
        QStringList words = line.split(" ");
        if (words.count() >=3)
        {
            if (words[0].length() >=2)
                serverName = words[0].remove(0, 1);
            if (words.count() >4 && QString::compare(words[3], "=", Qt::CaseInsensitive) == 0 && line.contains("353"))
            {
                QString channelName = words[4];
                updateUsersList(channelName, line);
                continue;
            }
            for(int j=3; j<words.count(); j++)
            {
                if (j>3)
                    message.append(" ");
                if (j==3 && words[j].startsWith(QChar(':')))
                    message.append(words[j].remove(0, 1));
                else
                    message.append(words[j]);
            }
        }
    }
    if (serverName.length())
    {
        ecrire(message);
    }
}
*/

void IrcServer::sendData(QString txt)
{
	if(this->state()==QAbstractSocket::ConnectedState)
    {
        this->write((txt+"\r\n").toUtf8());
	}
}

QString IrcServer::parseCommande(QString comm, bool serveur)
{
    if(comm.startsWith("/"))
    {
        comm.remove(0,1);
        QString pref=comm.split(" ").first();
        QStringList args=comm.split(" ");
        args.removeFirst();
        QString destChan=tab->tabText(tab->currentIndex());
        QString msg=args.join(" ");

        if(pref=="me")
            return "PRIVMSG "+destChan+" ACTION " + msg + "";
        else if(pref=="msg")
            return "MSG "+destChan+" ACTION " + msg + "";
        else if(pref=="join")
        {
            join(msg);
            return " ";
        }
        else if(pref=="quit")
        {
            if(msg == "")
                return "QUIT "+msgQuit;
            else
                return "QUIT "+msg;
        }
        else if(pref=="part")
        {
            tab->removeTab(tab->currentIndex());

            if(msg == "")
            {
                if(msg.startsWith("#"))
                    destChan=msg.split(" ").first();

                if(msgQuit=="")
                    return "PART "+destChan+" using IrcLightClient";
                else
                    return "PART "+destChan+" "+msgQuit;
            }
            else
                return "PART "+destChan+" "+msg;

            conversations.remove(destChan);
        }
        else if(pref=="kick")
        {
            QStringList tableau=msg.split(" ");
            QString c1,c2,c3;
            if(tableau.count() > 0) c1=" "+tableau.first();
            else c1="";
            if(tableau.count() > 1) c2=" "+tableau.at(1);
            else c2="";
            if(tableau.count() > 2) c3=" "+tableau.at(2);
            else c3="";

            if(c1.startsWith("#"))
                return "KICK"+c1+c2+c3;
            else
                return "KICK "+destChan+c1+c2;
        }
        else if(pref=="update")
        {
            conversations[destChan]->updateUsers=true;
            return "WHO "+destChan;
        }
        else if(pref=="ns")
        {
            return "NICKSERV "+msg;
        }
        else if(pref=="nick")
        {
            emit pseudoChanged(msg);
            writeToTab("-> Nickname changed to "+msg);
            return "NICK "+msg;
        }
        else if(pref=="msg")
        {
            return "MSG "+msg;
        }

        else
            return pref+" "+msg;
    }
    else if(!serveur)
	{
        QString destChan=tab->tabText(tab->currentIndex());
        if(comm.endsWith("<br />"))
            comm=comm.remove(QRegExp("<br />$"));
        writeToTab("<b>&lt;"+pseudo+"&gt;</b> "+comm, destChan);

        if(!comm.startsWith(":"))
            comm.insert(0,":");

        return "PRIVMSG "+destChan+" "+comm.replace(" "," ");
    }
    return "";
}

void IrcServer::join(QString chan)
{
    editor->append("Joining "+ chan +" channel");
	emit joinTab();
    /*
	QTextEdit *textEdit=new QTextEdit;    
    int index=tab->insertTab(tab->currentIndex()+1, textEdit, chatin);
	tab->setTabToolTip(index,serverName);
	tab->setCurrentIndex(index);
	textEdit->setReadOnly(true);
    */
    ChatTab* newTab = new ChatTab(tab, chan, 0, this);
    int index=tab->insertTab(tab->currentIndex()+1, newTab, chan);
    tab->setCurrentIndex(index);

    conversations.insert(chan, newTab);

    sendData("JOIN "+chan);

	emit tabJoined();
}
void IrcServer::leave(QString chan)
{
    sendData(parseCommande("/part "+chan+ " "+msgQuit));
}

void IrcServer::writeToTab(QString txt, QString destChan, QString msgTray)
{
    if(destChan!="")
    {
        conversations[destChan]->textEdit->setHtml(conversations[destChan]->textEdit->toHtml()+txt);
        QScrollBar *sb = conversations[destChan]->textEdit->verticalScrollBar();
        sb->setValue(sb->maximum());
    }
    else if(txt.startsWith("#"))
    {
        QString dest=txt.split(" ").first();
        QStringList list=txt.split(" ");
        list.removeFirst();
        txt=list.join(" ");
        conversations[dest]->textEdit->setHtml(conversations[dest]->textEdit->toHtml()+txt);
        QScrollBar *sb = conversations[dest]->textEdit->verticalScrollBar();
        sb->setValue(sb->maximum());        }
    else
    {
        txt.replace("\r\n","<br />");
        editor->setHtml(editor->toHtml()+txt);
        QScrollBar *sb = editor->verticalScrollBar();
        sb->setValue(sb->maximum());
    }
}

void IrcServer::updateUsersList(QString chan, QString message)
{
    /*
    message = message.replace("\r\n","");
    message = message.replace("\r","");
    QListView* userList = tab->ChannelTab(chan)->userView;
    if(chan!=serverName)
    {
        if(conversations[chan]->updateUsers==true || message != "")
        {
            QString liste2=message.replace(":","");
            QStringList liste=liste2.split(" ");
            if (delist == true) users.clear();
            for(int i=5; i < liste.count(); i++)
            {
                users.append(liste.at(i));
            }
            conversations[chan]->updateUsers=false;
            if (liste.count() < 53)
                delist = true;
            else
                delist = false;
            users.sort();
            QStringListModel *model = new QStringListModel(users);
            if (userList)
            {
                userList->setModel(model);
                userList->update();
            }
        }
        else
        {
            conversations[chan]->updateUsers=true;
            sendData("NAMES "+chan);
        }
    }
    else
    {
        QStringListModel model;
        userList->setModel(&model);
        userList->update();
    }
    */
}
