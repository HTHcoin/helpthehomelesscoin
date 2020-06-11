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

#include <QtCore/QCoreApplication>
#include <QStringList>

#ifdef DEBUG
#include <QDebug>
#endif

#include "server.h"

/*!
  \defgroup server QCC server module

  The QCC-server manages all registered users and observes
  the package transfer between all connected \ref client "clients".
 */

//! The default network port, on which the server will listen.
/*!
  \ingroup server
 */
const quint16 DEFAULT_PORT = 54321;

//! Helper function for reading a command line argument of type \c quint16.
/*!
  \param arguments The list of command-line arguments.
  \param name The name of the agument.
  \param defaultValue The default value for the argument, in case there
                      is none or an error occurred while processing it.
  \return The value of the argument interpreted as an \c quint16 or the \a defaultValue.
  \ingroup server
 */
quint16 readUShort(const QStringList &arguments, const QString &name, quint16 defaultValue = 0)
{
    if (arguments.isEmpty())
        return defaultValue;

    int index = arguments.indexOf(name);
    if (index < 0 || index + 1 >= arguments.count())
        return defaultValue;

    bool ok;
    quint16 value = arguments.at(index + 1).toUShort(&ok);
    return ok ? value : defaultValue;
}

//! The main entry point of this application.
/*!
  \param argc The command-line argument count.
  \param argv The command-line argument-array.
  \return The return code of the application
  \ingroup server
 */
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QStringList arguments = QCoreApplication::arguments();

#ifdef DEBUG
    qDebug() << "QCC server started with arguments:" << arguments;
#endif

    QHostAddress adress = QHostAddress::Any;
    quint16 port = readUShort(arguments, "-port", DEFAULT_PORT);

    Server server;
    if (server.listen(adress, port)) {
        qDebug("The server is listening on interface %s, port %i",
               qPrintable(server.serverAddress().toString()), server.serverPort());
    } else {
        qCritical("Failed to start the server on interface %s, port %i: %s (error code %i)",
               qPrintable(adress.toString()), port, qPrintable(server.errorString()), server.serverError());
        return 1;
    }

    return app.exec();
}
