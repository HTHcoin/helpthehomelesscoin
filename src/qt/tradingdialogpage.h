#ifndef TRADINGDIALOGPAGE_H
#define TRADINGDIALOGPAGE_H

#include <QDialog>
#include <QObject>
#include <stdint.h>
#include "ui_tradingdialogpage.h"

#include <QJsonObject>
#include <QJsonArray>

namespace Ui {
class TradingDialogPage;
}

class TradingDialogPage : public QDialog
{
    Q_OBJECT

public:
    explicit TradingDialogPage(QWidget *parent = 0);
    ~TradingDialogPage();
    
private Q_SLOTS:

    void InitTrading();
    void on_TradingTabWidget_tabBarClicked(int index);
    void ParseAndPopulateOrderBookTables(QString Response);
    void ParseAndPopulateMarketHistoryTable(QString Response);
    void ParseAndPopulateAccountHistoryTable(QString Response);
    void ParseAndPopulateOpenOrdersTable(QString Response);
    void UpdaterFunction();
    void CreateOrderBookTables(QTableWidget& Table,QStringList TableHeader);
    void CalculateBuyCostLabel();
    void CalculateSellCostLabel();
    void DisplayBalance(QLabel &BalanceLabel,QLabel &Available, QLabel &Pending, QString Currency,QString Response);
    void ActionsOnSwitch(int index);
    void CancelOrderSlot(int row, int col);
    void on_UpdateKeys_clicked();
    void on_GenDepositBTN_clicked();
    void on_Buy_Max_Amount_clicked();
    void on_buyOrdertypeCombo_activated(const QString &arg1);
    void on_BuyBidcomboBox_currentIndexChanged(const QString &arg1);
    void on_UnitsInput_textChanged(const QString &arg1);
    void on_BuyBidPriceEdit_textChanged(const QString &arg1);
    void on_BuyMACRON_clicked();
    void on_SellMACRONBTN_clicked();
    void on_SellBidcomboBox_currentIndexChanged(const QString &arg1);
    void on_Sell_Max_Amount_clicked();
    void on_UnitsInputMACRON_textChanged(const QString &arg1);
    void on_SellBidPriceEdit_textChanged(const QString &arg1);
    void on_AdvancedView_stateChanged(int arg1);

    int SetExchangeInfoTextLabels();

    QString BittrexTimeStampToReadable(QString DateTime);
    QString CancelOrder(QString Orderid);
    QString BuyMACRON(QString OrderType, double Quantity, double Rate);
    QString SellMACRON(QString OrderType, double Quantity, double Rate);
    QString GetMarketHistory();
    QString GetMarketSummary();
    QString GetOrderBook();
    QString GetOpenOrders();
    QString GetAccountHistory();
    QString GetBalance(QString Currency);
    QString GetDepositAddress();
    QString HMAC_SHA512_SIGNER(QString UrlToSign,QString Secretkey);
    QString sendRequest(QString url);
    QJsonObject GetResultObjectFromJSONObject(QString response);
    QJsonObject GetResultObjectFromJSONArray(QString response);
    QJsonArray  GetResultArrayFromJSONObject(QString response);

public Q_SLOTS:


private:
    Ui::TradingDialogPage *ui;
    //Socket *socket;
    int timerid;
    QTimer *timer;
    QString ApiKey;
    QString SecretKey;


};

#endif // TRADINGDIALOGPAGE_H
