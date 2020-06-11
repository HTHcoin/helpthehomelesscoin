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

#ifndef MESSAGEWINDOW_H
#define MESSAGEWINDOW_H

#include <QTabWidget>
#include <QHash>

QT_BEGIN_NAMESPACE
class QTcpSocket;
QT_BEGIN_NAMESPACE

class Contact;
class MessagePage;

//! The UI namespace.
namespace Ui {
    class MessageWindow;
}

//! The MessageWindow class defines the tabbed message window.
/*!
  \ingroup client
 */
class MessageWindow : public QTabWidget
{
    Q_OBJECT

public:

    //! Constructs a MessageWindow.
    /*!
      \param socket The TCP-socket to the server.
      \param parent The parent object.
     */
    explicit MessageWindow(QTcpSocket *socket, QWidget *parent = 0);

    //! Destroys the MessageWindow.
    ~MessageWindow();

    //! Adds a tab to the window.
    /*!
      \param contact The contact for the tab.
      \return \c True if a new tab has been added; otherwise returns \c false.
      \note If a tab for the contact already exists, the existing tab gets the focus.
      \sa closeTab(), appendMessage()
     */
    bool addTab(Contact *contact);

    //! Closes the tab for the \a contact (if any).
    /*!
      \param contact The contact of the tab that will be closed.
      \sa addTab(), closeAllTabs()
     */
    void closeTab(Contact *contact);

    //! Closes all tabs.
    /*!
      \sa closeTab()
     */
    void closeAllTabs();

    //! Appends a \a message to the tab of the given \a contact.
    /*!
      \param contact The sender of the message.
      \param message The message to append.
      \note If there is no tab for the \a contact, one will be created.
      \sa addTab()
     */
    void appendMessage(Contact *contact, const QString &message);

private slots:

    //! This slot should be called from a \a contact whenever the status changes.
    void contact_statusChanged();

    //! Removes the tab with the given \a index.
    /*!
      \param index The index of the tab to remove.
     */
    void tabCloseRequested(int index);

    //! Closes the current tab.
    /*!
      \sa QTabWidget::currentIndex()
     */
    void page_closeButtonClicked();

private:

    Ui::MessageWindow *ui;                 //!< Pointer to the UI.
    QTcpSocket *m_socket;                  //!< The TCP-socket to the server.
    QHash<Contact*, MessagePage*> m_pages; //!< A set of the contacts for all open tabs.
};

#endif // MESSAGEWINDOW_H
