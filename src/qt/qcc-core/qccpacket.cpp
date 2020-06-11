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

#include "qccpacket.h"

#include <QTcpSocket>

const char* QccPacket::PacketTypeNames[] = {
    "ConnectionAccepted",
    "ConnectionRefused",
    "UserRegister",
    "RegisterSuccess",
    "RegisterFailure",
    "UserAuthentication",
    "AuthenticationSuccess",
    "AuthenticationFailure",
    "RequestAuthorization",
    "AuthorizationAccepted",
    "AuthorizationDeclined",
    "AuthorizationFailure",
    "RequestContactList",
    "ContactList",
    "ContactStatusChanged",
    "RemoveContact",
    "ContactRemoved",
    "Message",
    "MessageSuccess",
    "MessageFailure",
    "IllegalMessage"
};

QString QccPacket::typeString(PacketType type)
{
    return QccPacket::PacketTypeNames[type];
}

QccPacket::QccPacket(PacketType type) :
    m_type(type), m_stream(&m_data, QIODevice::WriteOnly)
{
    m_stream.setVersion(QDataStream::Qt_4_0);
    m_stream << (quint32)0;
    m_stream << (qint32)type;
}

bool QccPacket::send(QTcpSocket *socket)
{
    if (!socket || m_data.length() < int(sizeof(quint32) + sizeof(qint32))) // size + type
        return false;

    m_stream.device()->seek(0);
    m_stream << (quint32)size();

    return socket->write(m_data) == m_data.size();
}
