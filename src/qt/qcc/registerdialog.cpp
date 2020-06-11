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

#include "registerdialog.h"
#include "ui_registerdialog.h"

#include <QMessageBox>

RegisterDialog::RegisterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

QString RegisterDialog::username() const
{
    return ui->usernameLineEdit->text();
}

QString RegisterDialog::password() const
{
    return ui->password1LineEdit->text();
}

void RegisterDialog::on_buttonBox_accepted()
{
    QString username = ui->usernameLineEdit->text();
    if (username.isEmpty() || username.length() < 3) {
        QMessageBox::warning(this, "Username error", "The username must be at least 3 characters long.");
        return;
    }

    QString password = ui->password1LineEdit->text();
    if (password.isEmpty() || password.length() < 3) {
        QMessageBox::warning(this, "Password error", "The password must be at least 3 characters long.");
        return;
    }
    if (password != ui->password2LineEdit->text()) {
        QMessageBox::warning(this, "Password error", "The passwords do not match.");
        return;
    }

    accept();
}
