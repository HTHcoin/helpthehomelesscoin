// Copyright (c) 2024 The HTH Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_SMARTCONTRACTSPAGE_H
#define BITCOIN_QT_SMARTCONTRACTSPAGE_H

#include <QWidget>
#include <QMap>
#include <QPair>

class ClientModel;
class WalletModel;
class PlatformStyle;

namespace Ui {
    class SmartContractsPage;
}

/**
 * Smart Contracts page widget - provides UI for creating and interacting with smart contracts
 */
class SmartContractsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SmartContractsPage(const PlatformStyle *platformStyle, QWidget *parent = 0);
    ~SmartContractsPage();

    void setClientModel(ClientModel *clientModel);
    void setWalletModel(WalletModel *walletModel);

private Q_SLOTS:
    void on_createButton_clicked();
    void on_sendToButton_clicked();
    void on_callButton_clicked();
    void on_compileButton_clicked();
    void on_contractSelector_currentIndexChanged(int index);

private:
    Ui::SmartContractsPage *ui;
    ClientModel *clientModel;
    WalletModel *walletModel;
    const PlatformStyle *platformStyle;

    // Store compiled contract data: contract name -> (bytecode, abi)
    QMap<QString, QPair<QString, QString>> compiledContracts;

    void updateAddressBook();
    bool checkSolcInstalled();
};

#endif // BITCOIN_QT_SMARTCONTRACTSPAGE_H
