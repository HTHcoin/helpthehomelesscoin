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

#ifndef CONTACTLISTMODEL_H
#define CONTACTLISTMODEL_H

#include <QAbstractListModel>
#include <QList>

class Contact;

//! The ContactListModel class provides a model to store and manage a list of contacts.
/*!
  \ingroup client
  \sa Contact
 */
class ContactListModel : public QAbstractListModel
{
    Q_OBJECT

public:

    //! Constructs an empty ContactListModel.
    /*!
      \param parent The parent object.
     */
    explicit ContactListModel(QObject *parent = 0);

    //! Destroys the ContactListModel.
    ~ContactListModel();

    //! Returns contact count.
    /*!
      \return The contact count.
     */
    int rowCount(const QModelIndex &) const;

    //! Returns the data stored under the given \a role for the contact referred to by the \a index.
    /*!
      \param index The index for the contact.
      \param role The role.
      \return The data.
      \sa contact()
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    //! Returns the contact with the given \a name.
    /*!
      \param contactName The name of the contact.
      \return The contact.
      \sa data()
     */
    Contact* contact(const QString &contactName);

    //! Returns the contact with the given \a index.
    /*!
      \param index The index of the contact.
      \return The contact.
      \sa data(), add()
     */
    Contact* contact(const QModelIndex &index);

    //! Adds a new contact to this model.
    /*!
      \param contact The contact to add.
      \sa contact(), remove()
     */
    void add(Contact *contact);

    //! Adds a list of contacts to this model.
    /*!
      \param contacts The list of contacts to add.
      \sa remove(), clear()
     */
    void add(const QList<Contact*> &contacts);

    //! Removes the \a contact from this model.
    /*!
      \param contact The contact to remove.
      \return \c True if the contact was removed; returns \c false otherwise.
      \sa add(), clear()
     */
    bool remove(const Contact *contact);

    //! Removes the contact with the given \a name from this model.
    /*!
      \param contactName The name of the contact to remove.
      \return \c True if the contact was removed; returns \c false otherwise.
      \sa add(), clear()
     */
    bool remove(const QString &contactName);

    //! Removes the contact with the given \a index from this model.
    /*!
      \param index The index of the contact to remove.
      \return \c True if the contact was removed; returns \c false otherwise.
      \sa add(), clear()
     */
    bool remove(const QModelIndex &index);

public slots:

    //! Removes all contacts from this model.
    void clear();

private slots:

    //! This slot should be called from a \a contact whenever the status changes.
    void contact_statusChanged();

private:

    QList<Contact*> m_contacts; //!< The list of contacts.
};

#endif // CONTACTLISTMODEL_H
