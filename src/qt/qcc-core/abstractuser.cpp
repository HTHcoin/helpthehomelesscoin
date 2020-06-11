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

#include "abstractuser.h"

const char* AbstractUser::StatusNames[] = {
    "Offline",
    "Online"
};

QString AbstractUser::statusString(Status status)
{
    return AbstractUser::StatusNames[status];
}

AbstractUser::AbstractUser(QObject *parent) :
    QObject(parent), m_status(Offline)
{
}

AbstractUser::AbstractUser(const QString &username, QObject *parent) :
    QObject(parent), m_username(username), m_status(Offline)
{
}

void AbstractUser::setStatus(Status status)
{
    if (status == m_status)
        return;

    m_status = status;
    emit statusChanged();
}

void AbstractUser::reset()
{
    setStatus(Offline);
}
