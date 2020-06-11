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

#include "user.h"

#include <QTcpSocket>

User::User(QObject *parent) :
    AbstractUser(parent), m_socket(NULL)
{
}

User::User(const QString &username, const QString &password, QObject *parent) :
    AbstractUser(username, parent), m_password(password), m_socket(NULL)
{
}

User* User::readUser(QXmlStreamReader &xml)
{
    QXmlStreamAttributes attr = xml.attributes();
    QString username = attr.value("username").toString();
    QString password = attr.value("password").toString();
    User *user = new User(username, password);
    while (!xml.atEnd()) {
        switch (xml.readNext()) {
        case QXmlStreamReader::Invalid:
            return NULL;
        case QXmlStreamReader::StartElement:
            if (xml.name() == "contact") {
                if (xml.readNext() == QXmlStreamReader::Characters) {
                    QString contactName = xml.text().toString();
                    user->addContact(contactName);
                }
            }
            break;
        case QXmlStreamReader::EndElement:
            if (xml.name() == "user")
                return user;
            else
                break;
        default:
            break;
        }
    }
    return NULL;
}

void User::writeUser(QXmlStreamWriter &xml)
{
    xml.writeStartElement("user");
    xml.writeAttribute("username", m_username);
    xml.writeAttribute("password", m_password);
    xml.writeStartElement("contacts");
    foreach (const QString &contactName, m_contacts)
        xml.writeTextElement("contact", contactName);
    xml.writeEndElement();
    xml.writeEndElement();
}

void User::reset()
{
    m_socket = NULL;
    AbstractUser::reset();
}
