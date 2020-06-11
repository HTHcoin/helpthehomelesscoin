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

#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QHash>

class User;

//! The Server class provides a custom TCP-based server.
/*!
  The Server class provides a custom TCP-based server with user
  management and robust network management of multiple clients.

  \ingroup server
 */
class Server : public QTcpServer
{
    Q_OBJECT

public:

    //! Constructs a QCC-server object.
    /*!
      \param parent The parent object.
      \sa QTcpServer::listen()
     */
    explicit Server(QObject *parent = 0);

    //! Destroys the QCC-server object.
    /*!
      \sa QTcpServer::~QTcpServer()
     */
    ~Server();

    //! Loads all users from the local file system into memory.
    /*!
      \sa USERS_FILE, saveUsers()
     */
    void loadUsers();

    //! Saves all users to the local file system for persistent storage.
    /*!
      \sa USERS_FILE, loadUsers()
     */
    void saveUsers();

private slots:

    //! This signal is emitted when the client's socket has been disconnected.
    /*!
      \sa incomingConnection(), QAbstractSocket::disconnected()
     */
    void client_disconnected();

    //! This signal is emitted once every time new data is available for reading from the client's socket.
    /*!
      \sa QIODevice::readyRead()
     */
    void client_readyRead();

    //! This signal is emitted whenever the client's status changes.
    /*!
      \sa AbstractUser::statusChanged()
     */
    void client_statusChanged();

private:

    //! The internal Client struct provides a thin wrapper for a User-object and the packet size.
    struct Client {

        //! Constructs a Client object with the packet size set to \c 0.
        /*!
          \param u The user object or \c NULL if there is not any user yet.
         */
        explicit inline Client(User *u = NULL) : packetSize(0), user(u) { }

        quint32 packetSize; //!< The packet size of the incoming QccPacket.
        User *user;         //!< A reference to the actual user object.
    };

    //! The filename for the persistent user storage (stored in XML-format).
    static const QString USERS_FILE;

    QHash<QString, User*> m_users;         //!< The hash-table for all user objects on this server.
    QHash<QTcpSocket*, Client*> m_clients; //!< The hash-table holds track of all connected clients.

    //! This virtual function is called by \c QTcpServer when a new connection is available.
    /*!
      \param socketDescriptor The native socket descriptor for the accepted connection.
      \sa client_disconnected(), QTcpServer::incomingConnection()
     */
    void incomingConnection(int socketDescriptor);
};

#endif // SERVER_H
