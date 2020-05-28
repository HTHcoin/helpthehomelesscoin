// Copyright (c) 2017-2018 The Aywa Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_NEWPROPOSALDIALOG_H
#define BITCOIN_QT_NEWPROPOSALDIALOG_H

#include <QWidget>

class WalletModel;
class PlatformStyle;

namespace Ui {
    class NewProposalDialog;
}

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** Dialog showing transaction details. */
class NewProposalDialog : public QWidget
{
    Q_OBJECT

public:
    explicit NewProposalDialog(const std::string idx, QWidget *parent = 0);
    ~NewProposalDialog();

     void setWalletModel(WalletModel *walletModel);
     void voteAction (std::string vote, std::string strProposalHash); 

private:
    WalletModel *walletModel;
    const PlatformStyle *platformStyle;
    std::string GetCurrentPropsalDataStr();

private Q_SLOTS:
    void on_spinboxPeriod_valueChanged(int nPeriod);

    void on_pushbuttonCheck_clicked();

    void on_spinboxAmount_valueChanged(int arg1);

    void on_pushbuttonPrepare_clicked();

    void on_toolbuttonSelectPaymentAddress_clicked();

    void on_toolButton_2_clicked();

    void on_pushbuttonSubmit_clicked();

    void on_dateeditPaymentStartDate_userDateChanged(const QDate &date);

    void on_dateeditPaymentEndDate_userDateChanged(const QDate &date);

    void on_dateeditPaymentEndDate_dateChanged(const QDate &date);

    void on_pushbuttonClose_clicked();

    void on_dateeditPaymentStartDate_dateTimeChanged(const QDateTime &dateTime);

    void on_bnShowHideChannelDetails_clicked();

    void on_toolButton_clicked();

    void on_pushbuttonDetails_clicked();

    void on_bnVoteYes_clicked();
    void on_bnVoteNo_clicked();
    void on_bnVoteAbstain_clicked();
    void on_bnJoinChannel_clicked();
    void on_bnOpenURL_clicked();
    void on_texteditProposalDescription_textChanged();
    void GetNewChannelAddress(std::string & strAddress, std::string & strPubKey, std::string &  strPrivateKey);
    void SetChannelSubscribtion(std::string strAddress, std::string strPubKey,
                         std::string strPrivateKey, std::string strName);
bool GetIsChannelSubscribed (std::string strChannelAddress);
bool GetChannelKeys(std::string strAddress, std::string & strPubKey, std::string & strPrivKey);
std::string GetChannelName(std::string strAddress);



private:
    Ui::NewProposalDialog *ui;
};

#endif // BITCOIN_QT_NEWPROPOSALDIALOG_H
