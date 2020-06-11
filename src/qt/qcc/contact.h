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

#ifndef CONTACT_H
#define CONTACT_H

#include "abstractuser.h"

#include <QIcon>
#include <QtCrypto>

//! The Contact class represents a single user on the client.
/*!
  \ingroup client
 */
class Contact : public AbstractUser
{
    Q_OBJECT

public:

    //! Constructs an empty contact.
    /*!
      \param parent The parent object.
     */
    explicit Contact(QObject *parent = 0);

    //! Constructs a contact with a username.
    /*!
      \param username The username.
      \param parent The parent object.
     */
    Contact(const QString &username, QObject *parent = 0);

    //! Returns the RSA public.
    /*!
      \return The RSA public key.
      \sa setPublicKey()
     */
    inline const QCA::PublicKey& publicKey() const { return m_publicKey; }

    //! Sets the public key.
    /*!
      \param publicKey The RSA public key in DER-format.
      \sa publicKey()
     */
    void setPublicKey(const QByteArray &publicKey);

    //! Returns the status icon.
    /*!
      \return The status icon.
      \sa AbstractUser::status()
     */
    QIcon statusIcon() const;

    //! Returns the encrypted \a text with the RSA public key.
    /*!
      \return The encrypted \a text.
      \sa publicKey()
     */
    QByteArray encrypt(const QString &text);

private:

    //! The status icons.
    static QIcon OfflineIcon, OnlineIcon;

    QCA::PublicKey m_publicKey; //!< The RSA public key.

    //! Initializes the static icons.
    void init();
};

#endif // CONTACT_H
