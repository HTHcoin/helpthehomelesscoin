// Copyright (c) 2024 The HTH Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "smartcontractspage.h"
#include "ui_smartcontractspage.h"

#include "addresstablemodel.h"
#include "bitcoinunits.h"
#include "guiutil.h"
#include "optionsmodel.h"
#include "platformstyle.h"
#include "walletmodel.h"

class ClientModel;

#include "base58.h"
#include "rpc/server.h"
#include "utilstrencodings.h"

#include <univalue.h>

#include <QMessageBox>
#include <QProcess>
#include <QTemporaryFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QRegularExpression>

extern UniValue createcontract(const JSONRPCRequest& request);
extern UniValue sendtocontract(const JSONRPCRequest& request);
extern UniValue callcontract(const JSONRPCRequest& request);

SmartContractsPage::SmartContractsPage(const PlatformStyle *_platformStyle, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SmartContractsPage),
    clientModel(0),
    walletModel(0),
    platformStyle(_platformStyle)
{
    ui->setupUi(this);

    // Set up validators for hex input
    // Contract addresses and bytecode should be hex
}

SmartContractsPage::~SmartContractsPage()
{
    delete ui;
}

void SmartContractsPage::setClientModel(ClientModel *_clientModel)
{
    this->clientModel = _clientModel;
}

void SmartContractsPage::setWalletModel(WalletModel *_walletModel)
{
    this->walletModel = _walletModel;
    if (_walletModel) {
        updateAddressBook();
    }
}

void SmartContractsPage::updateAddressBook()
{
    if (!walletModel)
        return;

    // Clear existing items
    ui->createSenderAddress->clear();
    ui->sendToSenderAddress->clear();

    // Get addresses from wallet
    std::vector<std::string> addresses;

    // Use the address table model to get receiving addresses
    AddressTableModel *addressModel = walletModel->getAddressTableModel();
    if (addressModel) {
        for (int i = 0; i < addressModel->rowCount(QModelIndex()); i++) {
            QModelIndex index = addressModel->index(i, AddressTableModel::Address, QModelIndex());
            QString address = addressModel->data(index, Qt::DisplayRole).toString();
            QString type = addressModel->data(index, AddressTableModel::TypeRole).toString();

            // Add receiving addresses
            if (type == AddressTableModel::Receive) {
                ui->createSenderAddress->addItem(address);
                ui->sendToSenderAddress->addItem(address);
            }
        }
    }
}

void SmartContractsPage::on_createButton_clicked()
{
    QString bytecode = ui->createBytecodeEdit->toPlainText().trimmed();

    if (bytecode.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Please enter contract bytecode."));
        return;
    }

    // Remove 0x prefix if present
    if (bytecode.startsWith("0x") || bytecode.startsWith("0X")) {
        bytecode = bytecode.mid(2);
    }

    // Validate hex
    if (!IsHex(bytecode.toStdString())) {
        QMessageBox::warning(this, tr("Error"), tr("Bytecode must be valid hexadecimal."));
        return;
    }

    try {
        JSONRPCRequest request;
        request.params = UniValue(UniValue::VARR);
        request.params.push_back(bytecode.toStdString());
        request.params.push_back(ui->createGasLimit->value());
        request.params.push_back(ui->createGasPrice->value());

        // Add sender address if selected
        QString senderAddr = ui->createSenderAddress->currentText();
        if (!senderAddr.isEmpty()) {
            request.params.push_back(senderAddr.toStdString());
        }

        UniValue result = createcontract(request);

        QString resultText;
        if (result.isObject()) {
            resultText = QString("Transaction ID: %1\n").arg(QString::fromStdString(result["txid"].get_str()));
            resultText += QString("Contract Address: %1\n").arg(QString::fromStdString(result["address"].get_str()));
            resultText += QString("Sender: %1").arg(QString::fromStdString(result["sender"].get_str()));
        } else {
            resultText = QString::fromStdString(result.write(2));
        }

        ui->createResultEdit->setText(resultText);

        QMessageBox::information(this, tr("Success"),
            tr("Contract creation transaction sent!\nContract Address: %1\n\nMine a block to confirm.")
            .arg(QString::fromStdString(result["address"].get_str())));

    } catch (const UniValue& e) {
        QString error = QString::fromStdString(e["message"].get_str());
        ui->createResultEdit->setText("Error: " + error);
        QMessageBox::critical(this, tr("Error"), error);
    } catch (const std::exception& e) {
        ui->createResultEdit->setText("Error: " + QString::fromStdString(e.what()));
        QMessageBox::critical(this, tr("Error"), QString::fromStdString(e.what()));
    }
}

void SmartContractsPage::on_sendToButton_clicked()
{
    QString contractAddress = ui->sendToAddress->text().trimmed();
    QString data = ui->sendToDataEdit->toPlainText().trimmed();

    if (contractAddress.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Please enter a contract address."));
        return;
    }

    if (contractAddress.length() != 40 || !IsHex(contractAddress.toStdString())) {
        QMessageBox::warning(this, tr("Error"), tr("Contract address must be 40 hex characters."));
        return;
    }

    // Remove 0x prefix from data if present
    if (data.startsWith("0x") || data.startsWith("0X")) {
        data = data.mid(2);
    }

    if (!data.isEmpty() && !IsHex(data.toStdString())) {
        QMessageBox::warning(this, tr("Error"), tr("Data must be valid hexadecimal."));
        return;
    }

    try {
        JSONRPCRequest request;
        request.params = UniValue(UniValue::VARR);
        request.params.push_back(contractAddress.toStdString());
        request.params.push_back(data.toStdString());
        request.params.push_back(ui->sendToAmount->value());
        request.params.push_back(ui->sendToGasLimit->value());
        request.params.push_back(ui->sendToGasPrice->value());

        // Add sender address if selected
        QString senderAddr = ui->sendToSenderAddress->currentText();
        if (!senderAddr.isEmpty()) {
            request.params.push_back(senderAddr.toStdString());
        }

        UniValue result = sendtocontract(request);

        QString resultText;
        if (result.isObject()) {
            resultText = QString("Transaction ID: %1\n").arg(QString::fromStdString(result["txid"].get_str()));
            resultText += QString("Sender: %1").arg(QString::fromStdString(result["sender"].get_str()));
        } else {
            resultText = QString::fromStdString(result.write(2));
        }

        ui->sendToResultEdit->setText(resultText);

        QMessageBox::information(this, tr("Success"),
            tr("Contract call transaction sent!\nTransaction ID: %1\n\nMine a block to confirm.")
            .arg(QString::fromStdString(result["txid"].get_str())));

    } catch (const UniValue& e) {
        QString error = QString::fromStdString(e["message"].get_str());
        ui->sendToResultEdit->setText("Error: " + error);
        QMessageBox::critical(this, tr("Error"), error);
    } catch (const std::exception& e) {
        ui->sendToResultEdit->setText("Error: " + QString::fromStdString(e.what()));
        QMessageBox::critical(this, tr("Error"), QString::fromStdString(e.what()));
    }
}

void SmartContractsPage::on_callButton_clicked()
{
    QString contractAddress = ui->callAddress->text().trimmed();
    QString data = ui->callDataEdit->toPlainText().trimmed();

    if (contractAddress.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Please enter a contract address."));
        return;
    }

    if (contractAddress.length() != 40 || !IsHex(contractAddress.toStdString())) {
        QMessageBox::warning(this, tr("Error"), tr("Contract address must be 40 hex characters."));
        return;
    }

    // Remove 0x prefix from data if present
    if (data.startsWith("0x") || data.startsWith("0X")) {
        data = data.mid(2);
    }

    if (data.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Please enter function call data."));
        return;
    }

    if (!IsHex(data.toStdString())) {
        QMessageBox::warning(this, tr("Error"), tr("Data must be valid hexadecimal."));
        return;
    }

    try {
        JSONRPCRequest request;
        request.params = UniValue(UniValue::VARR);
        request.params.push_back(contractAddress.toStdString());
        request.params.push_back(data.toStdString());

        UniValue result = callcontract(request);

        QString resultText;
        if (result.isObject()) {
            resultText = QString("Contract: %1\n\n").arg(QString::fromStdString(result["address"].get_str()));

            if (result.exists("executionResult")) {
                UniValue execResult = result["executionResult"];
                resultText += QString("Gas Used: %1\n").arg(execResult["gasUsed"].get_int64());
                resultText += QString("Status: %1\n").arg(QString::fromStdString(execResult["excepted"].get_str()));
                resultText += QString("Output: %1").arg(QString::fromStdString(execResult["output"].get_str()));

                // Try to decode output as uint256 if it's 64 chars (32 bytes)
                std::string output = execResult["output"].get_str();
                if (output.length() == 64) {
                    // Convert hex to decimal for display
                    std::string hexVal = output;
                    // Remove leading zeros for cleaner display
                    size_t firstNonZero = hexVal.find_first_not_of('0');
                    if (firstNonZero != std::string::npos) {
                        hexVal = hexVal.substr(firstNonZero);
                    } else {
                        hexVal = "0";
                    }
                    resultText += QString("\n\nDecoded (if uint256): 0x%1").arg(QString::fromStdString(hexVal));
                }
            }
        } else {
            resultText = QString::fromStdString(result.write(2));
        }

        ui->callResultEdit->setText(resultText);

    } catch (const UniValue& e) {
        QString error = QString::fromStdString(e["message"].get_str());
        ui->callResultEdit->setText("Error: " + error);
        QMessageBox::critical(this, tr("Error"), error);
    } catch (const std::exception& e) {
        ui->callResultEdit->setText("Error: " + QString::fromStdString(e.what()));
        QMessageBox::critical(this, tr("Error"), QString::fromStdString(e.what()));
    }
}

bool SmartContractsPage::checkSolcInstalled()
{
    QProcess process;
    process.start("solc", QStringList() << "--version");
    process.waitForFinished(5000);
    return process.exitCode() == 0;
}

void SmartContractsPage::on_compileButton_clicked()
{
    QString source = ui->solidityEditor->toPlainText().trimmed();

    if (source.isEmpty()) {
        ui->compilerOutput->setText("Error: Please enter Solidity source code.");
        return;
    }

    // Check if solc is installed
    if (!checkSolcInstalled()) {
        ui->compilerOutput->setText("Error: Solidity compiler (solc) not found.\n\n"
            "Please install solc:\n"
            "  Ubuntu/Debian: sudo add-apt-repository ppa:ethereum/ethereum && sudo apt update && sudo apt install solc\n"
            "  Or download from: https://github.com/ethereum/solidity/releases");
        return;
    }

    ui->compilerOutput->setText("Compiling...");
    ui->compileButton->setEnabled(false);

    // Write source to temp file
    QTemporaryFile tempFile;
    tempFile.setFileTemplate(QDir::tempPath() + "/hth_contract_XXXXXX.sol");
    if (!tempFile.open()) {
        ui->compilerOutput->setText("Error: Could not create temporary file.");
        ui->compileButton->setEnabled(true);
        return;
    }

    tempFile.write(source.toUtf8());
    tempFile.flush();

    // Run solc with combined-json output
    QProcess process;
    QStringList args;
    args << "--combined-json" << "bin,abi" << tempFile.fileName();

    process.start("solc", args);
    process.waitForFinished(30000);

    QString output = QString::fromUtf8(process.readAllStandardOutput());
    QString errors = QString::fromUtf8(process.readAllStandardError());

    ui->compileButton->setEnabled(true);

    if (process.exitCode() != 0) {
        ui->compilerOutput->setText("Compilation failed:\n" + errors);
        return;
    }

    // Parse JSON output
    QJsonDocument doc = QJsonDocument::fromJson(output.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        ui->compilerOutput->setText("Error: Could not parse compiler output.\n" + output);
        return;
    }

    QJsonObject root = doc.object();
    QJsonObject contracts = root["contracts"].toObject();

    if (contracts.isEmpty()) {
        ui->compilerOutput->setText("Error: No contracts found in source code.");
        return;
    }

    // Clear previous data
    compiledContracts.clear();
    ui->contractSelector->clear();

    // Parse each contract
    QString compileLog = "Compilation successful!\n\nContracts found:\n";
    for (auto it = contracts.begin(); it != contracts.end(); ++it) {
        QString fullName = it.key();  // e.g., "filename.sol:ContractName"
        QJsonObject contractData = it.value().toObject();

        // Extract just the contract name (after the colon)
        QString contractName = fullName;
        int colonPos = fullName.lastIndexOf(':');
        if (colonPos >= 0) {
            contractName = fullName.mid(colonPos + 1);
        }

        QString bytecode = contractData["bin"].toString();
        QString abi = contractData["abi"].toString();

        if (!bytecode.isEmpty()) {
            compiledContracts[contractName] = qMakePair(bytecode, abi);
            ui->contractSelector->addItem(contractName);
            compileLog += QString("  - %1 (%2 bytes)\n").arg(contractName).arg(bytecode.length() / 2);
        }
    }

    if (!errors.isEmpty()) {
        compileLog += "\nWarnings:\n" + errors;
    }

    ui->compilerOutput->setText(compileLog);

    // Auto-select first contract if available
    if (ui->contractSelector->count() > 0) {
        ui->contractSelector->setCurrentIndex(0);
    }
}

void SmartContractsPage::on_contractSelector_currentIndexChanged(int index)
{
    if (index < 0)
        return;

    QString contractName = ui->contractSelector->currentText();
    if (compiledContracts.contains(contractName)) {
        QString bytecode = compiledContracts[contractName].first;
        ui->createBytecodeEdit->setPlainText(bytecode);

        ui->compilerStatusLabel->setText(QString("Selected: %1").arg(contractName));
    }
}
