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
    cachedNumISLocks(-1),
    fFilterUpdatedDIP3(true),
    nTimeFilterUpdatedDIP3(0),
    nTimeUpdatedDIP3(0),
    mnListChanged(true)
    
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
   
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateDIP3ListScheduled()));
    timer->start(1000);  
      
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
            // Show warning if this is a prerelease version
       /* connect(model, SIGNAL(alertsChanged(QString)), this, SLOT(updateAlerts(QString)));
         updateAlerts(model->getStatusBarWarnings()); */
      
       this->clientModel = model;
    if (model) {
        // try to update list when masternode count changes
        connect(clientModel, SIGNAL(masternodeListChanged()), this, SLOT(handleMasternodeListChanged()));
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


void OverviewPage::on_filterLineEditDIP3_textChanged(const QString& strFilterIn)
{
    strCurrentFilterDIP3 = strFilterIn;
    nTimeFilterUpdatedDIP3 = GetTime();
    fFilterUpdatedDIP3 = true;
    ui->countLabelDIP3->setText(QString::fromStdString(strprintf("Please wait... %d", MASTERNODELIST_FILTER_COOLDOWN_SECONDS)));
}




 void OverviewPage::updateMasternodeInfo()  /** Peer Info for now **/
{
  if (masternodeSync.IsBlockchainSynced() && masternodeSync.IsSynced())
   
  {
    
          (timerinfo_mn->interval() == 1000);
           timerinfo_mn->setInterval(180000);
           
           int PeerCount = clientModel->getNumConnections();
           ui->label_count_2->setText(QString::number(PeerCount));
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





/****** Masternode Count Information Area *******/


#include "masternodelist.h"
#include "ui_masternodelist.h"

#include "activemasternode.h"
#include "clientmodel.h"
#include "clientversion.h"
#include "guiutil.h"
#include "init.h"
#include "masternode-sync.h"
#include "netbase.h"
#include "sync.h"
#include "wallet/wallet.h"
#include "walletmodel.h"

#include <univalue.h>

#include <QMessageBox>
#include <QTimer>
#include <QtGui/QClipboard>

int GetOffsetFromUtc()
{
#if QT_VERSION < 0x050200
    const QDateTime dateTime1 = QDateTime::currentDateTime();
    const QDateTime dateTime2 = QDateTime(dateTime1.date(), dateTime1.time(), Qt::UTC);
    return dateTime1.secsTo(dateTime2);
#else
    return QDateTime::currentDateTime().offsetFromUtc();
#endif
}

    int columnAddressWidth = 200;
    int columnStatusWidth = 80;
    int columnPoSeScoreWidth = 80;
    int columnRegisteredWidth = 80;
    int columnLastPaidWidth = 80;
    int columnNextPaymentWidth = 100;
    int columnPayeeWidth = 130;
    int columnOperatorRewardWidth = 130;

 /*   ui->tableWidgetMasternodesDIP3->setColumnWidth(0, columnAddressWidth);
    ui->tableWidgetMasternodesDIP3->setColumnWidth(1, columnStatusWidth);
    ui->tableWidgetMasternodesDIP3->setColumnWidth(2, columnPoSeScoreWidth);
    ui->tableWidgetMasternodesDIP3->setColumnWidth(3, columnRegisteredWidth);
    ui->tableWidgetMasternodesDIP3->setColumnWidth(4, columnLastPaidWidth);
    ui->tableWidgetMasternodesDIP3->setColumnWidth(5, columnNextPaymentWidth);
    ui->tableWidgetMasternodesDIP3->setColumnWidth(6, columnPayeeWidth);
    ui->tableWidgetMasternodesDIP3->setColumnWidth(7, columnOperatorRewardWidth); */

    // dummy column for proTxHash
    // TODO use a proper table model for the MN list
  /*  ui->tableWidgetMasternodesDIP3->insertColumn(8);
    ui->tableWidgetMasternodesDIP3->setColumnHidden(8, true);

    ui->tableWidgetMasternodesDIP3->setContextMenuPolicy(Qt::CustomContextMenu);

    QAction* copyProTxHashAction = new QAction(tr("Copy ProTx Hash"), this);
    QAction* copyCollateralOutpointAction = new QAction(tr("Copy Collateral Outpoint"), this);
    contextMenuDIP3 = new QMenu();
    contextMenuDIP3->addAction(copyProTxHashAction);
    contextMenuDIP3->addAction(copyCollateralOutpointAction);
    connect(ui->tableWidgetMasternodesDIP3, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenuDIP3(const QPoint&)));
    connect(ui->tableWidgetMasternodesDIP3, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(extraInfoDIP3_clicked()));
    connect(copyProTxHashAction, SIGNAL(triggered()), this, SLOT(copyProTxHash_clicked()));
    connect(copyCollateralOutpointAction, SIGNAL(triggered()), this, SLOT(copyCollateralOutpoint_clicked())); */

  
/*void OverviewPage::showContextMenuDIP3(const QPoint& point)
{
    QTableWidgetItem* item = ui->tableWidgetMasternodesDIP3->itemAt(point);
    if (item) contextMenuDIP3->exec(QCursor::pos());
} */

void OverviewPage::handleMasternodeListChanged()
{
    LOCK(cs_dip3list);
    mnListChanged = true;
}

void OverviewPage::updateDIP3ListScheduled()
{
    TRY_LOCK(cs_dip3list, fLockAcquired);
    if (!fLockAcquired) return;

    if (!clientModel || ShutdownRequested()) {
        return;
    }

    // To prevent high cpu usage update only once in MASTERNODELIST_FILTER_COOLDOWN_SECONDS seconds
    // after filter was last changed unless we want to force the update.
    if (fFilterUpdatedDIP3) {
        int64_t nSecondsToWait = nTimeFilterUpdatedDIP3 - GetTime() + MASTERNODELIST_FILTER_COOLDOWN_SECONDS;
        ui->countLabelDIP3->setText(QString::fromStdString(strprintf("Please wait... %d", nSecondsToWait)));

        if (nSecondsToWait <= 0) {
            updateDIP3List();
            fFilterUpdatedDIP3 = false;
        }
    } else if (mnListChanged) {
        int64_t nSecondsToWait = nTimeUpdatedDIP3 - GetTime() + MASTERNODELIST_UPDATE_SECONDS;

        if (nSecondsToWait <= 0) {
            updateDIP3List();
            mnListChanged = false;
        }
    }
}

void OverviewPage::updateDIP3List()
{
    if (!clientModel || ShutdownRequested()) {
        return;
    }

    LOCK(cs_dip3list);

    QString strToFilter;
    ui->countLabelDIP3->setText("Updating...");
 /*   ui->tableWidgetMasternodesDIP3->setSortingEnabled(false);
    ui->tableWidgetMasternodesDIP3->clearContents();
    ui->tableWidgetMasternodesDIP3->setRowCount(0); */

    auto mnList = clientModel->getMasternodeList();
    nTimeUpdatedDIP3 = GetTime();

    auto projectedPayees = mnList.GetProjectedMNPayees(mnList.GetValidMNsCount());
    std::map<uint256, int> nextPayments;
    for (size_t i = 0; i < projectedPayees.size(); i++) {
        const auto& dmn = projectedPayees[i];
        nextPayments.emplace(dmn->proTxHash, mnList.GetHeight() + (int)i + 1);
    }

 /*   std::set<COutPoint> setOutpts;
    if (walletModel && ui->checkBoxMyMasternodesOnly->isChecked()) {
        std::vector<COutPoint> vOutpts;
        walletModel->listProTxCoins(vOutpts);
        for (const auto& outpt : vOutpts) {
            setOutpts.emplace(outpt);
        }
    } */

    mnList.ForEachMN(false, [&](const CDeterministicMNCPtr& dmn) {
   /*     if (walletModel && ui->checkBoxMyMasternodesOnly->isChecked()) {
            bool fMyMasternode = setOutpts.count(dmn->collateralOutpoint) ||
                walletModel->havePrivKey(dmn->pdmnState->keyIDOwner) ||
                walletModel->havePrivKey(dmn->pdmnState->keyIDVoting) ||
                walletModel->havePrivKey(dmn->pdmnState->scriptPayout) ||
                walletModel->havePrivKey(dmn->pdmnState->scriptOperatorPayout);
            if (!fMyMasternode) return;
        } */
        // populate list
        // Address, Protocol, Status, Active Seconds, Last Seen, Pub Key
        QTableWidgetItem* addressItem = new QTableWidgetItem(QString::fromStdString(dmn->pdmnState->addr.ToString()));
        QTableWidgetItem* statusItem = new QTableWidgetItem(mnList.IsMNValid(dmn) ? tr("ENABLED") : (mnList.IsMNPoSeBanned(dmn) ? tr("POSE_BANNED") : tr("UNKNOWN")));
        QTableWidgetItem* PoSeScoreItem = new QTableWidgetItem(QString::number(dmn->pdmnState->nPoSePenalty));
        QTableWidgetItem* registeredItem = new QTableWidgetItem(QString::number(dmn->pdmnState->nRegisteredHeight));
        QTableWidgetItem* lastPaidItem = new QTableWidgetItem(QString::number(dmn->pdmnState->nLastPaidHeight));
        QTableWidgetItem* nextPaymentItem = new QTableWidgetItem(nextPayments.count(dmn->proTxHash) ? QString::number(nextPayments[dmn->proTxHash]) : tr("UNKNOWN"));

        CTxDestination payeeDest;
        QString payeeStr;
        if (ExtractDestination(dmn->pdmnState->scriptPayout, payeeDest)) {
            payeeStr = QString::fromStdString(CBitcoinAddress(payeeDest).ToString());
        } else {
            payeeStr = tr("UNKNOWN");
        }
        QTableWidgetItem* payeeItem = new QTableWidgetItem(payeeStr);

        QString operatorRewardStr;
        if (dmn->nOperatorReward) {
            operatorRewardStr += QString::number(dmn->nOperatorReward / 100.0, 'f', 2) + "% ";

            if (dmn->pdmnState->scriptOperatorPayout != CScript()) {
                CTxDestination operatorDest;
                if (ExtractDestination(dmn->pdmnState->scriptOperatorPayout, operatorDest)) {
                    operatorRewardStr += tr("to %1").arg(QString::fromStdString(CBitcoinAddress(operatorDest).ToString()));
                } else {
                    operatorRewardStr += tr("to UNKNOWN");
                }
            } else {
                operatorRewardStr += tr("but not claimed");
            }
        } else {
            operatorRewardStr = tr("NONE");
        }
        QTableWidgetItem* operatorRewardItem = new QTableWidgetItem(operatorRewardStr);
        QTableWidgetItem* proTxHashItem = new QTableWidgetItem(QString::fromStdString(dmn->proTxHash.ToString()));

        if (strCurrentFilterDIP3 != "") {
            strToFilter = addressItem->text() + " " +
                          statusItem->text() + " " +
                          PoSeScoreItem->text() + " " +
                          registeredItem->text() + " " +
                          lastPaidItem->text() + " " +
                          nextPaymentItem->text() + " " +
                          payeeItem->text() + " " +
                          operatorRewardItem->text() + " " +
                          proTxHashItem->text();
            if (!strToFilter.contains(strCurrentFilterDIP3)) return;
        }

    /*   ui->tableWidgetMasternodesDIP3->insertRow(0);
        ui->tableWidgetMasternodesDIP3->setItem(0, 0, addressItem);
        ui->tableWidgetMasternodesDIP3->setItem(0, 1, statusItem);
        ui->tableWidgetMasternodesDIP3->setItem(0, 2, PoSeScoreItem);
        ui->tableWidgetMasternodesDIP3->setItem(0, 3, registeredItem);
        ui->tableWidgetMasternodesDIP3->setItem(0, 4, lastPaidItem);
        ui->tableWidgetMasternodesDIP3->setItem(0, 5, nextPaymentItem);
        ui->tableWidgetMasternodesDIP3->setItem(0, 6, payeeItem);
        ui->tableWidgetMasternodesDIP3->setItem(0, 7, operatorRewardItem);
        ui->tableWidgetMasternodesDIP3->setItem(0, 8, proTxHashItem); */
    });

    ui->countLabelDIP3->setText(QString::number(ui->countLabelDIP3/*->rowCount()*/));
 /*   ui->tableWidgetMasternodesDIP3->setSortingEnabled(true);  */
}


/* void OverviewPage::on_checkBoxMyMasternodesOnly_stateChanged(int state)
{
    // no cooldown
    nTimeFilterUpdatedDIP3 = GetTime() - MASTERNODELIST_FILTER_COOLDOWN_SECONDS;
    fFilterUpdatedDIP3 = true;
}

CDeterministicMNCPtr OverviewPage::GetSelectedDIP3MN()
{
    if (!clientModel) {
        return nullptr;
    }

    std::string strProTxHash;
    {
        LOCK(cs_dip3list);

        QItemSelectionModel* selectionModel = ui->tableWidgetMasternodesDIP3->selectionModel();
        QModelIndexList selected = selectionModel->selectedRows();

        if (selected.count() == 0) return nullptr;

        QModelIndex index = selected.at(0);
        int nSelectedRow = index.row();
        strProTxHash = ui->tableWidgetMasternodesDIP3->item(nSelectedRow, 8)->text().toStdString();
    } 

    uint256 proTxHash;
    proTxHash.SetHex(strProTxHash);

    auto mnList = clientModel->getMasternodeList();
    return mnList.GetMN(proTxHash);
}

void OverviewPage::extraInfoDIP3_clicked()
{
    auto dmn = GetSelectedDIP3MN();
    if (!dmn) {
        return;
    }

    UniValue json(UniValue::VOBJ);
    dmn->ToJson(json);

    // Title of popup window
    QString strWindowtitle = tr("Additional information for DIP3 Masternode %1").arg(QString::fromStdString(dmn->proTxHash.ToString()));
    QString strText = QString::fromStdString(json.write(2));

    QMessageBox::information(this, strWindowtitle, strText);
}

void OverviewPage::copyProTxHash_clicked()
{
    auto dmn = GetSelectedDIP3MN();
    if (!dmn) {
        return;
    }

    QApplication::clipboard()->setText(QString::fromStdString(dmn->proTxHash.ToString()));
}

void OverviewPage::copyCollateralOutpoint_clicked()
{
    auto dmn = GetSelectedDIP3MN();
    if (!dmn) {
        return;
    }

    QApplication::clipboard()->setText(QString::fromStdString(dmn->collateralOutpoint.ToStringShort()));
}
 */







