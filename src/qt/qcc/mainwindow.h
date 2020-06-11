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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QModelIndex>
#include <QtCrypto>

class ContactListModel;
class MessageWindow;

//! The UI namespace.
namespace Ui {
    class MainWindow;
}

//! The MainWindow class defines the main window of this application.
/*!
  \ingroup client
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    //! Constructs the MainWindow.
    /*!
      \param parent The parent object.
     */
    explicit MainWindow(QWidget *parent = 0);

    //! Destroys the MainWindow.
    ~MainWindow();

private Q_SLOTS:

    //! Connects the client to a QCC-server.
    /*!
      \sa \ref server "QCC server module"
     */
    void connectToHost();

    //! The client is connected to the server.
    /*!
      \sa connectToHost()
     */
    void socket_connected();

    //! Closes all open message windows and returns to the login screen.
    void socket_disconnected();

    //! Handles all incoming data packets from the server.
    void socket_readyRead();

    //! Handles all socket error messages.
    /*!
      \param error The socket error.
     */
    void socket_error(QAbstractSocket::SocketError error);

#ifdef DEBUG
    //! Prints the sockets state to the debug output (in debug mode only).
    /*!
      \param state The socket state.
     */
    void socket_stateChanged(QAbstractSocket::SocketState state);
#endif

    //! Removes the current contact from the contact list on the remote server.
    void removeCurrentContact(bool);

    //! Establishes a connection to the QCC-server and sends a user authentication request.
    /*!
      \sa connectToHost(), QccPacket::UserAuthentication
     */
    void on_loginButton_clicked();

    //! Establishes a connection to the QCC-server and sends a user register request.
    /*!
      \sa connectToHost(), QccPacket::UserRegister
     */
    void on_registerButton_clicked();

    //! Opens the activated contact's tab of the message window.
    /*!
      \sa MessageWindow
     */
    void on_contactListView_activated(const QModelIndex &index);

    //! Shows the context menu to remove the selected contact.
    /*!
      \param pos The position of the context menu.
      \sa QccPacket::RemoveContact
     */
    void on_contactListView_customContextMenuRequested(const QPoint &pos);

    //! Sends a authorization request to the QCC-server.
    /*!
      \sa QccPacket::RequestAuthorization
     */
    void on_addContactButton_clicked();

private:

    //! The default network port, which will be used if no port is provided by the user.
    static const quint16 DEFAULT_PORT = 54321;

    Ui::MainWindow *ui;             //!< Pointer to the UI:
    ContactListModel *m_contacts;   //!< The contact list model.
    MessageWindow *m_messageWindow; //!< The message window.
    QTcpSocket m_socket;            //!< The TCP-socket for the connection with the QCC-server.
    quint32 m_packetSize;           //!< The packet size of the current network packet from the server.
    bool m_register;                //!< Whether or not the user wants to register a new user or just log in.

    QCA::PrivateKey m_privateKey;   //!< The RSA private key used for the message encryption.

    //! Exits the application.
    void closeEvent(QCloseEvent *);
};

#endif // MAINWINDOW_H
