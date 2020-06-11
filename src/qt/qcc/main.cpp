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

#include <QtGui/QApplication>
#include "mainwindow.h"

/*!
  \defgroup client QCC client module

  The QCC-client can connect to a \ref server "QCC-server" to send and receive instant
  messages from other QCC-clients and also see their online status.
 */

//! The main entry point of this application.
/*!
  \param argc The command-line argument count.
  \param argv The command-line argument-array.
  \return The return code of the application
  \ingroup client
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
