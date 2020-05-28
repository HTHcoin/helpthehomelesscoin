// Copyright (c) 2017-2018 The Aywa Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "guiutil.h"
#include "newproposaldialog.h"
#include "governance-validators.h"
#include "utilstrencodings.h"
#include "addressbookpage.h"
#include "masternode-sync.h"
/* #include "masternodeman.h" */
#include "activemasternode.h"
#include "walletmodel.h"
#include "bitcoingui.h"
#include "governance-object.h"
#include "governance.h"
#include "init.h"
#include "transactionrecord.h"
//#ifdef ENABLE_WALLET //TODO: uncomment
#include "wallet/wallet.h"
#include "rpc/server.h"
//#include "univalue.h"
//#endif // ENABLE_WALLET
#include "validation.h"
#include "ui_newproposaldialog.h"

#include "masternode-utils.h"
#include "messagesigner.h"


#include <QModelIndex>
#include <QSettings>
#include <QString>
#include <QMessageBox>
#include <QDesktopServices>
#include <QToolTip>

uint256 txidFee;
std::string strProposalName, strProposalDescriptionHex, strPaymentAddress,
            sProposalChannelAddress, strProposalChannelPubKey, strProposalChannelPrivKey, strProposalStatus,
            strProposalUrl, nPaymentEndDate_str;
int nProposalRevision, nProposalTime, nPaymentStartDate, nPaymentEndDate, nPaymentAmount;
int GetNextSuperblockTime();
CGovernanceObject* pGovObj;

NewProposalDialog::NewProposalDialog(const std::string idx, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NewProposalDialog)
{
    //GUIUtil::restoreWindowGeometry("NewProposalDialogWindow", this->size(), this);//TODO
    ui->setupUi(this);
    /* Open CSS when configured */
    this->setStyleSheet(GUIUtil::loadStyleSheet());

    ui->labelProposalChannelAddress->setVisible(false);
    ui->lineeditPrivateChatAddress->setVisible(false);
    ui->lineeditChannelPrivKey->setVisible(false);
    ui->lineeditPrivateChatPubKey->setVisible(false);
    ui->labelPublicKey->setVisible(false);
    ui->labelPrivKey->setVisible(false);
    ui->pushbuttonReset->setVisible(false);


    if (!idx.empty()) {   //Open exists proposal item
        uint256 hash = uint256S(idx);
        //CGovernanceObject*
        pGovObj = governance.FindGovernanceObject(hash);
        if (!pGovObj) return; //only GOVERNANCE_OBJECT_PROPOSAL can be opened
        UniValue objJSON = pGovObj->GetJSONObject();
        std::string strName = objJSON["name"].get_str();
        std::string strWindowTitle = "HTH - Proposal "+strName;
        int nEnd_epoch = objJSON["end_epoch"].get_int();
        std::string strPayment_address = objJSON["payment_address"].get_str();
        int nPayment_amount = objJSON["payment_amount"].get_int();
        int nStart_epoch = objJSON["start_epoch"].get_int();
        std::string strUrl = objJSON["url"].get_str();
        std::string strProposalChannelAddress, strProposalChannelPrivKey, strProposalChannelPubKey;
        try{//some proposals dont have a description
            strProposalChannelAddress = objJSON["smsg_addr"].get_str();
            strProposalChannelPubKey = objJSON["smsg_pubkey"].get_str();
            strProposalChannelPrivKey = objJSON["smsg_privkey"].get_str();
            std::string strDescription = objJSON["description"].get_str();
            std::vector<unsigned char> v = ParseHex(strDescription);
            std::string strProposalDescription(v.begin(), v.end());
            ui->texteditProposalDescription->setHtml(strProposalDescription.c_str());
        }
        catch(std::exception& e)
        {
            ui->texteditProposalDescription->setText(e.what());
        };

        ui->lineeditProposalName->setText(strName.c_str());
        ui->lineeditProposalName->setReadOnly(true);

        ui->texteditProposalDescription->setReadOnly(true);
        ui->lineeditPaymentAddress->setText(strPayment_address.c_str());
        ui->lineeditPaymentAddress->setReadOnly(true);
        ui->dateeditPaymentStartDate->setDateTime(QDateTime::fromTime_t(nStart_epoch+43200));// + Params().GetConsensus().nBudgetPaymentsCycleBlocks * Params().GetConsensus().nPowTargetSpacing / 2));
        ui->dateeditPaymentStartDate->setEnabled(false);
        ui->dateeditPaymentEndDate->setDateTime(QDateTime::fromTime_t(nEnd_epoch-43200));// - Params().GetConsensus().nBudgetPaymentsCycleBlocks * Params().GetConsensus().nPowTargetSpacing / 2));
        ui->dateeditPaymentEndDate->setEnabled(false);
        ui->spinboxAmount->setValue(nPayment_amount);
        ui->spinboxAmount->setEnabled(false);
        const Consensus::Params& consensusParams = Params().GetConsensus();
        const int nBudgetPaymentsCycleBlocks = consensusParams.nBudgetPaymentsCycleBlocks;
        ui->spinboxPeriod->setValue((nEnd_epoch - nStart_epoch)/86400);

        ui->spinboxPeriod->setEnabled(false);

        ui->lineeditProposalUrl->setText(strUrl.c_str());
        ui->lineeditProposalUrl->setReadOnly(true);

        ui->lineeditPrivateChatAddress->setText(strProposalChannelAddress.c_str());
        ui->lineeditPrivateChatPubKey->setText(strProposalChannelPubKey.c_str());
        ui->lineeditChannelPrivKey->setText(strProposalChannelPrivKey.c_str());

        ui->bnJoinChannel->setEnabled(!GetIsChannelSubscribed(ui->lineeditPrivateChatAddress->text().toStdString()));

        ui->lineeditPrivateChatAddress->setReadOnly(true);
        ui->pushbuttonCheck->setEnabled(false);
        ui->toolbuttonSelectPaymentAddress->setEnabled(false);
        this->setWindowTitle(strWindowTitle.c_str());
//        connect(MasternodeList.voteAction();, )
    }
    else //new item
    {
        ui->dateeditPaymentStartDate->setMaximumDate(QDate::currentDate().addDays(30));//no more, than 1 month planning future

        int nNextSuperBlockTime = GetNextSuperblockTime();
        int nProposalRecommendedTime = nNextSuperBlockTime + Params().GetConsensus().nBudgetPaymentsCycleBlocks * Params().GetConsensus().nPowTargetSpacing;
        int nProposalMinimumTime = nNextSuperBlockTime;


        ui->dateeditPaymentStartDate->setDateTime(QDateTime::fromTime_t(nProposalRecommendedTime));
        ui->dateeditPaymentStartDate->setMinimumDateTime(QDateTime::fromTime_t(nProposalMinimumTime));

        ui->dateeditPaymentStartDate->setMaximumTime(ui->dateeditPaymentStartDate->time());
        int nPeriod = 0;
        ui->dateeditPaymentEndDate->setDateTime(QDateTime::fromTime_t(ui->dateeditPaymentStartDate->dateTime().toTime_t() + nPeriod*Params().GetConsensus().nPowTargetSpacing*Params().GetConsensus().nBudgetPaymentsCycleBlocks));
        std::string strAddress, strPubKey, strPrivateKey;
        GetNewChannelAddress(strAddress, strPubKey, strPrivateKey);
        ui->lineeditPrivateChatAddress->setText(strAddress.c_str());
        ui->lineeditPrivateChatPubKey->setText(strPubKey.c_str());
        ui->lineeditChannelPrivKey->setText(strPrivateKey.c_str());
        ui->bnJoinChannel->setEnabled(false);
        ui->bnVoteYes->setVisible(false);
        ui->bnVoteNo->setVisible(false);
        ui->bnVoteAbstain->setVisible(false);
    }
    ui->labelTotalProposalBudget->setText(QString::number(ui->spinboxPeriod->value()*ui->spinboxAmount->value()));
}

//Return random string data for channel subsciption. Not wallet data.
void GetNewChannelAddress(std::string & strAddress, std::string & strPubKey, std::string & strPrivateKey)
{
    CKey secret;
    CPubKey pubKey;
    secret.MakeNewKey(true);
    pubKey = secret.GetPubKey();
    assert(secret.VerifyPubKey(pubKey));
    CKeyID keyID = pubKey.GetID();
    strAddress = CBitcoinAddress(keyID).ToString();
    strPubKey = EncodeBase58(pubKey.begin(), pubKey.end());
    strPrivateKey = CBitcoinSecret(secret).ToString();
}

NewProposalDialog::~NewProposalDialog()
{
    GUIUtil::saveWindowGeometry("NewProposalDialogWindow", this);
    delete ui;
}



void NewProposalDialog::on_spinboxPeriod_valueChanged(int nPeriod)
{
    ui->labelTotalProposalBudget->setText(QString::number(ui->spinboxPeriod->value()*ui->spinboxAmount->value()));
    ui->dateeditPaymentEndDate->setDateTime(QDateTime::fromTime_t(ui->dateeditPaymentStartDate->dateTime().toTime_t() + (nPeriod-1) * Params().GetConsensus().nPowTargetSpacing*Params().GetConsensus().nBudgetPaymentsCycleBlocks));
    //ui->dateeditPaymentStartDate->setDisplayFormat("dd.MM.yyyy hh:mm");
}

void NewProposalDialog::on_pushbuttonCheck_clicked()
{
    uint256 hashParent;
    int nRevision = 1;
    strProposalName = ui->lineeditProposalName->text().toStdString();
    strProposalDescriptionHex = HexStr(ui->texteditProposalDescription->toHtml().toStdString());//TODO: toHtml/toPlainText
    std::vector<unsigned char> v = ParseHex(strProposalDescriptionHex);
    std::string strProposalDescription(v.begin(), v.end());
    ui->texteditProposalDescription->setHtml(strProposalDescription.c_str());
    strPaymentAddress = ui->lineeditPaymentAddress->text().toStdString();
    sProposalChannelAddress = ui->lineeditPrivateChatAddress->text().toStdString();
    strProposalChannelPubKey = ui->lineeditPrivateChatPubKey->text().toStdString();
    strProposalUrl = ui->lineeditProposalUrl->text().toStdString();
    //strProposalStatus = "NEW";
    nPaymentStartDate = ui->dateeditPaymentStartDate->dateTime().toTime_t() - Params().GetConsensus().nBudgetPaymentsCycleBlocks*Params().GetConsensus().nPowTargetSpacing/2;//Adding payment window for superblock-half payment/superblock cycle size.
    nPaymentEndDate = ui->dateeditPaymentEndDate->dateTime().toTime_t() + Params().GetConsensus().nBudgetPaymentsCycleBlocks*Params().GetConsensus().nPowTargetSpacing/2;
    nPaymentAmount = ui->spinboxAmount->value();
    //nPaymentEndDate_str = boost::lexical_cast<std::string>(nPaymentEndDate);
    UniValue uvProposalData (UniValue::VARR);
    uvProposalData.push_back("proposal");
    UniValue uvStrData (UniValue::VOBJ);
    uvStrData.pushKV("end_epoch",nPaymentEndDate);
    uvStrData.pushKV("name",strProposalName);
    uvStrData.pushKV("payment_address",strPaymentAddress);
    uvStrData.pushKV("payment_amount",nPaymentAmount);
    uvStrData.pushKV("start_epoch",nPaymentStartDate);
    uvStrData.pushKV("type",1);
    uvStrData.pushKV("url",strProposalUrl);
    uvStrData.pushKV("description",strProposalDescriptionHex);
    uvStrData.pushKV("smsg_addr",sProposalChannelAddress);
    uvStrData.pushKV("smsg_pubkey",strProposalChannelPubKey);
    uvStrData.pushKV("smsg_pubkey","PUBKEY");
    uvStrData.pushKV("smsg_privkey","PRIVKEY");
    uvProposalData.push_back(uvStrData);

     UniValue uvProposal (UniValue::VARR);
     uvProposal.push_back(uvProposalData);

    std::string strDataAll = uvProposal.write();
    std::string strErrorMessages;
    std::string strData = HexStr (strDataAll);
    //ui->texteditProposalDescription->setText(QString::fromStdString(strData));
    //ui->texteditHEX->setText(QString::fromStdString(strData));
    int64_t nTime = GetAdjustedTime();
    strData = GetCurrentPropsalDataStr();
    CGovernanceObject govobj(hashParent, nRevision, nTime, uint256(), strData);
    if(govobj.GetObjectType() == GOVERNANCE_OBJECT_PROPOSAL) {
        CProposalValidator validator(strData);
        if(!validator.Validate())  {
            std::string strErrorMessages = validator.GetErrorMessages();
            QMessageBox msgBox;
            strErrorMessages = "Invalid proposal data, error messages: "+strErrorMessages;
            msgBox.setText("Proposal validation error!");
            msgBox.setInformativeText(strErrorMessages.c_str());
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            return;
            }
    }
        else{
        strErrorMessages = "Invalid object type, only proposals can be validated";
        QMessageBox msgBox;
        msgBox.setText("Proposal validation error!");
        msgBox.setInformativeText(strErrorMessages.c_str());
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        return;
        }
   //objResult.push_back(Pair("Object status", "OK"));
    ui->labelProposalStatus->setText(QString::fromStdString("CHECKED"));
    ui->pushbuttonCheck->setEnabled(false);
    ui->pushbuttonPrepare->setEnabled(true);
    ui->pushbuttonReset->setEnabled(true);

    ui->lineeditProposalName->setReadOnly(true);
    ui->texteditProposalDescription->setReadOnly(true);
    ui->lineeditPaymentAddress->setReadOnly(true);
    ui->dateeditPaymentStartDate->setEnabled(false);
    ui->dateeditPaymentEndDate->setEnabled(false);
    ui->spinboxAmount->setEnabled(false);
    ui->spinboxPeriod->setEnabled(false);

    ui->lineeditProposalUrl->setReadOnly(true);


    QMessageBox msgBox;
    msgBox.setText("Proposal validation passed.");
    msgBox.setInformativeText("Proposal is valid. "
                              "Press 'Prepare' button to create 1 AYWA proposal collateral tx.");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.exec();

}

void NewProposalDialog::on_spinboxAmount_valueChanged(int arg1)
{
    ui->labelTotalProposalBudget->setText(QString::number(ui->spinboxPeriod->value()*ui->spinboxAmount->value()));

}

void NewProposalDialog::on_pushbuttonPrepare_clicked()
{
    uint256 hashParent; hashParent = uint256();//TODO -
    std::string strRevision = "1";//TODO Check it later
    int nRevision = boost::lexical_cast<int>(strRevision);
    int nTime = GetAdjustedTime();
    nProposalTime = nTime;
    std::string strData = GetCurrentPropsalDataStr();

    // CREATE A NEW COLLATERAL TRANSACTION FOR THIS SPECIFIC OBJECT

    CGovernanceObject govobj(hashParent, nRevision, nTime, uint256(), strData);
    //TODO:Logging object creation details.
    if(govobj.GetObjectType() == GOVERNANCE_OBJECT_PROPOSAL) {
        CProposalValidator validator(strData);
        if(!validator.Validate())  {
            std::string strErrorMessages = "Invalid proposal data, error messages: "+validator.GetErrorMessages();
            QMessageBox msgBox;
            msgBox.setText("Proposal prepare error!");
            msgBox.setInformativeText(strErrorMessages.c_str());
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            return;
        }
    }
    if((govobj.GetObjectType() == GOVERNANCE_OBJECT_TRIGGER) ||
       (govobj.GetObjectType() == GOVERNANCE_OBJECT_WATCHDOG)) {
        std::string strErrorMessages = "Trigger and watchdog objects need not be prepared (however only masternodes can create them).";
        QMessageBox msgBox;
        msgBox.setText("Proposal preapare error!");
        msgBox.setInformativeText(strErrorMessages.c_str());
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        return;
    }
    {
        LOCK(cs_main);
        std::string strError = "";
        if(!govobj.IsValidLocally(strError, false)){
            std::string strErrorMessages = "Governance object is not valid - " + govobj.GetHash().ToString() + " - " + strError;
            QMessageBox msgBox;
            msgBox.setText("Proposal prepare error!");
            msgBox.setInformativeText(strErrorMessages.c_str());
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            return;
        }
    }

    // request unlock only if was locked or unlocked for mixing:
    // this way we let users unlock by walletpassphrase or by menu
    // and make many transactions while unlocking through this dialog
    // will call relock
    if(!walletModel || !walletModel->getOptionsModel())
        return;
    WalletModel::EncryptionStatus encStatus = walletModel->getEncryptionStatus();
    if(encStatus == walletModel->Locked || encStatus == walletModel->UnlockedForMixingOnly)
    {
        WalletModel::UnlockContext ctx(walletModel->requestUnlock());
        if(!ctx.isValid())
        {
            // Unlock wallet was cancelled
            //fNewRecipientAllowed = true;
            return;
        }
        //TODO - code duplicate. remove it
        CWalletTx wtx;
        if(!pwalletMain->GetBudgetSystemCollateralTX(wtx, govobj.GetHash(), govobj.GetMinCollateralFee(), false)) {
            std::string strErrorMessages = "Error making collateral transaction for governance object. Please check your wallet balance.";
            QMessageBox msgBox;
            msgBox.setText("Proposal validation error!");
            msgBox.setInformativeText(strErrorMessages.c_str());
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            return;
        }

        // -- make our change address
        CReserveKey reservekey(pwalletMain);
        // Display message box
        QMessageBox::StandardButton retval = QMessageBox::question(this,
                                                                   tr("Confirm the creation if a collateral tx for the proposal. \n"),
                                                                   tr("Are you sure you want to spend 1 AYWA? \n"
                                                                      "This operation can not be cancelled."),
                                                                   QMessageBox::Yes | QMessageBox::Cancel,
                                                                   QMessageBox::Cancel);

        if(retval != QMessageBox::Yes) return;
        // -- send the tx to the network
        pwalletMain->CommitTransaction(wtx, reservekey, g_connman.get(), NetMsgType::TX);
        txidFee = wtx.GetHash();
        ui->pushbuttonPrepare->setEnabled(false);
        ui->pushbuttonSubmit->setEnabled(true);
        ui->lineeditProposalName->setEnabled(false);
        ui->texteditProposalDescription->setReadOnly(true);
        ui->labelProposalStatus->setText(QString::fromStdString("PREPARED"));
        return;
    }
    CWalletTx wtx;
    if(!pwalletMain->GetBudgetSystemCollateralTX(wtx, govobj.GetHash(), govobj.GetMinCollateralFee(), false)) {
        std::string strErrorMessages = "Error making collateral transaction for governance object. Please check your wallet balance and make sure your wallet is unlocked.";
        QMessageBox msgBox;
        msgBox.setText("Proposal validation error!");
        msgBox.setInformativeText(strErrorMessages.c_str());
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        return;
    }

    // -- make our change address
    CReserveKey reservekey(pwalletMain);


    QMessageBox::StandardButton retval = QMessageBox::question(this,
                                                               tr("Confirm the creation of the collateral tx for the proposal"),
                                                               tr("Are you sure you want to spend 1 AYWA to prepare proposal? \n"
                                                                  "This operation can not be cancelled."),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);

    if(retval != QMessageBox::Yes) return;
    // -- send the tx to the network
    pwalletMain->CommitTransaction(wtx, reservekey, g_connman.get(), NetMsgType::TX);
    txidFee = wtx.GetHash();
    ui->pushbuttonPrepare->setEnabled(false);
    ui->pushbuttonSubmit->setEnabled(true);
    ui->lineeditProposalName->setEnabled(false);
    ui->texteditProposalDescription->setReadOnly(true);
    ui->labelProposalStatus->setText(QString::fromStdString("PREPARED"));



    ui->lineeditProposalName->setReadOnly(true);
    ui->texteditProposalDescription->setReadOnly(true);
    ui->lineeditPaymentAddress->setReadOnly(true);
    ui->dateeditPaymentStartDate->setEnabled(false);
    ui->dateeditPaymentEndDate->setEnabled(false);
    ui->spinboxAmount->setEnabled(false);
    ui->spinboxPeriod->setEnabled(false);

    ui->lineeditProposalUrl->setReadOnly(true);


    //TODO: disable close button or add reminder.
    std::string strMessages = "Please wait about 1-5 min until transaction confirmed.\n"
                              "Do not close this form, before submit the proposal.\n";
    QMessageBox msgBox;
    msgBox.setText("The proposal collateral tx is sucsessfully sent to blockchain.");
    msgBox.setInformativeText(strMessages.c_str());
    msgBox.setIcon(QMessageBox::Information);
    msgBox.exec();

}

void NewProposalDialog::on_toolbuttonSelectPaymentAddress_clicked()
{
    if (ui->lineeditProposalName->text().isEmpty())
    {
        //        QMessageBox msgBox;
        //        msgBox.setText("The Proposal Channel is successfully subscribed");
        //        msgBox.setIcon(QMessageBox::Information);
        //        msgBox.exec();
        ui->lineeditProposalName->setFocus();
        QToolTip::showText(ui->lineeditProposalName->mapToGlobal(QPoint()), tr("Type Proposal Name first"));
        return;//TODO insert dlg here
    }
    //TODO: Check if lineeditProposalName has correct text. Check if wallet address label already present.
    UniValue label = ui->lineeditProposalName->text().toStdString();
    UniValue params (UniValue::VARR);
    params.push_back(label);
    UniValue newAddress = getnewaddress(params, false);
    try
    {
        ui->lineeditPaymentAddress->setText(newAddress.get_str().c_str());

    }
    catch(...){
    }
    ui->toolbuttonSelectPaymentAddress->setEnabled(false);
}

void NewProposalDialog::on_toolButton_2_clicked()
{
    if(!walletModel)
        return;
    AddressBookPage dlg(AddressBookPage::ForSelection, AddressBookPage::ReceivingTab, this);
    dlg.setModel(walletModel->getAddressTableModel());

    if(dlg.exec())
    {
        ui->lineeditPaymentAddress->setText(dlg.getReturnValue());

    }
}

void NewProposalDialog::on_pushbuttonSubmit_clicked()
{
    if(!masternodeSync.IsBlockchainSynced()) {
        std::string strErrorMessages = "Must wait for client to sync with masternode network. Try again in a minute or so.";
        QMessageBox msgBox;
        msgBox.setText("Proposal subit error!");
        msgBox.setInformativeText(strErrorMessages.c_str());
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();

    }

    //bool fMnFound = mnodeman.Has(activeMasternode.outpoint);
    uint256 hashParent; hashParent = uint256();//TODO -
    std::string strRevision = "1";
    int nRevision = boost::lexical_cast<int>(strRevision);
    int nTime = nProposalTime;
    std::string strData = GetCurrentPropsalDataStr();
    CGovernanceObject govobj(hashParent, nRevision, nTime, txidFee, strData);//TODO CHECK Hash Parent
    if(govobj.GetObjectType() == GOVERNANCE_OBJECT_PROPOSAL) {
        CProposalValidator validator(strData);
        if(!validator.Validate())  {
            std::string strErrorMessages = "Invalid proposal data, error messages:" + validator.GetErrorMessages();
            QMessageBox msgBox;
            msgBox.setText("Proposal subit error!");
            msgBox.setInformativeText(strErrorMessages.c_str());
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            return;
        }
    }
    std::string strHash = govobj.GetHash().ToString();
    //TODO: Logging result strHash etc.
    LogPrintf("gobject(submit) -- Proposal submittred. Hash:%s", strHash);


    std::string strError = "";
    bool fMissingMasternode = false;
    bool fMissingConfirmations = false;
    {
        LOCK(cs_main);
        if(!govobj.IsValidLocally(strError, fMissingMasternode, fMissingConfirmations, true) && !fMissingConfirmations) {
            LogPrintf("gobject(submit) -- Object submission rejected because object is not valid - hash = %s, strError = %s\n", strHash, strError);
            std::string strErrorMessages = "Governance object is not valid - " + strHash + " - " + strError;
            QMessageBox msgBox;
            msgBox.setText("Proposal subit error!");
            msgBox.setInformativeText(strErrorMessages.c_str());
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            return;
        }
    }
    // RELAY THIS OBJECT
    // Reject if rate check fails but don't update buffer
    if(!governance.MasternodeRateCheck(govobj)) {
        LogPrintf("gobject(submit) -- Object submission rejected because of rate check failure - hash = %s\n", strHash);
        //return;
    }

    LogPrintf("gobject(submit) -- Adding locally created governance object - %s\n", strHash);

    if(fMissingConfirmations) {
        governance.AddPostponedObject(govobj);
        govobj.Relay(*g_connman);
    } else {
        governance.AddGovernanceObject(govobj, *g_connman);
    }
    SetChannelSubscribtion(ui->lineeditPrivateChatAddress->text().toStdString(),
                        ui->lineeditPrivateChatPubKey->text().toStdString(),
                        ui->lineeditChannelPrivKey->text().toStdString(),
                        std::string("Proposal ") + ui->lineeditProposalName->text().toStdString());

    strHash = govobj.GetHash().ToString();
    //TODO: oLgging it
    std::string strErrorMessages = "Proposal hash - " + strHash + "\n"+
            "You can view approval status if type following command in the Console: \n"
            "gobject getvotes "+ strHash +"\n";
    QMessageBox msgBox;
    msgBox.setText("The proposal sucsessfully submited!");
    msgBox.setInformativeText(strErrorMessages.c_str());
    msgBox.setIcon(QMessageBox::Information);
    msgBox.exec();
    ui->pushbuttonSubmit->setEnabled(false);
    ui->labelProposalStatus->setText(QString::fromStdString("SUBMITED"));

    ui->lineeditProposalName->setReadOnly(true);
    ui->texteditProposalDescription->setReadOnly(true);
    ui->lineeditPaymentAddress->setReadOnly(true);
    ui->dateeditPaymentStartDate->setEnabled(false);
    ui->dateeditPaymentEndDate->setEnabled(false);
    ui->spinboxAmount->setEnabled(false);
    ui->spinboxPeriod->setEnabled(false);

    ui->lineeditProposalUrl->setReadOnly(true);

}

void NewProposalDialog::on_dateeditPaymentStartDate_userDateChanged(const QDate &date)
{
    ui->dateeditPaymentEndDate->setDate(date.addDays(ui->spinboxPeriod->value()-1));
}

void NewProposalDialog::on_dateeditPaymentEndDate_userDateChanged(const QDate &date)
{
    //ui->spinboxPeriod->setValue(date-(ui->dateeditPaymentStartDate->day()));
    //TODO Later

}

void NewProposalDialog::on_dateeditPaymentEndDate_dateChanged(const QDate &date)
{

}

void NewProposalDialog::on_pushbuttonClose_clicked()
{
//    if (ui->pushbuttonCheck->isEnabled())

    this->close();
}

void NewProposalDialog::on_dateeditPaymentStartDate_dateTimeChanged(const QDateTime &dateTime)
{
    const Consensus::Params& consensusParams = Params().GetConsensus();
    ui->dateeditPaymentEndDate->setDateTime(QDateTime::fromTime_t(dateTime.toTime_t()+(ui->spinboxPeriod->value()-1)*consensusParams.nPowTargetSpacing*consensusParams.nBudgetPaymentsCycleBlocks));

}

void NewProposalDialog::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
}

void NewProposalDialog::on_bnShowHideChannelDetails_clicked()
{
    ui->labelProposalChannelAddress->setVisible(!ui->labelProposalChannelAddress->isVisible());
    ui->lineeditPrivateChatAddress->setVisible(!ui->lineeditPrivateChatAddress->isVisible());
    ui->lineeditChannelPrivKey->setVisible(!ui->lineeditChannelPrivKey->isVisible());
    ui->lineeditPrivateChatPubKey->setVisible(!ui->lineeditPrivateChatPubKey->isVisible());
    ui->labelPublicKey->setVisible(!ui->labelPublicKey->isVisible());
    ui->labelPrivKey->setVisible(!ui->labelPrivKey->isVisible());
}

string NewProposalDialog::GetCurrentPropsalDataStr()
{
    std::string strProposalName = ui->lineeditProposalName->text().toStdString();
    std::string strProposalDescriptionHex = HexStr(ui->texteditProposalDescription->toHtml().toStdString());//TODO: toHtml/toPlainText
    std::string strPaymentAddress = ui->lineeditPaymentAddress->text().toStdString();
    std::string strPrivateChatAddress = ui->lineeditPrivateChatAddress->text().toStdString();
    std::string strPrivatChatPubKey = ui->lineeditPrivateChatPubKey->text().toStdString();
    std::string strChannelPrivKey = ui->lineeditChannelPrivKey->text().toStdString();
    std::string strProposalUrl = ui->lineeditProposalUrl->text().toStdString();
    //int nPaymentStartDate = ui->dateeditPaymentStartDate->dateTime().toTime_t();
    //int nPaymentEndDate = ui->dateeditPaymentEndDate->dateTime().toTime_t();
    int nPaymentStartDate = ui->dateeditPaymentStartDate->dateTime().toTime_t() - Params().GetConsensus().nBudgetPaymentsCycleBlocks*Params().GetConsensus().nPowTargetSpacing/2;//Adding payment window for superblock-half payment/superblock cycle size.
    int nPaymentEndDate = ui->dateeditPaymentEndDate->dateTime().toTime_t() + Params().GetConsensus().nBudgetPaymentsCycleBlocks*Params().GetConsensus().nPowTargetSpacing/2;

    int nPaymentAmount = ui->spinboxAmount->value();
    //nPaymentEndDate_str = boost::lexical_cast<std::string>(nPaymentEndDate);
    UniValue uvProposalData (UniValue::VARR);
    uvProposalData.push_back("proposal");
    UniValue uvStrData (UniValue::VOBJ);
    uvStrData.pushKV("end_epoch",nPaymentEndDate);
    uvStrData.pushKV("name",strProposalName);
    uvStrData.pushKV("payment_address",strPaymentAddress);
    uvStrData.pushKV("payment_amount",nPaymentAmount);
    uvStrData.pushKV("start_epoch",nPaymentStartDate);
    uvStrData.pushKV("type",1);
    uvStrData.pushKV("url",strProposalUrl);
    uvStrData.pushKV("description",strProposalDescriptionHex);
    uvStrData.pushKV("smsg_addr",strPrivateChatAddress);
    uvStrData.pushKV("smsg_pubkey",strPrivatChatPubKey);
    uvStrData.pushKV("smsg_privkey",strChannelPrivKey);
    uvProposalData.push_back(uvStrData);
    UniValue uvProposal (UniValue::VARR);
    uvProposal.push_back(uvProposalData);
    std::string strCurrentPropsalData = uvProposal.write();
    std::string strCurrentPropsalDataHex = HexStr (strCurrentPropsalData);
    return strCurrentPropsalDataHex;
}

void NewProposalDialog::on_toolButton_clicked()
{
    //if empty - paste, else openURL;
}

void NewProposalDialog::on_pushbuttonDetails_clicked()
{
    QMessageBox msgBox;
    msgBox.setText("Proposal details");
    msgBox.setInformativeText("Do not close this dialog. ");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.exec();
}

int GetNextSuperblockTime()
{
    //return 0;
    // Compute last/next superblock
    int nLastSuperblock, nNextSuperblock;

    // Get current block height
    int nBlockHeight = 0;
    {
        LOCK(cs_main);
        nBlockHeight = (int)chainActive.Height();
    }
   // getgovernanceinfo();

    // Get chain parameters
    int nSuperblockStartBlock = Params().GetConsensus().nSuperblockStartBlock;
    int nSuperblockCycle = Params().GetConsensus().nSuperblockCycle;

    // Get first superblock
    int nFirstSuperblockOffset = (nSuperblockCycle - nSuperblockStartBlock % nSuperblockCycle) % nSuperblockCycle;
    int nFirstSuperblock = nSuperblockStartBlock + nFirstSuperblockOffset;

    if(nBlockHeight < nFirstSuperblock){
        nLastSuperblock = 0;
        nNextSuperblock = nFirstSuperblock;
    } else {
        nLastSuperblock = nBlockHeight - nBlockHeight % nSuperblockCycle;
        nNextSuperblock = nLastSuperblock + nSuperblockCycle;
    }


    return nNextSuperblock*Params().GetConsensus().nPowTargetSpacing
            +chainActive.Genesis()->GetBlockTime();

}

void NewProposalDialog::on_bnVoteYes_clicked()
{
  voteAction( "yes","");
}

void NewProposalDialog::voteAction(std::string vote, std::string strProposalHash ="")
{

    strProposalHash = pGovObj->GetHash().ToString();
    uint256 hash;
    hash = uint256S(strProposalHash);
    std::string strVoteSignal, strVoteOutcome;
    if (vote == "yes" || vote == "no" || vote =="abstain"){
        strVoteSignal = "funding";
        strVoteOutcome = vote;
    }
    else if (vote == "delete"){
        strVoteSignal = "delete";
        strVoteOutcome = "yes";
    };

    vote_signal_enum_t eVoteSignal = CGovernanceVoting::ConvertVoteSignal(strVoteSignal);
    vote_outcome_enum_t eVoteOutcome = CGovernanceVoting::ConvertVoteOutcome(strVoteOutcome);
    int nSuccessful = 0;
    int nFailed = 0;

    std::vector<CMasternodeConfig::CMasternodeEntry> mnEntries;
    mnEntries = masternodeConfig.getEntries();

    UniValue resultsObj(UniValue::VOBJ);

    BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, mnEntries) {
        std::string strError;
        std::vector<unsigned char> vchMasterNodeSignature;
        std::string strMasterNodeSignMessage;

        CPubKey pubKeyCollateralAddress;
        CKey keyCollateralAddress;
        CPubKey pubKeyMasternode;
        CKey keyMasternode;

        UniValue statusObj(UniValue::VOBJ);

        if(!CMessageSigner::GetKeysFromSecret(mne.getPrivKey(), keyMasternode, pubKeyMasternode)){
            nFailed++;
            statusObj.push_back(Pair("result", "failed"));
            statusObj.push_back(Pair("errorMessage", "Masternode signing error, could not set key correctly"));
            resultsObj.push_back(Pair(mne.getAlias(), statusObj));
            continue;
        }

            uint256 nTxHash;
            nTxHash.SetHex(mne.getTxHash());

            int nOutputIndex = 0;
            if(!ParseInt32(mne.getOutputIndex(), &nOutputIndex)) {
                continue;
            }

            COutPoint outpoint(nTxHash, nOutputIndex);

            CMasternode mn;
            bool fMnFound = mnodeman.Get(outpoint, mn);

            if(!fMnFound) {
                nFailed++;
                statusObj.push_back(Pair("result", "failed"));
                statusObj.push_back(Pair("errorMessage", "Can't find masternode by collateral output"));
                resultsObj.push_back(Pair(mne.getAlias(), statusObj));
                continue;
            }

            CGovernanceVote vote(mn.vin.prevout, hash, eVoteSignal, eVoteOutcome);
            if(!vote.Sign(keyMasternode, pubKeyMasternode)){
                nFailed++;
                statusObj.push_back(Pair("result", "failed"));
                statusObj.push_back(Pair("errorMessage", "Failure to sign."));
                resultsObj.push_back(Pair(mne.getAlias(), statusObj));
                continue;
            }

            CGovernanceException exception;
            if(governance.ProcessVoteAndRelay(vote, exception, *g_connman)) {
                nSuccessful++;
                statusObj.push_back(Pair("result", "success"));
            }
            else {
                nFailed++;
                statusObj.push_back(Pair("result", "failed"));
    //            statusObj.push_back(Pair("errorMessage", exception.GetMessage()));
            }

            resultsObj.push_back(Pair(mne.getAlias(), statusObj));
        }

        UniValue returnObj(UniValue::VOBJ);
        returnObj.push_back(Pair("overall", strprintf("Voted successfully %d time(s) and failed %d time(s).", nSuccessful, nFailed)));
        returnObj.push_back(Pair("detail", resultsObj));


        QMessageBox msgBox;
        msgBox.setText(QString::fromStdString(returnObj[0].get_str()));//  ..get_str()));
        //msgBox.setInformativeText(strErrorMessages.c_str());
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();
}

void NewProposalDialog::on_bnVoteNo_clicked()
{
    voteAction("no");
}

void NewProposalDialog::on_bnVoteAbstain_clicked()
{
     voteAction("abstain");
}

void NewProposalDialog::on_bnJoinChannel_clicked()
{
    if (ui->lineeditPrivateChatAddress->text().isEmpty()){
        std::string strAddress, strPubKey, strPrivateKey;
        GetNewChannelAddress(strAddress, strPubKey, strPrivateKey);
        ui->lineeditPrivateChatAddress->setText(strAddress.c_str());
        ui->lineeditPrivateChatPubKey->setText(strPubKey.c_str());
        ui->lineeditChannelPrivKey->setText(strPrivateKey.c_str());
    }
    else{
        //add dialog here for confirmations
        SetChannelSubscribtion(ui->lineeditPrivateChatAddress->text().toStdString(),
                            ui->lineeditPrivateChatPubKey->text().toStdString(),
                            ui->lineeditChannelPrivKey->text().toStdString(),
                            std::string("PR-") + ui->lineeditProposalName->text().toStdString()
                            );
        ui->bnJoinChannel->setEnabled(!GetIsChannelSubscribed(ui->lineeditPrivateChatAddress->text().toStdString()));
//        QMessageBox msgBox;
//        msgBox.setText("The Proposal Channel is successfully subscribed");
//        msgBox.setIcon(QMessageBox::Information);
//        msgBox.exec();

    }
}

void NewProposalDialog::on_bnOpenURL_clicked()
{
    QDesktopServices::openUrl (QUrl(ui->lineeditProposalUrl->text()));
}

void NewProposalDialog::on_texteditProposalDescription_textChanged()
{
    if (ui->texteditProposalDescription->toPlainText().size()>1000)
    {
        QTextCursor  cursor = ui->texteditProposalDescription->textCursor();
        QToolTip::showText(ui->texteditProposalDescription->mapToGlobal(QPoint()), tr("Only 1000 symbols allowed for description. Please use additional recource, if required."));
        ui->texteditProposalDescription->setText(ui->texteditProposalDescription->toPlainText().left(1000));
        //ui->texteditProposalDescription->setCursorPosition(1000);
        ui->texteditProposalDescription->setTextCursor( cursor );
    }
}
