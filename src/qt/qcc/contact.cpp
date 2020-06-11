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

#include "contact.h"

QIcon Contact::OfflineIcon;
QIcon Contact::OnlineIcon;

Contact::Contact(QObject *parent) :
    AbstractUser(parent)
{
    init();
}

Contact::Contact(const QString &username, QObject *parent) :
    AbstractUser(username, parent)
{
    init();
}

void Contact::init()
{
    if (Contact::OfflineIcon.isNull()) {
        Contact::OfflineIcon = QIcon(":/icons/offline.png");
        Contact::OnlineIcon = QIcon(":/icons/online.png");
    }
}

void Contact::setPublicKey(const QByteArray &publicKey)
{
    if (publicKey.isEmpty()) // should only happen if the contact is offline
        return;

    QCA::ConvertResult conversionResult;
    m_publicKey = QCA::PublicKey::fromDER(publicKey, &conversionResult);
    if (conversionResult != QCA::ConvertGood || !m_publicKey.canEncrypt())
        qWarning("Contact::setPublicKey: not a valid public key");
}

QIcon Contact::statusIcon() const
{
    switch (m_status) {
    case Offline: return Contact::OfflineIcon;
    case Online: return Contact::OnlineIcon;
    default: return QIcon();
    }
}

QByteArray Contact::encrypt(const QString &text)
{
    QByteArray bytes = text.toAscii();
    QCA::SecureArray result;

    static const int padding = 42; // always constant?
    const int keySize = m_publicKey.bitSize() / 8;
    const int blockSize = keySize - padding;

    for (int i = 0; i < bytes.length(); i += blockSize)
        result.append(m_publicKey.encrypt(bytes.mid(i, blockSize), QCA::EME_PKCS1_OAEP));

    if (result.isEmpty())
        qWarning("Error encrypting");

    return result.toByteArray();
}
