// Copyright (c) 2019 PM-Tech
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_PROPOSALDIALOG_H
#define BITCOIN_QT_PROPOSALDIALOG_H

#include <qt/walletmodel.h>

#include <QAbstractButton>
#include <QDialog>

class ClientModel;
class PlatformStyle;

namespace Ui {
    class ProposalDialog;
}

class ProposalDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProposalDialog(const PlatformStyle *platformStyle, QWidget *parent);
    ~ProposalDialog();

    void setClientModel(ClientModel *_clientModel);
    void setModel(WalletModel *_model);
    void setAddress(const QString &address);

private:
    Ui::ProposalDialog *ui;
    ClientModel *clientModel;
    WalletModel *model;
    const PlatformStyle *platformStyle;
    void processSendCoinsReturn(const WalletModel::SendCoinsReturn &sendCoinsReturn, const QString &msgArg = QString());

private Q_SLOTS:
    void on_addressBookButton_clicked();
    void on_pasteButton_clicked();
    void on_submitButton_clicked();
    void on_clearButton_clicked();
    void on_closeButton_clicked(QAbstractButton* button);
    void on_proposalIn_textChanged();

Q_SIGNALS:
    // Fired when a message should be reported to the user
    void message(const QString &title, const QString &message, unsigned int style);
    void coinsSent(const uint256& txid);
};

#endif // BITCOIN_QT_PROPOSALDIALOG_H
