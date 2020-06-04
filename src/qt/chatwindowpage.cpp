/*Copyright (C) 2009 Cleriot Simon
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA*/

#include "chatwindowpage.h"
#include "ui_chatwindowpage.h"

#include <QUrl>
#include <QDesktopServices>
#include <QtNetwork/QNetworkAccessManager>	
#include <QtNetwork/QNetworkReply>
#include <QProcess>
#include <QDir>

ChatWindowPage::ChatWindowPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::ChatWindowPage)
{
    ui->setupUi(this);
    setFixedSize(750,600);
    ui->splitter->hide();

	connect(ui->buttonConnect, SIGNAL(clicked()), this, SLOT(connecte()));

	connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui->actionCloseTab, SIGNAL(triggered()), this, SLOT(closeTab()));

	connect(ui->lineEdit, SIGNAL(returnPressed()), this, SLOT(sendCommande()));
	    
	    
	    ui->pushButton_WebChat->setStatusTip(tr("Visit Help The Homeless Worldwide Web Chat"));





    connect(ui->disconnect, SIGNAL(clicked()), this, SLOT(disconnectFromServer()));
	connect(ui->tab, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)) );
	connect(ui->tab, SIGNAL(tabCloseRequested(int)), this, SLOT(tabClosing(int)) );

}



void ChatWindowPage::tabChanged(int index)
{
	if(index!=0 && joining == false)
		currentTab()->updateUsersList(ui->tab->tabText(index));
}

void ChatWindowPage::tabClosing(int index)
{
	currentTab()->leave(ui->tab->tabText(index));
}
/*void ChatWindow::tabRemoved(int index)
{
	currentTab()->leave(ui->tab->tabText(index));
}*/

void ChatWindowPage::disconnectFromServer() {

    QMapIterator<QString, Serveur*> i(serveurs);

    while(i.hasNext())
    {
        i.next();
        QMapIterator<QString, QTextEdit*> i2(i.value()->conversations);
        while(i2.hasNext())
        {
            i2.next();
            i.value()->sendData("QUIT "+i2.key() + " ");
        }
    }


    ui->splitter->hide();
    ui->hide3->show();

}

Serveur *ChatWindowPage::currentTab()
{
	QString tooltip=ui->tab->tabToolTip(ui->tab->currentIndex());
	return serveurs[tooltip];
	//return ui->tab->currentWidget()->findChild<Serveur *>();
}

void ChatWindowPage::closeTab()
{
	QString tooltip=ui->tab->tabToolTip(ui->tab->currentIndex());
	QString txt=ui->tab->tabText(ui->tab->currentIndex());

	if(txt==tooltip)
	{
		QMapIterator<QString, QTextEdit*> i(serveurs[tooltip]->conversations);

		int count=ui->tab->currentIndex()+1;

		while(i.hasNext())
		{
			i.next();
			ui->tab->removeTab(count);
		}

		currentTab()->abort();
		ui->tab->removeTab(ui->tab->currentIndex());
	}
	else
	{

        ui->tab->removeTab(ui->tab->currentIndex());
		currentTab()->conversations.remove(txt);
	}
}

void ChatWindowPage::sendCommande()
{
	QString tooltip=ui->tab->tabToolTip(ui->tab->currentIndex());
	QString txt=ui->tab->tabText(ui->tab->currentIndex());
	if(txt==tooltip)
	{
		currentTab()->sendData(currentTab()->parseCommande(ui->lineEdit->text(),true) );
	}
	else
	{
		currentTab()->sendData(currentTab()->parseCommande(ui->lineEdit->text()) );
	}
	ui->lineEdit->clear();
	ui->lineEdit->setFocus();
}

void ChatWindowPage::tabJoined()
{
	joining=true;
}
void ChatWindowPage::tabJoining()
{
	joining=false;
}

void ChatWindowPage::connecte()
{

    ui->splitter->show();
	Serveur *serveur=new Serveur;
    QTextEdit *textEdit=new QTextEdit;
    ui->hide3->hide();

    ui->tab->addTab(textEdit,"Console/PM");
    ui->tab->setTabToolTip(ui->tab->count()-1,"irc.freenode.net");
    // current tab is now the last, therefore remove all but the last
    for (int i = ui->tab->count(); i > 1; --i) {
       ui->tab->removeTab(0);
    }

    serveurs.insert("irc.freenode.net",serveur);

	serveur->pseudo=ui->editPseudo->text();
    serveur->serveur="irc.freenode.net";
    serveur->port=6667;
	serveur->affichage=textEdit;
    serveur->tab=ui->tab;
	serveur->userList=ui->userView;
	serveur->parent=this;

	textEdit->setReadOnly(true);

	connect(serveur, SIGNAL(joinTab()),this, SLOT(tabJoined() ));
	connect(serveur, SIGNAL(tabJoined()),this, SLOT(tabJoining() ));

    serveur->connectToHost("irc.freenode.net",6667);

	ui->tab->setCurrentIndex(ui->tab->count()-1);
}

void ChatWindowPage::closeEvent(QCloseEvent *event)
{
	(void) event;

	QMapIterator<QString, Serveur*> i(serveurs);

	while(i.hasNext())
	{
		i.next();
		QMapIterator<QString, QTextEdit*> i2(i.value()->conversations);
		while(i2.hasNext())
		{
			i2.next();
            i.value()->sendData("QUIT "+i2.key() + " ");
		}
	}
}
void ChatWindowPage ::setModel(ClientModel *model)
{
    this->model = model;
}

void ChatWindowPage::on_pushButton_WebChat_clicked() {  // #HTHWorld Chat
    
    QDesktopServices::openUrl(QUrl("https://webchat.freenode.net//", QUrl::TolerantMode));
    
}


ChatWindowPage::~ChatWindowPage()
{
    delete ui;
    QMapIterator<QString, Serveur*> i(serveurs);

    while(i.hasNext())
    {
        i.next();
        QMapIterator<QString, QTextEdit*> i2(i.value()->conversations);
        while(i2.hasNext())
        {
            i2.next();
            i.value()->sendData("QUIT "+i2.key() + " ");
        }
    }
}
