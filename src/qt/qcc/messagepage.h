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

#ifndef MESSAGEPAGE_H
#define MESSAGEPAGE_H

#include <QWidget>
#include <QIcon>

QT_BEGIN_NAMESPACE
class QTcpSocket;
QT_BEGIN_NAMESPACE

//! The UI namespace.
namespace Ui {
    class MessagePage;
}

class Contact;

//! The MessagePage class defines one tab of the MessageWindow.
/*!
  \ingroup client
 */
class MessagePage : public QWidget
{
    Q_OBJECT

public:

    //! Constructs a MessagePage.
    /*!
      \param socket The TCP-socket to the server.
      \param contact The contact associated with this page.
      \param parent The parent object.
     */
    MessagePage(QTcpSocket *socket, Contact *contact, QWidget *parent = 0);

    //! Destroys the MessagePage.
    ~MessagePage();

    //! Returns the contact of this page.
    /*!
      \return The contact of this page.
     */
    inline Contact* contact() { return m_contact; }

    //! Appends a \a message to this page.
    /*!
      \param message The message to append.
      \param color The color for the \a message.
      \sa MessageWindow::appendMessage()
     */
    void appendMessage(const QString &message, const QColor &color = Qt::black);

Q_SIGNALS:

    //! This signal is emitted whenever the close button of this page is clicked.
    void closeButtonClicked();

public Q_SLOTS:

    //! Sets the focus on the input field of this page.
    void setFocusOnInput();

private Q_SLOTS:

    //! Sends the \a message from the input field to the server.
    void on_sendButton_clicked();

    //! Prints a status changed text to the chat field.
    void contact_statusChanged();

private:

    Ui::MessagePage *ui;  //!< Pointer to the UI.
    QTcpSocket *m_socket; //!< The TCP-socket to the server.
    Contact *m_contact;   //!< Pointer to the contact of this page.

    //! Filters key events for the input edit to send messages via the \c Return and \c Enter key.
    /*!
      \param obj The object to be filtered.
      \param event The event that occured.
      \return \c True to forward this event; otherwise returns \c false.
      \sa QObject::eventFilter()
     */
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // MESSAGEPAGE_H
