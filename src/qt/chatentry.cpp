// Copyright (c) 2011-2015 The Bitcoin Core developers
// Copyright (c) 2014-2017 The Dash Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chatentry.h"
#include "ui_chatentry.h"

#include "addressbookpage.h"
#include "addresstablemodel.h"
#include "transactionfilterproxy.h"
#include "transactiontablemodel.h"
#include "guiutil.h"
#include "optionsmodel.h"
#include "platformstyle.h"
#include "walletmodel.h"
#include "base64.h"

#include <QApplication>
#include <QClipboard>

#include <QDialog>
#include <QString>
#include <QStringList>
#include <QFileDialog>

#include <QComboBox>
#include <QDateTimeEdit>
#include <QDesktopServices>
#include <QDoubleValidator>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QAction>
#include <QMenu>
#include <QPoint>
#include <QScrollBar>
#include <QSettings>
#include <QSignalMapper>
#include <QTableView>
#include <QUrl>
#include <QVBoxLayout>

#include <string>

base64 base64chat;
typedef unsigned char BYTE;

bool fileselectedchat=false;

ChatEntry::ChatEntry(const PlatformStyle *platformStyle, QWidget *parent) :
    QStackedWidget(parent),
    ui(new Ui::ChatEntry),
    model(0),
    platformStyle(platformStyle)
{
    ui->setupUi(this);

    setCurrentWidget(ui->Chat);

    if (platformStyle->getUseExtraSpacing())
        ui->chatToLayout->setSpacing(4);


    QString theme = GUIUtil::getThemeName();

    // These icons are needed on Mac also!
    ui->addressBookButton->setIcon(QIcon(":/icons/" + theme + "/address-book"));
    ui->pasteButton->setIcon(QIcon(":/icons/" + theme + "/editpaste"));
    ui->deleteButton->setIcon(QIcon(":/icons/" + theme + "/remove"));
    ui->deleteButton_is->setIcon(QIcon(":/icons/" + theme + "/remove"));
    ui->deleteButton_s->setIcon(QIcon(":/icons/" + theme + "/remove"));
    //ui->pasteButtonBase64->setIcon(QIcon(":/icons/" + theme + "/editpaste"));
    ui->Imgbase64Edit->setMaxLength(10000000);

    //receive address icons

     ui->ReceiveAddressBookButton->setIcon(QIcon(":/icons/" + theme + "/address-book"));
     ui->pasteReceiveButton->setIcon(QIcon(":/icons/" + theme + "/editpaste"));

     //ui->payAmount->setDisabled(true);

     ui->payAmount->setValue(0.0001);


    // ui->payAmount->setVisible(false);

    // normal dash address field
    GUIUtil::setupAddressWidget(ui->chatReceive, this);
    GUIUtil::setupAddressWidget(ui->chatTo, this);
    // just a label for displaying dash address(es)
    ui->payTo_is->setFont(GUIUtil::fixedPitchFont());

    // Connect signals
    //connect(ui->payAmount, SIGNAL(valueChanged()), this, SIGNAL(payAmountChanged()));
    //connect(ui->chatTo, SIGNAL(textChanged()), this, SIGNAL(on_chatTo_textChanged()));
    //connect(ui->checkboxSubtractFeeFromAmount, SIGNAL(toggled(bool)), this, SIGNAL(subtractFeeFromAmountChanged()));
    connect(ui->pasteButton, SIGNAL(clicked()), this, SLOT(on_pasteButton_clicked()));
    connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(deleteClicked()));
    connect(ui->deleteButton_is, SIGNAL(clicked()), this, SLOT(deleteClicked()));
    connect(ui->deleteButton_s, SIGNAL(clicked()), this, SLOT(deleteClicked()));


    connect(ui->ReceiveAddressBookButton, SIGNAL(clicked()), this, SLOT(on_addressReceiveBookButton_clicked()));

    connect(ui->pasteReceiveButton, SIGNAL(clicked()), this, SLOT(on_pasteReceiveAddressButton_clicked()));



    //connect(ui->chooserButton, SIGNAL(clicked()), this, SLOT(on_chooserButton_clicked()));
    //connect(ui->pasteButtonBase64, SIGNAL(clicked()), this, SLOT(on_pasteButtonBase64_clicked()));

}

ChatEntry::~ChatEntry()
{
    delete ui;
}

void ChatEntry::on_pasteButton_clicked()
{
    // Paste text from clipboard into recipient field
    ui->chatTo->setText(QApplication::clipboard()->text());
}


void ChatEntry::on_pasteReceiveAddressButton_clicked()
{
    // Paste text from clipboard into recipient field
    ui->chatReceive->setText(QApplication::clipboard()->text());
}

void ChatEntry::on_pasteButtonBase64_clicked()
{
    // Paste text from clipboard into recipient field
    ui->Imgbase64Edit->setText(QApplication::clipboard()->text());
}

void ChatEntry::on_addressBookButton_clicked()
{
    if(!model)
        return;
    AddressBookPage dlg(platformStyle, AddressBookPage::ForSelection, AddressBookPage::SendingTab, this);
    dlg.setModel(model->getAddressTableModel());
    if(dlg.exec())
    {
        ui->chatTo->setText(dlg.getReturnValue());
        //ui->payAmount->setFocus();
    }
}

void ChatEntry::on_addressReceiveBookButton_clicked()
{
    if(!model)
        return;
    AddressBookPage dlg(platformStyle, AddressBookPage::ForSelection, AddressBookPage::ReceivingTab, this);
    dlg.setModel(model->getAddressTableModel());

    if(dlg.exec())
    {
        ui->chatReceive->setText(dlg.getReturnValue());
        //ui->payAmount->setFocus();
    }
}

void ChatEntry::on_chatTo_textChanged(const QString &address)
{
    updateLabel(address);
    checkaddresstransactions(address);
}

void ChatEntry::on_chatReceive_textChanged(const QString &address)
{
	 checkaddresstransactions(address);
}

void ChatEntry::checkaddresstransactions(const QString &address)
{
	 if (model->validateAddress(ui->chatTo->text()) && model->validateAddress(ui->chatReceive->text()))
	    {

		  ui->chatTo->setDisabled(true);
		  ui->chatReceive->setDisabled(true);
          ui->Imgbase64Edit->setText("Start messaging using to address : " + ui->chatReceive->text() + " and me : " + ui->chatTo->text());
		  transactionProxyModel = new TransactionFilterProxy(this);
		  transactionProxyModel->setAddressPrefix(ui->chatTo->text(),ui->chatReceive->text());
		  transactionProxyModel->setSourceModel(model->getTransactionTableModel());



		   ui->chattableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
		   ui->chattableView->setTabKeyNavigation(false);
		   ui->chattableView->setContextMenuPolicy(Qt::CustomContextMenu);
		   ui->chattableView->installEventFilter(this);

		   QAction *copyImgbase64Action = new QAction(tr("Copy "), this);

		   contextMenu = new QMenu(this);

		   contextMenu->addAction(copyImgbase64Action);

		   connect(copyImgbase64Action, SIGNAL(triggered()), this, SLOT(copyImgbase64()));

		   connect(ui->chattableView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextualMenu(QPoint)));

		   transactionView = ui->chattableView;
		   transactionView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		   transactionView->setModel(transactionProxyModel);
		   transactionView->setAlternatingRowColors(true);
		   transactionView->setSelectionBehavior(QAbstractItemView::SelectRows);
		   transactionView->setSelectionMode(QAbstractItemView::ExtendedSelection);
		   transactionView->setSortingEnabled(true);
		   transactionView->sortByColumn(TransactionTableModel::Date, Qt::AscendingOrder);
		   transactionView->verticalHeader()->hide();
		   //transactionView->horizontalHeader()->hide();
		   transactionView->setColumnWidth(TransactionTableModel::Status, STATUS_COLUMN_WIDTH);
		   //transactionView->setColumnWidth(TransactionTableModel::Watchonly, WATCHONLY_COLUMN_WIDTH);
		   transactionView->setColumnWidth(TransactionTableModel::Date, DATE_COLUMN_WIDTH);
		   transactionView->setColumnWidth(TransactionTableModel::Type, TYPE_COLUMN_WIDTH);
		   transactionView->setColumnWidth(TransactionTableModel::Imgbase64, IMGBASE64_COLUMN_WIDTH);
		   //transactionView->setColumnWidth(TransactionTableModel::Amount, AMOUNT_MINIMUM_COLUMN_WIDTH);
		      // Actions


		   connect(transactionView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(computeSum()));

		   columnResizingFixer = new GUIUtil::TableViewLastColumnResizingFixer(transactionView, IMGBASE64_COLUMN_WIDTH, MINIMUM_COLUMN_WIDTH, this);


		   // show/hide column Watch-only

		          transactionView->setColumnHidden(TransactionTableModel::Watchonly, true); //Watchonly

		          transactionView->setColumnHidden(TransactionTableModel::ToAddress, true); //To address

		          transactionView->setColumnHidden(TransactionTableModel::Amount, true); // Amount

		          // Watch-only signal
		         // connect(model, SIGNAL(notifyWatchonlyChanged(bool)), this, SLOT(updateWatchOnlyColumn(bool)));


	    }
}

void ChatEntry::setModel(WalletModel *model)
{
    this->model = model;

    if (model && model->getOptionsModel())
        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));

    clear();
}

void ChatEntry::clear()
{
    // clear UI elements for normal payment
	ui->chatReceive->clear();
    ui->chatTo->clear();
    //ui->addAsLabel->clear();
    //ui->payAmount->clear();
    ui->Imgbase64Edit->clear();
    ui->Imgbase64Edit->setEnabled(1);
    fileselectedchat=false;
    //ui->checkboxSubtractFeeFromAmount->setCheckState(Qt::Unchecked);
    ui->messageTextLabel->clear();
    ui->messageTextLabel->hide();
    ui->messageLabel->hide();
    // clear UI elements for unauthenticated payment request
    ui->payTo_is->clear();
    ui->memoTextLabel_is->clear();
    //ui->payAmount_is->clear();
    // clear UI elements for authenticated payment request
    ui->payTo_s->clear();
    ui->memoTextLabel_s->clear();
    ui->payAmount_s->clear();

    // update the display unit, to not use the default ("BTC")
    updateDisplayUnit();
}


void ChatEntry::on_chooserButton_clicked()
{
	//clear

	  ui->Imgbase64Edit->setStyleSheet("QLineEdit { background: rgb(255, 255, 255); selection-background-color: rgb(255, 128, 128); }");
	  ui->Imgbase64Edit->setToolTip("Enter base64 string for this tx. ");

    // Paste text from clipboard into recipient field
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);

    //dialog.setViewMode(QFileDialog::List);
    dialog.setOption(QFileDialog::DontUseNativeDialog, false);

    if (dialog.exec()){
    	QStringList fileNames = dialog.selectedFiles();

         if(fileNames.size()>0){



      	  QString file = fileNames[0];
      	  ui->FileNamesTxt->setText(file);
      	  std::string filestr = file.toUtf8().constData();
      	  std::string encodedstring = base64chat.encode(filestr);
      	  QString qsencoded = QString::fromStdString(encodedstring);

        	if(!base64chat.base64Validator(encodedstring)){

        		ui->Imgbase64Edit->setStyleSheet("QLineEdit { background: rgb(220, 20, 60); selection-background-color: rgb(233, 99, 0); }");
        		ui->Imgbase64Edit->setToolTip("Base64 string not valid.");
        		ui->Imgbase64Edit->setText("");
        		 return;
        	}
        	if(qsencoded.size()>10000000)
        	{
        		 ui->Imgbase64Edit->setStyleSheet("QLineEdit { background: rgb(220, 20, 60); selection-background-color: rgb(233, 99, 0); }");
        		 ui->Imgbase64Edit->setToolTip("Large file maxSize 8MB ");
        		 ui->Imgbase64Edit->setText("");
        		 return;
        	}
        	 fileselectedchat=true;
        	 ui->Imgbase64Edit->setText(qsencoded);
        	 ui->Imgbase64Edit->setDisabled(1);
        }
    }
}

void ChatEntry::deleteClicked()
{
    Q_EMIT removeEntry(this);
}

bool ChatEntry::validate()
{
    if (!model)
        return false;


    // Check input validity
    bool retval = true;

    // Skip checks for payment request
    if (recipient.paymentRequest.IsInitialized())
        return retval;

    if (!model->validateAddress(ui->chatTo->text()))
    {
        ui->chatTo->setValid(false);
        retval = false;
    }

    if (!model->validateAddress(ui->chatReceive->text()))
    {
        ui->chatReceive->setValid(false);
        retval = false;
    }


    ui->Imgbase64Edit->setStyleSheet("QLineEdit { background: rgb(255, 255, 255); selection-background-color: rgb(255, 128, 128); }");
    ui->Imgbase64Edit->setToolTip("Enter base64 string for this tx. ");
    if (!ui->Imgbase64Edit->text().isEmpty())
    {

    	std::string imgbase64=ui->Imgbase64Edit->text().toUtf8().constData();


    	if(fileselectedchat)
    	{
     	  if(!base64chat.base64Validator(imgbase64)){

    		ui->Imgbase64Edit->setStyleSheet("QLineEdit { background: rgb(220, 20, 60); selection-background-color: rgb(233, 99, 0); }");
    		ui->Imgbase64Edit->setToolTip("Base64 string not valid.");
    		ui->Imgbase64Edit->setText("");
    	    retval = false;
    	  }
        }


    	if(ui->Imgbase64Edit->text().length()>10000000)
    	{
    		 ui->Imgbase64Edit->setStyleSheet("QLineEdit { background: rgb(220, 20, 60); selection-background-color: rgb(233, 99, 0); }");
    		 ui->Imgbase64Edit->setToolTip("Large file maxSize 8MB ");
    		 ui->Imgbase64Edit->setText("");
    		 retval = false;
    	}
    }
    else {
    	 retval = false;
    }

    if (!ui->payAmount->validate())
    {
        retval = false;
    }

   // Sending a zero amount is invalid
    if (ui->payAmount->value(0) <= 0)
    {
        ui->payAmount->setValid(false);
       retval = false;
    }

    // Reject dust outputs:
   // if (retval && GUIUtil::isDust(ui->chatTo->text(), ui->payAmount->value())) {
       // ui->payAmount->setValid(false);
     //   retval = false;
    //}

    return retval;
}

SendCoinsRecipient ChatEntry::getValue()
{
    // Payment request
    if (recipient.paymentRequest.IsInitialized())
        return recipient;

    // Normal payment
    recipient.address = ui->chatTo->text();
    //recipient.label = ui->addAsLabel->text();
    recipient.imgbase64 = ui->Imgbase64Edit->text();
    if(ui->Imgbase64Edit->text().size()>0 && !fileselectedchat){ //message
    recipient.imgbase64 ="from:" + ui->chatReceive->text() + ":"+ ui->Imgbase64Edit->text();
    }
    recipient.amount = ui->payAmount->value();
    recipient.message = ui->messageTextLabel->text();
    recipient.fSubtractFeeFromAmount = false;//(ui->checkboxSubtractFeeFromAmount->checkState() == Qt::Checked);

    return recipient;
}

QWidget *ChatEntry::setupTabChain(QWidget *prev)
{
    QWidget::setTabOrder(prev, ui->chatTo);
    QWidget::setTabOrder(ui->chatTo, ui->labelchatTo);
    //QWidget *w = ui->payAmount->setupTabChain(ui->addAsLabel);
    //QWidget::setTabOrder(w, ui->checkboxSubtractFeeFromAmount);
    //QWidget::setTabOrder(ui->checkboxSubtractFeeFromAmount, ui->addressBookButton);

    QWidget::setTabOrder(ui->ReceiveAddressBookButton, ui->pasteReceiveButton);
    QWidget::setTabOrder(ui->addressBookButton, ui->pasteButton);
    QWidget::setTabOrder(ui->pasteButton, ui->deleteButton);
    return ui->deleteButton;
}

void ChatEntry::setValue(const SendCoinsRecipient &value)
{
    recipient = value;

    if (recipient.paymentRequest.IsInitialized()) // payment request
    {
        if (recipient.authenticatedMerchant.isEmpty()) // unauthenticated
        {
            ui->chatTo->setText(recipient.address);
            //ui->memoTextLabel_is->setText(recipient.message);
            ui->payAmount_is->setValue(recipient.amount);
            ui->Imgbase64Edit->setText(recipient.imgbase64);
            ui->payAmount->setValue(recipient.amount);
            ui->payAmount_is->setReadOnly(true);
            setCurrentWidget(ui->SendCoins_UnauthenticatedPaymentRequest);
        }
        else // authenticated
        {
            ui->chatTo->setText(recipient.authenticatedMerchant);
            //ui->memoTextLabel_s->setText(recipient.message);

            ui->payAmount_s->setValue(recipient.amount);
            ui->payAmount_s->setReadOnly(true);
            ui->Imgbase64Edit->setText(recipient.imgbase64);
            setCurrentWidget(ui->SendCoins_AuthenticatedPaymentRequest);
        }
    }
    else // normal payment
    {
        // message
        ui->messageTextLabel->setText(recipient.message);
        ui->messageTextLabel->setVisible(!recipient.message.isEmpty());
        ui->messageLabel->setVisible(!recipient.message.isEmpty());

        //ui->addAsLabel->clear();
        ui->chatTo->setText(recipient.address); // this may set a label from addressbook
        if (!recipient.label.isEmpty()) // if a label had been set from the addressbook, don't overwrite with an empty label
            ui->labelchatTo->setText(recipient.label);
        ui->payAmount->setValue(recipient.amount);
        ui->Imgbase64Edit->setText(recipient.imgbase64);
    }
}

void ChatEntry::setAddress(const QString &address, QString imgbase64)
{
    ui->chatTo->setText(address);
    if(!imgbase64.isNull()) ui->Imgbase64Edit->setText(imgbase64);
    //ui->payAmount->setFocus();
}

bool ChatEntry::isClear()
{
    return ui->chatTo->text().isEmpty() && ui->chatReceive->text().isEmpty();
}

void ChatEntry::setFocus()
{
    ui->chatTo->setFocus();
}

void ChatEntry::updateDisplayUnit()
{
    if(model && model->getOptionsModel())
    {
       // Update payAmount with the current unit
        ui->payAmount->setDisplayUnit(model->getOptionsModel()->getDisplayUnit());
        ui->payAmount_is->setDisplayUnit(model->getOptionsModel()->getDisplayUnit());
        ui->payAmount_s->setDisplayUnit(model->getOptionsModel()->getDisplayUnit());
    }
}

bool ChatEntry::updateLabel(const QString &address)
{
    if(!model)
        return false;

    // Fill in label from address book, if address has an associated label
    QString associatedLabel = model->getAddressTableModel()->labelForAddress(address);
    if(!associatedLabel.isEmpty())
    {
        ui->labelchatTo->setText(associatedLabel);
        return true;
    }

    return false;
}

void ChatEntry::contextualMenu(const QPoint &point)
{
    QModelIndex index = transactionView->indexAt(point);
    QModelIndexList selection = transactionView->selectionModel()->selectedRows(0);
    if (selection.empty())
        return;

    // check if transaction can be abandoned, disable context menu action in case it doesn't
    uint256 hash;
    hash.SetHex(selection.at(0).data(TransactionTableModel::TxHashRole).toString().toStdString());
    // abandonAction->setEnabled(model->transactionCanBeAbandoned(hash));

    if(index.isValid())
    {
        contextMenu->exec(QCursor::pos());
    }
}

void ChatEntry::copyImgbase64()
{
    GUIUtil::copyEntryData(transactionView, 0, TransactionTableModel::Imgbase64Role);
}
