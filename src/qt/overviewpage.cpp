// Copyright (c) 2011-2015 The Bitcoin Core developers
// Copyright (c) 2014-2018 The Dash Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "overviewpage.h"
#include "ui_overviewpage.h"

#include "activemasternode.h"
#include "clientversion.h"
#include "netbase.h"
#include "sync.h"
#include "walletmodel.h"
#include "evo/deterministicmns.h"

#include "bitcoinunits.h"
#include "clientmodel.h"
#include "guiconstants.h"
#include "guiutil.h"
#include "init.h"
#include "optionsmodel.h"
#include "platformstyle.h"
#include "transactionfilterproxy.h"
#include "transactiontablemodel.h"
#include "utilitydialog.h"
#include "walletmodel.h"
#include "rpc/blockchain.cpp"
#include "chainparams.h"
#include "amount.h"
#include "validation.h"
#include "wallet/wallet.h"
#include "net.h"

#include "instantx.h"
#include "masternode-sync.h"


#include <univalue.h>

#include <QMessageBox>
#include <QtGui/QClipboard>
#include <QAbstractItemDelegate>
#include <QPainter>
#include <QSettings>
#include <QTimer>
#include <QUrl>
#include <QDesktopServices>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>	
#include <QtNetwork/QNetworkReply>
#include <QProcess>
#include <QDir>
#include <QLabel>
#include <QObject>
#include <QApplication>
#include <QTableWidget>
#include <QTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QElapsedTimer>

#define ICON_OFFSET 16
#define DECORATION_SIZE 54
#define NUM_ITEMS 5
#define NUM_ITEMS_ADV 7



#include "overviewpage.moc"

OverviewPage::OverviewPage(const PlatformStyle *platformStyle, QWidget *parent) :
  
    QWidget(parent),
    timer(nullptr),
    ui(new Ui::OverviewPage),
    clientModel(0),
    walletModel(0),
    currentBalance(-1),
    currentUnconfirmedBalance(-1),
    currentImmatureBalance(-1),
    currentWatchOnlyBalance(-1),
    currentWatchUnconfBalance(-1),
    currentWatchImmatureBalance(-1),
    cachedNumISLocks(-1)
   /* currentReply(0) */
    
{
               
    ui->setupUi(this);
    QString theme = GUIUtil::getThemeName();
	     
        
    // init "out of sync" warning labels
    ui->labelWalletStatus->setText("(" + tr("out of sync") + ")");
	    
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateNewsList()));
    timer->setInterval(10 * 1000); // after 10 seconds on the 1st cycle
    timer->setSingleShot(true);
    timer->start();	    
   	    
    timerinfo_mn = new QTimer(this);
    connect(timerinfo_mn, SIGNAL(timeout()), this, SLOT(updateMasternodeInfo()));
    timerinfo_mn->start(1000);  
      
    timerinfo_blockchain = new QTimer(this);
    connect(timerinfo_blockchain, SIGNAL(timeout()), this, SLOT(updateBlockChainInfo()));
    timerinfo_blockchain->start(1000); //30sec    
      
    timerinfo_peers = new QTimer(this);
    connect(timerinfo_peers, SIGNAL(timeout()), this, SLOT(updatePeersInfo()));
    timerinfo_peers->start(1000);

    
                  
    // start with displaying the "out of sync" warnings
    showOutOfSyncWarning(true);

    // that's it for litemode
    if(fLiteMode) return;
}

void OverviewPage::handleOutOfSyncWarningClicks()
{
    Q_EMIT outOfSyncWarningClicked();
}

OverviewPage::~OverviewPage()
{
    /*if(timer) disconnect(timer, SIGNAL(timeout()), this, SLOT(privateSendStatus()));
    delete ui; */ 
  
  delete ui;
}

void OverviewPage::setBalance(const CAmount& balance, const CAmount& unconfirmedBalance, const CAmount& immatureBalance, const CAmount& anonymizedBalance, const CAmount& watchOnlyBalance, const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance)
{
    currentBalance = balance;
    currentUnconfirmedBalance = unconfirmedBalance;
    currentImmatureBalance = immatureBalance;
    currentAnonymizedBalance = anonymizedBalance;
    currentWatchOnlyBalance = watchOnlyBalance;
    currentWatchUnconfBalance = watchUnconfBalance;
    currentWatchImmatureBalance = watchImmatureBalance;
    ui->labelBalance->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, balance, false, BitcoinUnits::separatorAlways));
    ui->labelImmature->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, immatureBalance, false, BitcoinUnits::separatorAlways));
  /*  ui->labelUnconfirmed->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, anonymizedBalance, false, BitcoinUnits::separatorAlways)); */
    ui->labelTotal->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, balance + unconfirmedBalance + immatureBalance, false, BitcoinUnits::separatorAlways));
    ui->labelWatchAvailable->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, watchOnlyBalance, false, BitcoinUnits::separatorAlways));
  /*  ui->labelWatchPending->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, watchUnconfBalance, false, BitcoinUnits::separatorAlways));   */
    ui->labelWatchImmature->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, watchImmatureBalance, false, BitcoinUnits::separatorAlways));
    ui->labelWatchTotal->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, watchOnlyBalance + watchUnconfBalance + watchImmatureBalance, false, BitcoinUnits::separatorAlways));
    
    // only show immature (newly mined) balance if it's non-zero, so as not to complicate things
    // for the non-mining users
    bool showImmature = immatureBalance != 0;
    bool showWatchOnlyImmature = watchImmatureBalance != 0;

    // for symmetry reasons also show immature label when the watch-only one is shown
    ui->labelImmature->setVisible(showImmature || showWatchOnlyImmature);
    ui->labelImmatureText->setVisible(showImmature || showWatchOnlyImmature);
    ui->labelWatchImmature->setVisible(showWatchOnlyImmature); // show watch-only immature balance
}

// show/hide watch-only labels
void OverviewPage::updateWatchOnlyLabels(bool showWatchOnly)
{
    ui->labelSpendable->setVisible(showWatchOnly);      // show spendable label (only when watch-only is active)
    ui->labelWatchonly->setVisible(showWatchOnly);      // show watch-only label
    ui->lineWatchBalance->setVisible(showWatchOnly);    // show watch-only balance separator line
    ui->labelWatchAvailable->setVisible(showWatchOnly); // show watch-only available balance
  /*  ui->labelWatchPending->setVisible(showWatchOnly); */  // show watch-only pending balance 
    ui->labelWatchTotal->setVisible(showWatchOnly);     // show watch-only total balance

    if (!showWatchOnly){
        ui->labelWatchImmature->hide();
    }
    else{
        ui->labelBalance->setIndent(20);
     /*   ui->labelUnconfirmed->setIndent(20); */
        ui->labelImmatureText->setIndent(20);
        ui->labelTotal->setIndent(20);
    }
}

void OverviewPage::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    if(model)
    {
        // Show warning if this is a prerelease version
       /* connect(model, SIGNAL(alertsChanged(QString)), this, SLOT(updateAlerts(QString)));
         updateAlerts(model->getStatusBarWarnings()); */
    }
}

void OverviewPage::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
    if(model && model->getOptionsModel())
    {
        // update the display unit, to not use the default ("HTH")
        updateDisplayUnit();
        // Keep up to date with wallet
        setBalance(model->getBalance(), model->getUnconfirmedBalance(), model->getImmatureBalance(), model->getAnonymizedBalance(),
                   model->getWatchBalance(), model->getWatchUnconfirmedBalance(), model->getWatchImmatureBalance());
        connect(model, SIGNAL(balanceChanged(CAmount,CAmount,CAmount,CAmount,CAmount,CAmount,CAmount)), this, SLOT(setBalance(CAmount,CAmount,CAmount,CAmount,CAmount,CAmount,CAmount)));

        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));
        updateWatchOnlyLabels(model->haveWatchOnly());
        connect(model, SIGNAL(notifyWatchonlyChanged(bool)), this, SLOT(updateWatchOnlyLabels(bool)));    
    }
}



void OverviewPage::updateDisplayUnit()
{
    if(walletModel && walletModel->getOptionsModel())
    {
        nDisplayUnit = walletModel->getOptionsModel()->getDisplayUnit();
        if(currentBalance != -1)
            setBalance(currentBalance, currentUnconfirmedBalance, currentImmatureBalance, currentAnonymizedBalance,
                       currentWatchOnlyBalance, currentWatchUnconfBalance, currentWatchImmatureBalance);

    }
}

void OverviewPage::showOutOfSyncWarning(bool fShow)
{
    ui->labelWalletStatus->setVisible(fShow);  

 /*   ui->labelTransactionsStatus->setVisible(fShow);  */
}

/**** Blockchain Information *****/



 void OverviewPage::updateMasternodeInfo()
{
    if (!clientModel) {
        return;
    }
    auto mnList = clientModel->getMasternodeList();
    QString strMasternodeCount = tr("%1")
        .arg(QString::number(mnList.GetAllMNsCount()))
     /*   .arg(QString::number(mnList.GetValidMNsCount()))*/;
    ui->countLabelDIP3->setText(strMasternodeCount);
}


 void OverviewPage::updatePeersInfo()  /** Peer Info  **/
{
  if (masternodeSync.IsBlockchainSynced() && masternodeSync.IsSynced())
   
  {
    
          (timerinfo_peers->interval() == 1000);
           timerinfo_peers->setInterval(180000);
           
           int PeerCount = clientModel->getNumConnections();
           ui->label_count_2->setText(QString::number(PeerCount));
  }
}

void OverviewPage::updateBlockChainInfo()
{
    if (masternodeSync.IsBlockchainSynced())
    {
        int CurrentBlock = clientModel->getNumBlocks();
        double CurrentDiff = GetDifficulty();
      
        ui->label_CurrentBlock_value_3->setText(QString::number(CurrentBlock));
        ui->label_Nethash_3->setText(tr("Difficulty:"));
        ui->label_Nethash_value_3->setText(QString::number(CurrentDiff,'f',4));    
    }
};
                /**** End Blockchain Information ******/
