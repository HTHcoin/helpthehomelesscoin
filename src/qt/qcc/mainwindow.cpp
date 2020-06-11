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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qccpacket.h"
#include "contact.h"
#include "contactlistmodel.h"
#include "messagewindow.h"
#include "registerdialog.h"

#include <QMessageBox>
#include <QMenu>
#include <QTimer>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), m_contacts(new ContactListModel(this)),
    m_messageWindow(new MessageWindow(&m_socket)), m_packetSize(0)
{
    ui->setupUi(this);

    connect(&m_socket, SIGNAL(connected()), SLOT(socket_connected()));
    connect(&m_socket, SIGNAL(disconnected()), SLOT(socket_disconnected()));
    connect(&m_socket, SIGNAL(readyRead()), SLOT(socket_readyRead()));
    connect(&m_socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(socket_error(QAbstractSocket::SocketError)));
#ifdef DEBUG
    connect(&m_socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SLOT(socket_stateChanged(QAbstractSocket::SocketState)));
#endif

    ui->contactListView->setModel(m_contacts);

    QCA::init();
    if (!QCA::isSupported("sha256") || !QCA::isSupported("pkey") || !QCA::PKey::supportedIOTypes().contains(QCA::PKey::RSA)) {
        QMessageBox::critical(this, "QCA error", "QCA OpenSSL-Plugin not found!\n\nThe application will now quit.");
        QTimer::singleShot(100, qApp, SLOT(quit()));
    }
}

MainWindow::~MainWindow()
{
    delete m_messageWindow;
    delete m_contacts;
    delete ui;
}

void MainWindow::connectToHost()
{
    if (m_socket.state() > 0)
        return;

    QStringList server = ui->serverLineEdit->text().split(':');
    QString host = server.at(0);
    quint16 port = server.length() < 2 ? MainWindow::DEFAULT_PORT : server.at(1).toUShort();
    //qDebug("server = %s:%d", qPrintable(host), port);

    m_packetSize = 0;
    m_socket.connectToHost(host, port);
    ui->loginButton->setEnabled(false);
    ui->registerButton->setEnabled(false);
}

void MainWindow::socket_connected()
{
#ifdef DEBUG
    qDebug("MainWindow::socket_connected()");
#endif
}

void MainWindow::socket_disconnected()
{
#ifdef DEBUG
    qDebug("MainWindow::socket_disconnected()");
#endif

    if (isHidden()) // hack, true when window is closing
        return;

    ui->stackedWidget->setCurrentIndex(0);
    setWindowTitle("QCC");
    m_messageWindow->closeAllTabs();
    m_messageWindow->close();
    ui->loginButton->setEnabled(true);
    ui->registerButton->setEnabled(true);
}

void MainWindow::socket_readyRead()
{
#ifdef DEBUG
    qDebug("MainWindow::socket_readyRead(): %li bytes available", (long)m_socket.bytesAvailable());
#endif

    QDataStream in(&m_socket);
    in.setVersion(QDataStream::Qt_4_0);
    if (m_packetSize == 0) {
        if (m_socket.bytesAvailable() < (int)sizeof(quint32)) // packet size
            return;
        in >> m_packetSize;
    }
    if (m_socket.bytesAvailable() < m_packetSize)
        return;
    m_packetSize = 0; // reset packet size

    qint32 type;
    in >> type;

#ifdef DEBUG
    qDebug("PacketType = %i (%s)", type, qPrintable(QccPacket::typeString((QccPacket::PacketType)type)));
#endif

    switch ((QccPacket::PacketType)type) {
    case QccPacket::ConnectionAccepted:
    {
        m_privateKey = QCA::KeyGenerator().createRSA(1024);
        if (m_privateKey.isNull() || !m_privateKey.canDecrypt()) {
            qWarning("Failed to generate private key");
            break;
        }
        QccPacket packet(m_register ? QccPacket::UserRegister : QccPacket::UserAuthentication);
        packet.stream() << ui->usernameLineEdit->text()
                        << QCA::Hash("sha256").hashToString(ui->passwordLineEdit->text().toUtf8())
                        << m_privateKey.toPublicKey().toDER();
        packet.send(&m_socket);
        break;
    }
    case QccPacket::ConnectionRefused:
        QMessageBox::critical(this, "Connection Refused", "Access denied!");
        break;
    case QccPacket::RegisterSuccess:
        QMessageBox::information(this, "Register Success", "Your registration has been successful.\n"
                                 "You will now be logged in...");
        ui->stackedWidget->setCurrentIndex(1);
        setWindowTitle("QCC - " + ui->usernameLineEdit->text());
        break;
    case QccPacket::RegisterFailure:
    {
        QString reason;
        in >> reason;
        m_socket.disconnectFromHost();
        QMessageBox::warning(this, "Register Failure", reason);
        break;
    }
    case QccPacket::AuthenticationSuccess:
        QccPacket(QccPacket::RequestContactList).send(&m_socket);
        ui->stackedWidget->setCurrentIndex(1);
        setWindowTitle("QCC - " + ui->usernameLineEdit->text());
        break;
    case QccPacket::AuthenticationFailure:
    {
        QString reason;
        in >> reason;
        m_socket.disconnectFromHost();
        QMessageBox::warning(this, "Authentication Failure", reason);
        break;
    }
    case QccPacket::RequestAuthorization:
    {
        QString username;
        in >> username;

        int question = QMessageBox::question(this, "Request Authorization", "The user \"" + username +
                                           "\" would like to add you to her/his contact list.\n"
                                           "Do you accept the authorization request?",
                                           "Accept", "Decline");

        QccPacket packet(question == 0 ? QccPacket::AuthorizationAccepted : QccPacket::AuthorizationDeclined);
        packet.stream() << username;
        packet.send(&m_socket);
        break;
    }
    case QccPacket::AuthorizationAccepted:
    {
        QString username;
        qint32 status;
        QByteArray publicKey;
        in >> username >> status >> publicKey;
        Contact *contact = m_contacts->contact(username);
        if (!contact) {
            contact = new Contact(username);
            m_contacts->add(contact);
        }
        contact->setPublicKey(publicKey);
        contact->setStatus((Contact::Status)status);
        break;
    }
    case QccPacket::AuthorizationDeclined:
    {
        QString username;
        in >> username;
        QMessageBox::information(this, "Authorization Declined", "The user \"" + username + "\" has declined your authorization request.");
        break;
    }
    case QccPacket::AuthorizationFailure:
    {
        QString reason;
        in >> reason;
        QMessageBox::warning(this, "Authorization Failure", reason);
        break;
    }
    case QccPacket::ContactList:
    {
        qint32 contactCount;
        in >> contactCount;
        m_contacts->clear();
        QList<Contact*> contacts;
        for (int i = 0; i < contactCount; i++) {
            QString username;
            qint32 status;
            QByteArray publicKey;
            in >> username >> status >> publicKey;
            Contact *contact = new Contact(username);
            contact->setPublicKey(publicKey);
            contact->setStatus((Contact::Status)status);
            contacts.append(contact);
        }
        m_contacts->add(contacts);
        break;
    }
    case QccPacket::ContactStatusChanged:
    {
        QString username;
        qint32 status;
        QByteArray publicKey;
        in >> username >> status >> publicKey;
        Contact *contact = m_contacts->contact(username);
        if (contact) {
            contact->setPublicKey(publicKey);
            contact->setStatus((Contact::Status)status);
        }
        break;
    }
    case QccPacket::ContactRemoved:
    {
        QString username;
        in >> username;
        Contact *contact = m_contacts->contact(username);
        if (contact) {
            m_messageWindow->closeTab(contact);
            m_contacts->remove(contact);
        }
        break;
    }
    case QccPacket::Message:
    {
        qint32 id;
        QString username;
        QByteArray encryptedMessage;
        in >> id >> username >> encryptedMessage;

        Contact *contact = m_contacts->contact(username);
        if (!contact) // received message from unknown user (not on contact list)
            break;

        const int keySize = m_privateKey.bitSize() / 8;
        QCA::SecureArray message;
        for (int i = 0; i < encryptedMessage.length() / keySize; i++) {
            QCA::SecureArray messageBlock;
            if (!m_privateKey.decrypt(encryptedMessage.mid(i * keySize, keySize), &messageBlock, QCA::EME_PKCS1_OAEP)) {
                qWarning("Error decrypting");
                return;
            }
            message.append(messageBlock);
        }
        m_messageWindow->appendMessage(contact, QString(message.toByteArray()));

        QccPacket packet(QccPacket::MessageSuccess);
        packet.stream() << id << username;
        packet.send(&m_socket);

        break;
    }
    case QccPacket::MessageSuccess:
    {
        qint32 id;
        QString username;
        in >> id >> username;
        qDebug() << "MessageSuccess: " << id << " => " << username;
        break;
    }
    case QccPacket::MessageFailure:
    {
        qint32 id;
        QString username, reason;
        in >> id >> username >> reason;
        QMessageBox::warning(this, "Message Failure", reason);
        break;
    }
    default:
        qWarning("MainWindow::socket_readyRead(): Illegal PacketType %i", type);
        return;
    }
}

void MainWindow::socket_error(QAbstractSocket::SocketError error)
{
#ifdef DEBUG
    qDebug("MainWindow::socket_error(%i) => %s", error, qPrintable(m_socket.errorString()));
#endif

    QMessageBox::critical(this, "Connection Error", m_socket.errorString());

    ui->loginButton->setEnabled(true);
    ui->registerButton->setEnabled(true);
}

#ifdef DEBUG
void MainWindow::socket_stateChanged(QAbstractSocket::SocketState state)
{
    qDebug("MainWindow::socket_stateChanged(%i)", state);
}
#endif

void MainWindow::on_loginButton_clicked()
{
    m_register = false;
    connectToHost();
}

void MainWindow::on_registerButton_clicked()
{
    RegisterDialog dialog(this);
    dialog.exec();
    if (dialog.result()) {
        ui->usernameLineEdit->setText(dialog.username());
        ui->passwordLineEdit->setText(dialog.password());
        m_register = true;
        connectToHost();
    }
}

void MainWindow::on_contactListView_activated(const QModelIndex &index)
{
    Contact *contact = m_contacts->contact(index);
    if (!contact)
        return;

    m_messageWindow->addTab(contact);
}

void MainWindow::on_contactListView_customContextMenuRequested(const QPoint &pos)
{
    Contact *contact = m_contacts->contact(ui->contactListView->currentIndex());
    if (!contact)
        return;

    QMenu menu(ui->contactListView);
    menu.addAction("Remove contact \"" + contact->username() + "\"", this, SLOT(removeCurrentContact(bool)));
    menu.exec(ui->contactListView->mapToGlobal(pos));
}

void MainWindow::on_addContactButton_clicked()
{
    QString username = ui->contactLineEdit->text();
    if (username.isEmpty())
        return;

    QccPacket packet(QccPacket::RequestAuthorization);
    packet.stream() << username;
    packet.send(&m_socket);

    ui->contactLineEdit->clear();
}

void MainWindow::removeCurrentContact(bool)
{
    Contact *contact = m_contacts->contact(ui->contactListView->currentIndex());
    if (!contact || !contact->isValid())
        return;

    int question = QMessageBox::question(this, "Remove contact", "Do you really want to remove the contact \"" +
                                         contact->username() + "\" from your contact list?",
                                         QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (question == QMessageBox::Yes) {
        QccPacket packet(QccPacket::RemoveContact);
        packet.stream() << contact->username();
        packet.send(&m_socket);
    }
}

void MainWindow::closeEvent(QCloseEvent *)
{
    qApp->quit();
}
