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

#include "messagewindow.h"
#include "ui_messagewindow.h"
#include "contact.h"
#include "messagepage.h"

MessageWindow::MessageWindow(QTcpSocket *socket, QWidget *parent) :
    QTabWidget(parent), ui(new Ui::MessageWindow), m_socket(socket)
{
    ui->setupUi(this);
    connect(this, SIGNAL(tabCloseRequested(int)), SLOT(tabCloseRequested(int)));
}

MessageWindow::~MessageWindow()
{
    qDeleteAll(m_pages.values());
    delete ui;
}

bool MessageWindow::addTab(Contact *contact)
{
    if (!contact)
        return false;

    bool tabAdded = false;
    MessagePage *page = NULL;

    if (m_pages.contains(contact)) {
        page = m_pages.value(contact);
        setCurrentWidget(page);
    } else {
        page = new MessagePage(m_socket, contact);
        connect(contact, SIGNAL(statusChanged()), SLOT(contact_statusChanged()));
        connect(page, SIGNAL(closeButtonClicked()), SLOT(page_closeButtonClicked()));
        int index = QTabWidget::addTab(page, contact->statusIcon(), contact->username());
        m_pages.insert(contact, page);
        setCurrentIndex(index);
        tabAdded = true;
    }

    show();
    raise();
    activateWindow();
    if (page != NULL)
        page->setFocusOnInput();

    return tabAdded;
}

void MessageWindow::closeTab(Contact *contact)
{
    if (!m_pages.contains(contact))
        return;

    // find tab and close it
    for (int i = 0; i < count(); i++) {
        if (tabText(i) == contact->username()) {
            tabCloseRequested(i);
            return;
        }
    }
}

void MessageWindow::closeAllTabs()
{
    clear();
    qDeleteAll(m_pages.values());
    m_pages.clear();
}

void MessageWindow::appendMessage(Contact *contact, const QString &message)
{
    if (!contact)
        return;

    addTab(contact);
    m_pages.value(contact)->appendMessage(message, Qt::red);
}

void MessageWindow::contact_statusChanged()
{
    Contact *contact = qobject_cast<Contact*>(sender());
    if (!contact) {
        qWarning("MessageWindow::contact_statusChanged(): Cast of sender() to Contact* failed");
        return;
    }

    // find tab and update the icon
    for (int i = 0; i < count(); i++) {
        if (tabText(i) == contact->username()) {
            setTabIcon(i, contact->statusIcon());
            return;
        }
    }
}

void MessageWindow::tabCloseRequested(int index)
{
    MessagePage *page = qobject_cast<MessagePage*>(widget(index));
    if (!page) {
        qWarning("MessageWindow::tabCloseRequested(int): Cast of sender() to MessagePage* failed");
        return;
    }

    m_pages.remove(page->contact());
    removeTab(index);
    disconnect(page->contact(), SIGNAL(statusChanged()), this, SLOT(contact_statusChanged()));
    delete page;

#ifdef DEBUG
    qDebug("tab closed %i", index);
#endif

    if (count() == 0) {
#ifdef DEBUG
    qDebug("last tab closed => closing MessageWindow");
#endif
        close();
    }
}

void MessageWindow::page_closeButtonClicked()
{
    emit tabCloseRequested(currentIndex());
}
