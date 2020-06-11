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

#ifndef ABSTRACTUSER_H
#define ABSTRACTUSER_H

#include <QObject>

/*!
  \defgroup core QCC core library

  The core library contains classes which are used in both other QCC-modules,
  the \ref server "QCC-server" and the \ref client "QCC-client".
 */

//! The AbstractUser class is the base class for all user objects.
/*!
  \ingroup core
 */
class AbstractUser : public QObject
{
    Q_OBJECT

public:

    //! This enum describes the different states in which a user can be.
    /*!
      \sa StatusNames
     */
    enum Status {
        Offline, //!< The user is offline.
        Online   //!< The user is online.
    };

    //! Returns a human-readable description of the status.
    /*!
      \param type The status.
      \return The status as a \c QString.
     */
    static QString statusString(Status type);

    //! Constructs an empty user.
    /*!
      \param parent The parent object.
     */
    explicit AbstractUser(QObject *parent = 0);

    //! Constructs an user with the username set to \a username.
    /*!
      \param username The username.
      \param parent The parent object.
     */
    AbstractUser(const QString &username, QObject *parent = 0);

    //! Returns the username.
    /*!
      \return The username.
      \sa setUsername()
     */
    inline const QString& username() const { return m_username; }

    //! Returns the status.
    /*!
      \return The status.
      \sa setStatus(), statusString(), statusChanged()
     */
    inline Status status() const { return m_status; }

    //! Returns a human-readable description of the current status.
    /*!
      \return The status as a \c QString.
      \sa status()
     */
    inline QString statusString() const { return AbstractUser::StatusNames[m_status]; }

    //! Returns \c true if the user object is valid; otherwise returns \c false.
    /*!
      \return \c True if the user object is valid; otherwise returns \c false.
      \sa invalidate()
     */
    inline bool isValid() const { return !m_username.isEmpty(); }

    //! Invalidates the user object.
    /*!
      \sa isValid()
     */
    inline void invalidate() { m_username.clear(); }

    //! Returns \c true if the user is online; otherwise returns \c false.
    /*!
      \return \c True if the user is online; otherwise returns \c false.
      \sa status()
     */
    inline bool isOnline() const { return m_status == Online; }

signals:

    //! This signal is emitted whenever the user's status changes.
    /*!
      \sa status()
     */
    void statusChanged();

public slots:

    //! Sets the username.
    /*!
      \param username The username.
      \sa username()
     */
    inline void setUsername(const QString &username) { m_username = username; }

    //! Sets the status.
    /*!
      \param status The status.
      \sa status(), statusChanged()
     */
    void setStatus(Status status);

    //! Resets the state of the user object.
    virtual void reset();

protected:

    QString m_username; //!< The username.
    Status m_status;    //!< The status.

private:

    //! The string representation of the the different user states.
    /*!
      \sa Status
     */
    static const char *StatusNames[];
};

#endif // ABSTRACTUSER_H
