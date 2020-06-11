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

#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>

//! The UI namespace.
namespace Ui {
    class RegisterDialog;
}

//! The RegisterDialog class defines the register window.
/*!
  \ingroup client
 */
class RegisterDialog : public QDialog
{
    Q_OBJECT

public:

    //! Constructs a RegisterDialog.
    /*!
      \param parent The parent object.
     */
    explicit RegisterDialog(QWidget *parent = 0);

    //! Destroys the RegisterDialog.
    ~RegisterDialog();

    //! Returns the username.
    /*!
      \return The username.
     */
    QString username() const;

    //! Returns the password.
    /*!
      \return The password.
     */
    QString password() const;

private Q_SLOTS:

    //! Checks if the username and password is valid and closes the window.
    void on_buttonBox_accepted();

private:

    Ui::RegisterDialog *ui; //!< Pointer to the UI.
};

#endif // REGISTERDIALOG_H
