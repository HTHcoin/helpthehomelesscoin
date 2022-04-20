// Copyright (c) 2011-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_OVERVIEWPAGE_H
#define BITCOIN_QT_OVERVIEWPAGE_H

#include "platformstyle.h"
#include "primitives/transaction.h"
#include "sync.h"
#include "util.h"

#include "evo/deterministicmns.h"


#include "amount.h"
#include <QTimer>
#include <QMenu>
#include <QWidget>
#include <memory>
#include <QDesktopServices>
#include <QProcess>
#include <QDir>
#include <QLabel>
#include <QtNetwork/QNetworkAccessManager>	
#include <QtNetwork/QNetworkReply>
#include <QUrl>
#include <QBuffer>
#include <QXmlStreamReader>

#define MASTERNODELIST_UPDATE_SECONDS 3
#define MASTERNODELIST_FILTER_COOLDOWN_SECONDS 3

class ClientModel;
class TransactionFilterProxy;
/* class TxViewDelegate; */
class PlatformStyle;
class WalletModel;
class QNetworkAccessManager;
class QNetworkRequest;


namespace Ui {
    class OverviewPage;
}


QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE




/** Overview ("home") page widget */
class OverviewPage : public QWidget
{
    Q_OBJECT

public:
    explicit OverviewPage(const PlatformStyle *platformStyle, QWidget *parent = 0);
    ~OverviewPage();

    void setClientModel(ClientModel *clientModel);
    void setWalletModel(WalletModel *walletModel);
    void showOutOfSyncWarning(bool fShow);
    
    
    
    
    
    
        
    

public Q_SLOTS:
  
    void setBalance(const CAmount& balance, const CAmount& unconfirmedBalance, const CAmount& immatureBalance, const CAmount& anonymizedBalance,
                    const CAmount& watchOnlyBalance, const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance); 
  
    void updateNewsList();
    void newsFinished(QNetworkReply *reply);
    void newsReadyRead();
    void newsMetaDataChanged();
    void newsError(QNetworkReply::NetworkError);

    
  
Q_SIGNALS:
/*    void transactionClicked(const QModelIndex &index); */
    void outOfSyncWarningClicked();
    void doubleClicked(const QModelIndex&);
    
    
   

private:
    
    QTimer *timer;
    QTimer* timerinfo_mn;
    QTimer* timerinfo_blockchain;
    QTimer* timerinfo_peers;
    QNetworkAccessManager* networkManager;
    QNetworkRequest* request;
    Ui::OverviewPage *ui;
    ClientModel *clientModel;
    WalletModel *walletModel;
    CAmount currentBalance;
    CAmount currentUnconfirmedBalance;
    CAmount currentImmatureBalance;
    CAmount currentAnonymizedBalance;
    CAmount currentWatchOnlyBalance;
    CAmount currentWatchUnconfBalance;
    CAmount currentWatchImmatureBalance;
    CAmount GetBlockSubsidy;
    int nDisplayUnit;
  /*  bool fShowAdvancedPSUI; */
    int cachedNumISLocks;
    
    void parseXml();
    void newsGet(const QUrl &url);
    QNetworkAccessManager manager;
    QNetworkReply *currentReply;
    QXmlStreamReader xml;

        
/*    TxViewDelegate *txdelegate;
    std::unique_ptr<TransactionFilterProxy> filter; */
    
 /*   void SetupTransactionList(int nNumItems); */
       


private Q_SLOTS:

    void updateDisplayUnit();
 
  /*  void updateAdvancedPSUI(bool fShowAdvancedPSUI);  */
  /*  void handleTransactionClicked(const QModelIndex &index); */
   /* void updateAlerts(const QString &warnings); */
    void updateWatchOnlyLabels(bool showWatchOnly);
    void handleOutOfSyncWarningClicks();
    void on_pushButton_Website_clicked();
    void on_pushButton_Website_1_clicked();
    void on_pushButton_Website_2_clicked();
    void on_pushButton_Website_3_clicked();
    void on_pushButton_Website_4_clicked();
    void on_pushButton_Website_5_clicked();
    void on_pushButton_Mine_clicked();
    void on_pushButton_Mine_AMD_clicked();
    void updateBlockChainInfo();
    void updateMasternodeInfo(); 
    void updatePeersInfo();
    };

#endif // BITCOIN_QT_OVERVIEWPAGE_H
