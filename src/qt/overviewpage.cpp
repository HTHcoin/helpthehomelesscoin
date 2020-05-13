// Copyright (c) 2011-2015 The Bitcoin Core developers
// Copyright (c) 2014-2018 The Dash Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "overviewpage.h"
#include "ui_overviewpage.h"


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
#include "masternodelist.h"


#include <QAbstractItemDelegate>
#include <QPainter>
#include <QSettings>
#include <QTimer>
#include <QUrl>
#include <QDesktopServices>
#include <QtNetwork/QNetworkAccessManager>	
#include <QtNetwork/QNetworkReply>

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
    
{
               
    ui->setupUi(this);
    QString theme = GUIUtil::getThemeName();

     
    ui->pushButton_Website->setStatusTip(tr("Visit Help The Homeless Worldwide A NJ Nonprofit Corporation"));
    ui->pushButton_Website_1->setStatusTip(tr("Visit Help The Homeless Coin"));
    ui->pushButton_Website_2->setStatusTip(tr("Visit AltMarkets.io to trade Help The Homeless Coin"));
    ui->pushButton_Website_3->setStatusTip(tr("Visit Open Chainz to see the Help The Homeless Coin Explorer"));  
    ui->pushButton_Website_4->setStatusTip(tr("Visit Help The Homeless Worldwide A NJ Nonprofit Corporation Partners"));
    ui->pushButton_Website_5->setStatusTip(tr("Visit AltMarkets.io to trade Help The Homeless Coin"));  
        
    // init "out of sync" warning labels
    ui->labelWalletStatus->setText("(" + tr("out of sync") + ")");
  
    //information block update
   
    timerinfo_mn = new QTimer(this);
    connect(timerinfo_mn, SIGNAL(timeout()), this, SLOT(updateMasternodeInfo()));
    timerinfo_mn->start(1000);  
      
    timerinfo_blockchain = new QTimer(this);
    connect(timerinfo_blockchain, SIGNAL(timeout()), this, SLOT(updateBlockChainInfo()));
    timerinfo_blockchain->start(1000); //30sec      
      
                  
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
    ui->labelUnconfirmed->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, unconfirmedBalance, false, BitcoinUnits::separatorAlways));
    ui->labelImmature->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, immatureBalance, false, BitcoinUnits::separatorAlways));
    ui->labelUnconfirmed->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, anonymizedBalance, false, BitcoinUnits::separatorAlways));
    ui->labelTotal->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, balance + unconfirmedBalance + immatureBalance, false, BitcoinUnits::separatorAlways));
    ui->labelWatchAvailable->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, watchOnlyBalance, false, BitcoinUnits::separatorAlways));
    ui->labelWatchPending->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, watchUnconfBalance, false, BitcoinUnits::separatorAlways));
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
    ui->labelWatchPending->setVisible(showWatchOnly);   // show watch-only pending balance
    ui->labelWatchTotal->setVisible(showWatchOnly);     // show watch-only total balance

    if (!showWatchOnly){
        ui->labelWatchImmature->hide();
    }
    else{
        ui->labelBalance->setIndent(20);
        ui->labelUnconfirmed->setIndent(20);
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

/**** Blockchain Information *****/


 void OverviewPage::updateMasternodeInfo()
{
  if (masternodeSync.IsBlockchainSynced() && masternodeSync.IsSynced())
  {
          (timerinfo_mn->interval() == 1000);
           timerinfo_mn->setInterval(180000);
        
           int MNCount = clientModel->getNumConnections();
           ui->label_count_2->setText(QString::number(MNCount));
  }
}

/* txt += tr("<li>Master Nodes <span> %1</span><br> </li>").arg( clientModel->getNumConnections()); */

void OverviewPage::updateBlockChainInfo()
{
    if (masternodeSync.IsBlockchainSynced())
    {
        int CurrentBlock = clientModel->getNumBlocks();
       /* int64_t netHashRate = chainActive.GetNetworkHashPS(24, CurrentBlock-1); */
       /*   double BlockReward = GetBlockHash(CurrentBlock);  */
       /*  double BlockRewardHTH =  static_cast<double>(BlockRewardHTH/COIN); */
        double CurrentDiff = GetDifficulty();

        ui->label_CurrentBlock_value_3->setText(QString::number(CurrentBlock));
        ui->label_Nethash_3->setText(tr("Difficulty:"));
        ui->label_Nethash_value_3->setText(QString::number(CurrentDiff,'f',4));
       /*ui->label_CurrentBlockReward_value_3->setText(QString::number(BlockRewardHTH, 'f', 1)); */
       /* ui->label_CurrentBlock_value_3->setText(QString::number(block24hCount)); */
  
  
    }
}

                /**** End Blockchain Information ******/



void OverviewPage::showOutOfSyncWarning(bool fShow)
{
    ui->labelWalletStatus->setVisible(fShow);  

 /*   ui->labelTransactionsStatus->setVisible(fShow);  */
}

/************** HTH Worldwide Button ******************/
 
void OverviewPage::on_pushButton_Website_clicked() {  // Nonprofit Wesbite
    
    QDesktopServices::openUrl(QUrl("https://www.helpthehomelessworldwide.org/", QUrl::TolerantMode));
    
}

void OverviewPage::on_pushButton_Website_1_clicked() {  // HTH Coin Wesbite
    
    QDesktopServices::openUrl(QUrl("https://hth.world", QUrl::TolerantMode));
    
}


void OverviewPage::on_pushButton_Website_2_clicked() {  // HTH Exchanges
    
    QDesktopServices::openUrl(QUrl("https://altmarkets.io/trading/hthbtc", QUrl::TolerantMode));
    
}

void OverviewPage::on_pushButton_Website_3_clicked() {  // HTH Explorer
    
    QDesktopServices::openUrl(QUrl("https://chainz.cryptoid.info/hth", QUrl::TolerantMode));
    
}

void OverviewPage::on_pushButton_Website_4_clicked() {  // HTH Partners
    
    QDesktopServices::openUrl(QUrl("https://hth.world/partners.html", QUrl::TolerantMode));
    
}


void OverviewPage::on_pushButton_Website_5_clicked() {  // HTH Partners
    
    QDesktopServices::openUrl(QUrl("https://nortexchange.com/exchange/?market=HTH_BTC", QUrl::TolerantMode));
    
}


/************** HTH Worldwide Button ******************/
