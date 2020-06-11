/****************************************************************************
**
** Copyright (C) 2011 Alexander Vos <info@vossoft.de>,
**                    Kai Wellmann <KaiWellmann@gmx.de>
**
** This file is part of Qt Crypto Chat (QCC).
**
** Qt Crypto Chat is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** Qt Crypto Chat is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Qt Crypto Chat.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include "server.h"
#include "qccpacket.h"
#include "user.h"

#include <QTcpSocket>
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#ifdef DEBUG
#include <QDebug>
#endif

const QString Server::USERS_FILE = "users.xml";

Server::Server(QObject *parent) :
    QTcpServer(parent)
{
    loadUsers();
}

Server::~Server()
{
    qDeleteAll(m_clients.values());
    qDeleteAll(m_users.values());
}

void Server::loadUsers()
{
#ifdef DEBUG
    qDebug("Server::loadUsers()");
#endif

    QFile file(Server::USERS_FILE);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Server::loadUsers(): cannot read file users.xml => %s", qPrintable(file.errorString()));
        return;
    }
    m_users.clear();
    QXmlStreamReader xml(&file);
    while (!xml.atEnd()) {
        if (!xml.readNextStartElement())
            continue;
        if (xml.name() == "user") {
            User *user = User::readUser(xml);
            if (user && user->isValid()) {
                connect(user, SIGNAL(statusChanged()), SLOT(client_statusChanged()));
                m_users.insert(user->username(), user);
#ifdef DEBUG
                qDebug() << "user:" << user->username() << "contacts =" << user->contacts();
#endif
            }
        }
    }
    if (xml.hasError()) {
        qCritical("Server::loadUsers(): parse error in file users.xml => %s: line %li, column %li",
                  qPrintable(xml.errorString()), (long)xml.lineNumber(), (long)xml.columnNumber());
    }
    file.close();

#ifdef DEBUG
    qDebug("Server::loadUsers(): %i users read from users.xml", m_users.count());
#endif
}

void Server::saveUsers()
{
#ifdef DEBUG
    qDebug("Server::saveUsers()");
#endif

    QFile file(Server::USERS_FILE);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning("Server::saveUsers(): cannot write file users.xml => %s", qPrintable(file.errorString()));
        return;
    }
    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("users");
    foreach (User *user, m_users)
        user->writeUser(xml);
    xml.writeEndElement();
    xml.writeEndDocument();
    file.close();

#ifdef DEBUG
    qDebug("Server::saveUsers(): %i users written to users.xml", m_users.count());
#endif
}

void Server::incomingConnection(int socketDescriptor)
{
    QTcpSocket *socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);
    addPendingConnection(socket);

#ifdef DEBUG
    qDebug("\nServer::incomingConnection(%i): %s:%i", socketDescriptor,
           qPrintable(socket->peerAddress().toString()), socket->peerPort());
#endif

    connect(socket, SIGNAL(disconnected()), SLOT(client_disconnected()));
    connect(socket, SIGNAL(readyRead()), SLOT(client_readyRead()));

    m_clients.insert(socket, new Client);
    QccPacket(QccPacket::ConnectionAccepted).send(socket); // QccPacket::ConnectionRefused

#ifdef DEBUG
    qDebug("ConnectionAccepted");
#endif
}

void Server::client_disconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) {
        qWarning("Server::client_disconnected(): Cast of sender() to QTcpSocket* failed");
        return;
    }

    Client *client = m_clients[socket];
    User *user = client->user;
    if (user)
        user->reset();

#ifdef DEBUG
    QString username = user ? user->username() : "[no user object]";
    qDebug("\nServer::client_disconnected(): '%s' (%s:%i)", qPrintable(username),
           qPrintable(socket->peerAddress().toString()), socket->peerPort());
#endif

    m_clients.remove(socket);
    delete client;
}

void Server::client_readyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) {
        qWarning("Server::client_readyRead(): Cast of sender() to QTcpSocket* failed");
        return;
    }

#ifdef DEBUG
    qDebug("\nServer::client_readyRead(): %li bytes available", (long)socket->bytesAvailable());
#endif

    Client *client = m_clients[socket];
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_4_0);
    if (client->packetSize == 0) {
        if (socket->bytesAvailable() < (int)sizeof(quint32)) // packet size
            return;
        in >> client->packetSize;
    }
    if (socket->bytesAvailable() < client->packetSize)
        return;
    client->packetSize = 0; // reset packet size

    qint32 type;
    in >> type;

#ifdef DEBUG
    qDebug("PacketType %i (%s) from '%s' (%s:%i)", type,
           qPrintable(QccPacket::typeString((QccPacket::PacketType)type)),
           qPrintable(client->user ? client->user->username() : "[no user object]"),
           qPrintable(socket->peerAddress().toString()), socket->peerPort());
#endif

    switch ((QccPacket::PacketType)type) {
    case QccPacket::UserRegister:
    {
        QString username, password;
        QByteArray publicKey;
        in >> username >> password >> publicKey;
#ifdef DEBUG
        qDebug("Username = '%s' password = '%s'", qPrintable(username), qPrintable(password));
#endif
        if (username.isEmpty()) {
            QString reason = "The username cannot be empty.";
            QccPacket packet(QccPacket::RegisterFailure);
            packet.stream() << reason;
            packet.send(socket);
#ifdef DEBUG
            qDebug("RegisterFailure: %s", qPrintable(reason));
#endif
            break;
        }
        if (m_users.contains(username)) {
            QString reason = QString("The username \"%1\" is not available.").arg(username);
            QccPacket packet(QccPacket::RegisterFailure);
            packet.stream() << reason;
            packet.send(socket);
#ifdef DEBUG
            qDebug("RegisterFailure: %s", qPrintable(reason));
#endif
            break;
        }
#ifdef DEBUG
            qDebug("RegisterSuccess");
#endif
        User *user = new User(username, password);
        user->setPublicKey(publicKey);
        user->setSocket(socket);
        connect(user, SIGNAL(statusChanged()), SLOT(client_statusChanged()));
        user->setStatus(User::Online);
        m_users.insert(username, user);
        saveUsers();
        client->user = user;
        QccPacket(QccPacket::RegisterSuccess).send(socket);
        break;
    }
    case QccPacket::UserAuthentication:
    {
        QString username, password;
        QByteArray publicKey;
        in >> username >> password >> publicKey;
#ifdef DEBUG
        qDebug("Username = '%s' password = '%s'", qPrintable(username), qPrintable(password));
#endif
        User *user = m_users.contains(username) ? m_users[username] : NULL;
        if (user && user->matchPassword(password)) {
#ifdef DEBUG
            qDebug("AuthenticationSuccess");
#endif
            user->setPublicKey(publicKey);
            user->setSocket(socket);
            user->setStatus(User::Online);
            client->user = user;
            QccPacket(QccPacket::AuthenticationSuccess).send(socket);
        } else {
            QString reason = "The username or password you entered is incorrect.";
            QccPacket packet(QccPacket::AuthenticationFailure);
            packet.stream() << reason;
            packet.send(socket);
#ifdef DEBUG
            qDebug("AuthenticationFailure: %s", qPrintable(reason));
#endif
        }
        break;
    }
    case QccPacket::RequestAuthorization:
    {
        QString username;
        in >> username;
        if (!m_users.contains(username)) {
            QString reason = QString("The user \"%1\" does not exist.").arg(username);
            QccPacket packet(QccPacket::AuthorizationFailure);
            packet.stream() << reason;
            packet.send(socket);
#ifdef DEBUG
            qDebug("AuthorizationFailure: %s", qPrintable(reason));
#endif
            break;
        }
        if (client->user->username() == username) {
            QString reason = QString("You cannot add yourself.");
            QccPacket packet(QccPacket::AuthorizationFailure);
            packet.stream() << reason;
            packet.send(socket);
#ifdef DEBUG
            qDebug("AuthorizationFailure: %s", qPrintable(reason));
#endif
            break;
        }
        if (client->user->containsContact(username)) {
            QString reason = QString("The user \"%1\" is already on your contact list.").arg(username);
            QccPacket packet(QccPacket::AuthorizationFailure);
            packet.stream() << reason;
            packet.send(socket);
#ifdef DEBUG
            qDebug("AuthorizationFailure: %s", qPrintable(reason));
#endif
            break;
        }
        User *receiver = m_users.value(username);
        if (receiver && receiver->isOnline()) {
            QccPacket packet(QccPacket::RequestAuthorization);
            packet.stream() << client->user->username();
            packet.send(receiver->socket());
#ifdef DEBUG
            qDebug("RequestAuthorization: forwarded to '%s'", qPrintable(username));
#endif
        } else {
            QString reason = QString("The user \"%1\" is not online.").arg(username);
            QccPacket packet(QccPacket::AuthorizationFailure);
            packet.stream() << reason;
            packet.send(socket);
#ifdef DEBUG
            qDebug("AuthorizationFailure: %s", qPrintable(reason));
#endif
        }
        break;
    }
    case QccPacket::AuthorizationAccepted:
    {
        QString username;
        in >> username;
        if (username.isEmpty())
            break;
        User *receiver = m_users[username];
        if (receiver && receiver->isOnline()) {
            receiver->addContact(client->user->username());
            client->user->addContact(username);
            saveUsers();
            QccPacket packet(QccPacket::AuthorizationAccepted);
            packet.stream() << client->user->username() << (qint32)client->user->status() << client->user->publicKey();
            packet.send(receiver->socket());
            QccPacket packet2(QccPacket::AuthorizationAccepted);
            packet2.stream() << username << (qint32)receiver->status() << receiver->publicKey();
            packet2.send(socket);
        }
#ifdef DEBUG
            qDebug("AuthorizationAccepted: forwarded to '%s'", qPrintable(username));
#endif
        break;
    }
    case QccPacket::AuthorizationDeclined:
    {
        QString username;
        in >> username;
        if (username.isEmpty())
            break;
        User *receiver = m_users.value(username);
        if (receiver && receiver->isOnline()) {
            QccPacket packet(QccPacket::AuthorizationDeclined);
            packet.stream() << client->user->username();
            packet.send(receiver->socket());
        }
#ifdef DEBUG
            qDebug("AuthorizationDeclined: forwarded to '%s'", qPrintable(username));
#endif
        break;
    }
    case QccPacket::RequestContactList:
    {
        QccPacket packet(QccPacket::ContactList);
        QSet<QString> contacts = client->user->contacts();
        packet.stream() << (qint32)contacts.count();
        foreach (const QString &contactName, contacts) {
            User *contact = m_users.value(contactName);
            if (!contact) continue;
            packet.stream() << contactName << qint32(contact->status()) << contact->publicKey();
        }
        packet.send(socket);
#ifdef DEBUG
            qDebug("ContactList: %i contacts send", contacts.count());
#endif
        break;
    }
    case QccPacket::RemoveContact:
    {
        QString username;
        in >> username;
        if (client->user->removeContact(username)) {
            QccPacket packet(QccPacket::ContactRemoved);
            packet.stream() << username;
            packet.send(socket);
            User *receiver = m_users[username];
            if (receiver && receiver->removeContact(client->user->username()) && receiver->isOnline()) {
                QccPacket packet2(QccPacket::ContactRemoved);
                packet2.stream() << client->user->username();
                packet2.send(receiver->socket());
            }
            saveUsers();
#ifdef DEBUG
            qDebug("ContactRemoved: contact '%s' removed", qPrintable(username));
#endif
        }
        break;
    }
    case QccPacket::Message:
    {
        qint32 id;
        QString receiverName;
        QString message;
        in >> id >> receiverName >> message;
        if (!client->user->containsContact(receiverName)) {
            QString reason = QString("The user \"%1\" is not on your contact list.").arg(receiverName);
            QccPacket packet(QccPacket::MessageFailure);
            packet.stream() << reason;
            packet.send(socket);
#ifdef DEBUG
            qDebug("MessageFailure: %s", qPrintable(reason));
#endif
            break;
        }
        User *receiver = m_users.value(receiverName);
        if (receiver && receiver->isOnline()) {
            QccPacket packet(QccPacket::Message);
            packet.stream() << id << client->user->username() << message;
            packet.send(receiver->socket());
#ifdef DEBUG
            qDebug("Message: forwarded to '%s'", qPrintable(receiverName));
#endif
        } else {
            QString reason = QString("The user \"%1\" is not online.").arg(receiverName);
            QccPacket packet(QccPacket::MessageFailure);
            packet.stream() << id << receiverName << reason;
            packet.send(socket);
#ifdef DEBUG
            qDebug("MessageFailure: failed to forward to '%s' => %s",
                   qPrintable(receiverName), qPrintable(reason));
#endif
        }
        break;
    }
    case QccPacket::MessageSuccess:
    {
        qint32 id;
        QString receiverName;
        in >> id >> receiverName;
        User *receiver = m_users.value(receiverName);
        if (receiver && receiver->isOnline()) {
            QccPacket packet(QccPacket::MessageSuccess);
            packet.stream() << id << client->user->username();
            packet.send(receiver->socket());
#ifdef DEBUG
            qDebug("MessageSuccess: forwarded to '%s'", qPrintable(receiverName));
#endif
        }
        break;
    }
    default:
        qWarning("Server::client_readyRead(): Illegal PacketType %i", type);
        return;
    }
}

void Server::client_statusChanged()
{
    User *user = qobject_cast<User*>(sender());
    if (!user) {
        qWarning("Server::client_statusChanged(): Cast of sender() to User* failed");
        return;
    }

#ifdef DEBUG
    qDebug("\nServer::client_statusChanged(): '%s' => %i (%s)",
           qPrintable(user->username()), user->status(), qPrintable(user->statusString()));
#endif

    // inform all online users that have this user on their contact list of the status change
    QccPacket packet(QccPacket::ContactStatusChanged);
    packet.stream() << user->username() << (qint32)user->status() << user->publicKey();
    foreach (Client *client, m_clients.values()) {
        if (client->user == NULL)
            continue;
        if (client->user->containsContact(user->username())) {
            packet.send(client->user->socket());
#ifdef DEBUG
    qDebug("ContactStatusChanged: send to contact '%s'", qPrintable(client->user->username()));
#endif
        }
    }
}
