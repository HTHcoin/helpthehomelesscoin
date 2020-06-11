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

#include "messagepage.h"
#include "ui_messagepage.h"
#include "qccpacket.h"
#include "contact.h"

#include <QKeyEvent>

MessagePage::MessagePage(QTcpSocket *socket, Contact *contact, QWidget *parent) :
    QWidget(parent), ui(new Ui::MessagePage), m_socket(socket), m_contact(contact)
{
    Q_ASSERT(m_contact); // should never happen, just in case...

    ui->setupUi(this);
    connect(m_contact, SIGNAL(statusChanged()), SLOT(contact_statusChanged()));
    connect(ui->closeButton, SIGNAL(clicked()), SIGNAL(closeButtonClicked()));
    ui->messageTextEdit->installEventFilter(this);
}

MessagePage::~MessagePage()
{
    delete ui;
}

void MessagePage::appendMessage(const QString &message, const QColor &color)
{
    QString messageHtml = message;
    messageHtml.replace('\n', "<br/>");
    ui->messagesTextEdit->append("<span style=\"color: " + color.name() + ";\">" +
                                 m_contact->username() + "</span>: " + messageHtml);
}

void MessagePage::contact_statusChanged()
{
    ui->messagesTextEdit->append(QString("%1 is now %2.").
                                 arg(m_contact->username()).
                                 arg(m_contact->statusString()));
}

void MessagePage::on_sendButton_clicked()
{
    QString text = ui->messageTextEdit->toPlainText().trimmed();
    if (text.isEmpty() || !m_contact->isOnline())
        return;

    QccPacket message;
    message.stream() << qint32(qrand()) << m_contact->username() << m_contact->encrypt(text);
    message.send(m_socket);

    text.replace('\n', "<br/>");
    ui->messagesTextEdit->append("<span style=\"color: #00f;\">You</span>: " + text);

    ui->messageTextEdit->clear();
    ui->messageTextEdit->setFocus();
}

void MessagePage::setFocusOnInput()
{
    ui->messageTextEdit->setFocus();
}

bool MessagePage::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->messageTextEdit) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if ((keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
                    && keyEvent->modifiers() == Qt::NoModifier) {
                on_sendButton_clicked();
                return true;
            } else
                return false;
        } else
            return false;
    } else
        return QWidget::eventFilter(obj, event);
}
