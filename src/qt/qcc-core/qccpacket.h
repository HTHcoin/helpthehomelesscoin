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

#ifndef TCPPACKET_H
#define TCPPACKET_H

#include <QByteArray>
#include <QDataStream>

QT_BEGIN_NAMESPACE
class QTcpSocket;
QT_END_NAMESPACE

//! The QccPacket class is used for sending custom TCP-packets over the network.
/*!
  The QccPacket class defines the different packet types QCC needs for the network
  communication beween the \ref server "QCC-server" and the \ref client "QCC-client".\n

  It is also used for sending QCC-packets over the network through the TCP.\n

  Each QCC-packet consists of three parts:
    -# The size of the actual packet in bytes (\c quint32).\n
       The size is defined by the size of the packet-type plus the size of the payload.
    -# The type of the packet (encoded as \c qint32).
    -# The actual data of the packet (payload) (the data-type depends on the packet-type).\n
       The payload can also be empty, because some packet-types don't need any data.

  \ingroup core
  \sa PacketType
 */
class QccPacket
{
public:

    //! This enum describes the different types of QCC-packets.
    /*!
      \sa PacketTypeNames
     */
    enum PacketType {

        //! The connection has been accepted by the QCC-server.
        /*!
          This packet has no payload.
         */
        ConnectionAccepted,

        //! The connection has been refused by the QCC-server.
        /*!
          This packet has no payload.
         */
        ConnectionRefused,

        //! The QCC-client sends a user registration.
        /*!
          Payload:
            -# \c QString(username)
            -# \c QString(passwort) - SHA-256 encoded hash
            -# \c QByteArray(publicKey) - RSA public key in DER-format
         */
        UserRegister,

        //! The QCC-server reports that the user registration has been successful.
        /*!
          This packet has no payload.
         */
        RegisterSuccess,

        //! The QCC-server reports that an error occured with the user registration.
        /*!
          Payload:
            -# \c QString(reason) - reason of the failure
         */
        RegisterFailure,

        //! The QCC-client sends a user authentication (login).
        /*!
          Payload:
            -# \c QString(username)
            -# \c QString(passwort) - SHA-256 encoded hash
            -# \c QByteArray(publicKey) - RSA public key in DER-format
         */
        UserAuthentication,

        //! The QCC-server reports that the user authentication has been successful.
        /*!
          This packet has no payload.
         */
        AuthenticationSuccess,

        //! The QCC-server reports that an error occured with the user authentication.
        /*!
          Payload:
            -# \c QString(reason) - reason of the failure
         */
        AuthenticationFailure,

        //! The QCC-client sends an authorization request to add another user to his contacts.
        /*!
          Payload:
            -# \c QString(username)

          The username gets replaced by the server with the username of the sender,
          so that the receiver knows from where the authorization request come from.
         */
        RequestAuthorization,

        //! The authorization request has been accepted.
        /*!
          Payload:
            -# \c QString(username)
            -# \c qint32(status) - user status
            -# \c QByteArray(publicKey) - RSA public key in DER-format

          The \a status and \a publicKey is appended by the server.
         */
        AuthorizationAccepted,

        //! The authorization request has been declined.
        /*!
          Payload:
            -# \c QString(username)
         */
        AuthorizationDeclined,

        //! The QCC-server reports that an error occured with the authorization request.
        /*!
          Payload:
            -# \c QString(reason) - reason of the failure
         */
        AuthorizationFailure,

        //! The QCC-client requests his list of contacts.
        /*!
          This packet has no payload.
         */
        RequestContactList,

        //! The QCC-server sends a list of the user's contacts.
        /*!
          Payload:
            -# \c qint32(count) - user count
            -# list of contacts (\a count user objects)
               - user object
                  -# \c QString(username)
                  -# \c qint32(status) - user status
                  -# \c QByteArray(publicKey) - RSA public key in DER-format
               - user object ...
         */
        ContactList,

        //! The QCC-Server sends a status-changed-packet.
        /*!
          The status-changed-packet is send to all users which have the user
          who changed the status on their contact list.\n

          Payload:
            -# \c QString(username) - user who changed the status
            -# \c qint32(status) - user status
            -# \c QByteArray(publicKey) - RSA public key in DER-format
         */
        ContactStatusChanged,

        //! The QCC-client sends a request to remove a user from his contacts.
        /*!
          Payload:
            -# \c QString(username)
         */
        RemoveContact,

        //! The QCC-Server confirms that the user has been removed from the contacts.
        /*!
          Payload:
            -# \c QString(username)
         */
        ContactRemoved,

        //! RSA-encrypted message from one user to another.
        /*!
          Payload:
            -# \c qint32(messageId) - the message id is used for association with
               the following \a MessageSuccess packet (if any)
            -# \c QString(receiver|sender) - username of the sender/receiver
            -# \c QByteArray(message) - RSA-encrypted message
         */
        Message,

        //! Confirms that the message has been successfully delivered.
        /*!
          Payload:
            -# \c qint32(messageId) - the message id is used for association with the message
            -# \c QString(sender) - username of the sender
         */
        MessageSuccess,

        //! The QCC-server reports that an error occured with the message.
        /*!
          Payload:
            -# \c qint32(messageId) - the message id is used for association with the message
            -# \c QString(receiver) - username of the receiver
            -# \c QString(reason) - reason of the failure
         */
        MessageFailure,

        //! Used for illegal packet types that don't match any valid packet type.
        IllegalPacketType = -1
    };

    //! Returns a human-readable description of the packet type.
    /*!
      \param type The packet type.
      \return The status as a \c QString.
     */
    static QString typeString(PacketType type);

    //! Constructs a QccPacket.
    /*!
      \param type The type of the packet.
     */
    QccPacket(PacketType type = Message);

    //! Returns the packet type.
    /*!
      \return The packet type.
      \sa typeString()
     */
    inline PacketType type() const { return m_type; }

    //! Returns a human-readable description of the current packet type.
    /*!
      \return The status as a \c QString.
      \sa type()
     */
    inline QString typeString() const { return QccPacket::PacketTypeNames[m_type]; }

    //! Returns the packet data.
    /*!
      \return The packet data.
      \sa stream()
     */
    inline QByteArray& data() { return m_data; }

    //! Returns the internal data stream to provide easy data serialization.
    /*!
      \return The packet data stream.
      \sa data()
     */
    inline QDataStream& stream() { return m_stream; }

    //! Returns the size of the playload.
    /*!
      \return The size of the playload.
     */
    inline int size() const { return m_data.size() - sizeof(quint32); }

    //! Writes the data to the \a socket and returns \c true on success.
    /*!
      \param socket The TCP-Socket.
      \return \c True if the data was successfully written to the socket; otherwise returns \c false.
     */
    bool send(QTcpSocket *socket);

private:
    //! The string representation of the the different types of QCC-packets.
    /*!
      \sa PacketType
     */
    static const char *PacketTypeNames[];

    PacketType m_type;    //!< The packet type.
    QByteArray m_data;    //!< The packet data.
    QDataStream m_stream; //!< The packet data stream.
};

#endif // TCPPACKET_H
