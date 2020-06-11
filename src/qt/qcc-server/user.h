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

#ifndef USER_H
#define USER_H

#include "abstractuser.h"

#include <QSet>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

QT_BEGIN_NAMESPACE
class QTcpSocket;
QT_END_NAMESPACE

//! The User class represents a single user on the server.
/*!
  \ingroup server
 */
class User : public AbstractUser
{
    Q_OBJECT

public:

    //! Constructs an empty user.
    /*!
      \param parent The parent object.
     */
    explicit User(QObject *parent = 0);

    //! Constructs an user with a username and password.
    /*!
      \param username The username.
      \param password The password.
      \param parent The parent object.
     */
    User(const QString &username, const QString &password, QObject *parent = 0);

    //! Returns the password.
    /*!
      \return The password.
      \sa setPassword(), matchPassword()
     */
    inline const QString& password() const { return m_password; }

    //! Returns the set of contacts.
    /*!
      \return The set of contacts.
      \sa addContact(), containsContact(), removeContact(), clearContacts()
     */
    inline const QSet<QString>& contacts() const { return m_contacts; }

    //! Returns the RSA public key in DER-format.
    /*!
      \return The RSA public key in DER-format.
      \sa setPublicKey()
     */
    inline const QByteArray& publicKey() const { return m_publicKey; }

    //! Returns the TCP-socket.
    /*!
      \return The TCP-socket.
      \sa setSocket()
     */
    inline QTcpSocket* socket() const { return m_socket; }

    //! Adds a contact to the user's set of contacts.
    /*!
      \param username The username of the contact.
      \note If the user already has this contact on his set of contacts, nothing happens.
      \sa contacts(), containsContact()
     */
    inline void addContact(const QString &username) { m_contacts.insert(username); }

    //! Checks whether the user's set of contacts contains an user.
    /*!
      \param username The username of the contact to check.
      \return \c True if the user's set of contacts contains the contact; otherwise returns \c false.
      \sa contacts(), addContact(), removeContact()
     */
    inline bool containsContact(const QString &username) const { return m_contacts.contains(username); }

    //! Removes a contact from the user's set of contacts.
    /*!
      \param username The username of the contact to remove.
      \return \c True if the contact has been removed; otherwise returns \c false.
      \sa contacts(), containsContact(), clearContacts()
     */
    inline bool removeContact(const QString &username) { return m_contacts.remove(username); }

    //! Removes all contact from the user's set of contacts.
    /*!
      \sa contacts(), removeContact()
     */
    inline void clearContacts() { m_contacts.clear(); }

    //! Compares the user's password with another one.
    /*!
      \param password The password to compare with.
      \return \c True if the passwords match; otherwise returns \c false.
      \sa password()
     */
    inline bool matchPassword(const QString &password) const { return m_password == password; }

    //! Reads one user from the given XML-stream.
    /*!
      \param xml The XML-stream from which the user will be read.
      \return The read user object or \c NULL if an error occured.
      \sa writeUser()
     */
    static User* readUser(QXmlStreamReader &xml);

    //! Writes this user to the given XML-stream.
    /*!
      \param xml The XML-stream to which the user will be written.
      \sa readUser()
     */
    void writeUser(QXmlStreamWriter &xml);

public slots:

    //! Sets the password.
    /*!
      \param password The password.
      \sa password()
     */
    void setPassword(const QString &password) { m_password = password; }

    //! Sets the public key.
    /*!
      \param publicKey The RSA public key in DER-format.
      \sa publicKey()
     */
    void setPublicKey(const QByteArray &publicKey) { m_publicKey = publicKey; }

    //! Sets the TCP-socket.
    /*!
      \param socket The TCP-socket.
      \sa socket()
     */
    void setSocket(QTcpSocket *socket) { m_socket = socket; }

    //! Resets the state of the user object.
    void reset();

private:

    QString m_password;       //!< The password (should be in some cryptographic hash format).
    QSet<QString> m_contacts; //!< The set of contacts.
    QByteArray m_publicKey;   //!< RSA public key in DER-format.
    QTcpSocket *m_socket;     //!< The TCP-socket.
};

#endif // USER_H
