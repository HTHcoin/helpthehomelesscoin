// Copyright (c) 2019-2024 The HTH Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpc/server.h"
#include "rpc/protocol.h"
#include "base58.h"
#include "chain.h"
#include "coins.h"
#include "consensus/validation.h"
#include "core_io.h"
#include "init.h"
#include "keystore.h"
#include "validation.h"
#include "net.h"
#include "policy/policy.h"
#include "primitives/transaction.h"
#include "script/script.h"
#include "script/sign.h"
#include "script/standard.h"
#include "txmempool.h"
#include "uint256.h"
#include "utilstrencodings.h"
#include "utilmoneystr.h"
#include "chainparams.h"
#include "qtum/hthcontract.h"
#include "qtum/simplevm.h"
#include "qtum/contractdb.h"

#ifdef ENABLE_WALLET
#include "wallet/wallet.h"
#include "wallet/rpcwallet.h"
#endif

#include <univalue.h>

// Default gas parameters
static const uint64_t DEFAULT_GAS_LIMIT_OP_CREATE = 2500000;
static const uint64_t DEFAULT_GAS_LIMIT_OP_SEND = 250000;
static const uint64_t DEFAULT_GAS_LIMIT_OP_CALL = 250000;
static const CAmount DEFAULT_GAS_PRICE = 40;  // 40 satoshis per gas unit
static const uint64_t MINIMUM_GAS_LIMIT = 10000;
static const uint64_t MAXIMUM_GAS_LIMIT = 50000000;

// Storage interface implementation for SimpleVM
class ContractStorage : public SimpleVM::StorageInterface {
private:
    std::vector<unsigned char> contractAddr;
public:
    ContractStorage(const std::vector<unsigned char>& addr) : contractAddr(addr) {}

    std::vector<unsigned char> getStorage(const std::vector<unsigned char>& key) override {
        return GetContractStorage(contractAddr, key);
    }

    void setStorage(const std::vector<unsigned char>& key, const std::vector<unsigned char>& value) override {
        // For read-only calls, we don't persist storage changes
        // This could be used for simulating state changes in the future
    }
};

// Calculate contract address from transaction hash and vout index
std::string GetContractAddressFromTx(const uint256& txHash, uint32_t vout)
{
    std::vector<unsigned char> txIdAndVout(txHash.begin(), txHash.end());

    // Append vout as little-endian 4 bytes
    std::vector<unsigned char> voutBytes(4);
    memcpy(voutBytes.data(), &vout, sizeof(vout));
    txIdAndVout.insert(txIdAndVout.end(), voutBytes.begin(), voutBytes.end());

    // SHA256 then RIPEMD160 to get 20-byte address
    std::vector<unsigned char> sha256Hash(32);
    CSHA256().Write(txIdAndVout.data(), txIdAndVout.size()).Finalize(sha256Hash.data());

    std::vector<unsigned char> contractAddress(20);
    CRIPEMD160().Write(sha256Hash.data(), sha256Hash.size()).Finalize(contractAddress.data());

    return HexStr(contractAddress);
}

UniValue createcontract(const JSONRPCRequest& request)
{
#ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);

    if (request.fHelp || request.params.size() < 1 || request.params.size() > 5)
        throw std::runtime_error(
            "createcontract \"bytecode\" ( gaslimit gasprice \"senderaddress\" broadcast )\n"
            "\nCreate a contract with bytecode.\n"
            + HelpRequiringPassphrase(pwallet) +
            "\nArguments:\n"
            "1. \"bytecode\"       (string, required) The contract bytecode (hex encoded)\n"
            "2. gaslimit          (numeric, optional, default=" + std::to_string(DEFAULT_GAS_LIMIT_OP_CREATE) + ") Gas limit for contract execution\n"
            "3. gasprice          (numeric, optional, default=" + FormatMoney(DEFAULT_GAS_PRICE) + ") Gas price in " + CURRENCY_UNIT + " per gas unit\n"
            "4. \"senderaddress\"  (string, optional) The address that will be used to create the contract\n"
            "5. broadcast         (boolean, optional, default=true) Whether to broadcast the transaction\n"
            "\nResult:\n"
            "{\n"
            "  \"txid\" : \"hex\",       (string) The transaction id\n"
            "  \"sender\" : \"address\", (string) " + CURRENCY_UNIT + " address of the sender\n"
            "  \"hash160\" : \"hex\",    (string) RIPEMD-160 hash of the sender\n"
            "  \"address\" : \"hex\"     (string) Expected contract address\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("createcontract", "\"6060604052...\"")
            + HelpExampleCli("createcontract", "\"6060604052...\" 2500000 0.0000004")
            + HelpExampleRpc("createcontract", "\"6060604052...\"")
        );

    if (!pwallet) {
        throw JSONRPCError(RPC_WALLET_ERROR, "Wallet not available");
    }

    LOCK2(cs_main, pwallet->cs_wallet);

    // Check if EVM is enabled
    const Consensus::Params& params = Params().GetConsensus();
    int currentHeight = chainActive.Height();
    if (!IsEVMEnabled(currentHeight + 1, params)) {
        throw JSONRPCError(RPC_MISC_ERROR, "EVM is not enabled at current height. EVM activates at height " +
            std::to_string(params.nEVMStartHeight));
    }

    EnsureWalletIsUnlocked(pwallet);

    // Parse bytecode
    std::string bytecode = request.params[0].get_str();
    if (bytecode.size() % 2 != 0 || !IsHex(bytecode)) {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid bytecode (must be hex encoded)");
    }
    if (bytecode.empty()) {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Bytecode cannot be empty");
    }

    // Parse gas limit
    uint64_t nGasLimit = DEFAULT_GAS_LIMIT_OP_CREATE;
    if (request.params.size() > 1 && !request.params[1].isNull()) {
        nGasLimit = request.params[1].get_int64();
        if (nGasLimit < MINIMUM_GAS_LIMIT) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Gas limit too low (minimum: " + std::to_string(MINIMUM_GAS_LIMIT) + ")");
        }
        if (nGasLimit > MAXIMUM_GAS_LIMIT) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Gas limit too high (maximum: " + std::to_string(MAXIMUM_GAS_LIMIT) + ")");
        }
    }

    // Parse gas price
    CAmount nGasPrice = DEFAULT_GAS_PRICE;
    if (request.params.size() > 2 && !request.params[2].isNull()) {
        nGasPrice = AmountFromValue(request.params[2]);
        if (nGasPrice <= 0) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid gas price");
        }
    }

    // Parse sender address (optional)
    CBitcoinAddress senderAddress;
    bool hasSender = false;
    if (request.params.size() > 3 && !request.params[3].isNull() && !request.params[3].get_str().empty()) {
        senderAddress.SetString(request.params[3].get_str());
        if (!senderAddress.IsValid()) {
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid sender address");
        }
        hasSender = true;
    }

    // Parse broadcast flag
    bool fBroadcast = true;
    if (request.params.size() > 4 && !request.params[4].isNull()) {
        fBroadcast = request.params[4].get_bool();
    }

    // Calculate gas fee
    CAmount nGasFee = nGasPrice * nGasLimit;

    // Check wallet balance
    CAmount curBalance = pwallet->GetBalance();
    if (nGasFee > curBalance) {
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Insufficient funds for gas fee");
    }

    // Find a suitable UTXO to fund the transaction
    std::vector<COutput> vecOutputs;
    pwallet->AvailableCoins(vecOutputs, true, nullptr, false, ALL_COINS, false);

    COutPoint selectedOutpoint;
    CAmount selectedAmount = 0;
    CScript selectedScript;
    CBitcoinAddress selectedAddress;
    CKeyID selectedKeyID;

    for (const COutput& out : vecOutputs) {
        CTxDestination dest;
        if (!ExtractDestination(out.tx->tx->vout[out.i].scriptPubKey, dest)) {
            continue;
        }

        // Check if it's a P2PKH output (required for contract sender)
        if (dest.type() != typeid(CKeyID)) {
            continue;
        }

        CKeyID keyID = boost::get<CKeyID>(dest);
        CBitcoinAddress addr(dest);

        // If sender specified, only use UTXOs from that address
        if (hasSender && addr.ToString() != senderAddress.ToString()) {
            continue;
        }

        // Check if we have the key
        CKey key;
        if (!pwallet->GetKey(keyID, key)) {
            continue;
        }

        // Use this UTXO if it's large enough
        CAmount utxoAmount = out.tx->tx->vout[out.i].nValue;
        if (utxoAmount >= nGasFee + 10000) { // Add small margin for tx fee
            selectedOutpoint = COutPoint(out.tx->tx->GetHash(), out.i);
            selectedAmount = utxoAmount;
            selectedScript = out.tx->tx->vout[out.i].scriptPubKey;
            selectedAddress = addr;
            selectedKeyID = keyID;
            break;
        }
    }

    if (selectedAmount == 0) {
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "No suitable UTXO found");
    }

    // Build the OP_CREATE script
    // Format: <version> <gasLimit> <gasPrice> <bytecode> OP_CREATE
    CScript contractScript;
    contractScript << CScriptNum(1);  // Version 1
    contractScript << CScriptNum(nGasLimit);
    contractScript << CScriptNum(nGasPrice);
    contractScript << ParseHex(bytecode);
    contractScript << OP_CREATE;

    // Create the transaction
    CMutableTransaction mtx;
    mtx.nVersion = 2;

    // Add input
    mtx.vin.push_back(CTxIn(selectedOutpoint));

    // Add contract output (0 value) - must be first for contract address calculation
    mtx.vout.push_back(CTxOut(0, contractScript));

    // Add change output
    CAmount nTxFee = 10000; // 0.0001 HTH transaction fee
    CAmount nChange = selectedAmount - nGasFee - nTxFee;
    if (nChange > 546) { // Dust threshold
        CScript changeScript = GetScriptForDestination(CTxDestination(selectedKeyID));
        mtx.vout.push_back(CTxOut(nChange, changeScript));
    }

    // Sign the transaction
    const CTransaction txToSign(mtx);

    if (!ProduceSignature(TransactionSignatureCreator(pwallet, &txToSign, 0, SIGHASH_ALL),
                          selectedScript, mtx.vin[0].scriptSig)) {
        throw JSONRPCError(RPC_WALLET_ERROR, "Failed to sign transaction");
    }

    // Create final transaction
    CTransaction finalTx(mtx);

    // Find the contract output vout (should be 0)
    uint32_t contractVout = 0;
    for (uint32_t i = 0; i < finalTx.vout.size(); i++) {
        if (finalTx.vout[i].scriptPubKey.HasOpCreate()) {
            contractVout = i;
            break;
        }
    }

    // Calculate contract address
    std::string contractAddress = GetContractAddressFromTx(finalTx.GetHash(), contractVout);

    UniValue result(UniValue::VOBJ);

    if (fBroadcast) {
        // Broadcast
        CValidationState state;
        bool fMissingInputs;

        if (!AcceptToMemoryPool(mempool, state, MakeTransactionRef(finalTx), true, &fMissingInputs, false, 0)) {
            if (state.IsInvalid()) {
                throw JSONRPCError(RPC_TRANSACTION_REJECTED, strprintf("Transaction rejected: %s", state.GetRejectReason()));
            } else {
                throw JSONRPCError(RPC_TRANSACTION_ERROR, strprintf("Transaction not accepted: %s", state.GetRejectReason()));
            }
        }

        g_connman->RelayTransaction(finalTx);

        result.push_back(Pair("txid", finalTx.GetHash().GetHex()));
    } else {
        result.push_back(Pair("raw transaction", EncodeHexTx(finalTx)));
    }

    result.push_back(Pair("sender", selectedAddress.ToString()));
    result.push_back(Pair("hash160", HexStr(std::vector<unsigned char>(selectedKeyID.begin(), selectedKeyID.end()))));
    result.push_back(Pair("address", contractAddress));

    return result;

#else
    throw JSONRPCError(RPC_WALLET_ERROR, "Wallet support not compiled in");
#endif
}

UniValue sendtocontract(const JSONRPCRequest& request)
{
#ifdef ENABLE_WALLET
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);

    if (request.fHelp || request.params.size() < 2 || request.params.size() > 7)
        throw std::runtime_error(
            "sendtocontract \"contractaddress\" \"datahex\" ( amount gaslimit gasprice \"senderaddress\" broadcast )\n"
            "\nSend funds and data to a contract.\n"
            + HelpRequiringPassphrase(pwallet) +
            "\nArguments:\n"
            "1. \"contractaddress\" (string, required) The contract address (20 bytes hex)\n"
            "2. \"datahex\"         (string, required) The data to send (hex encoded ABI)\n"
            "3. amount             (numeric, optional, default=0) The amount in " + CURRENCY_UNIT + " to send\n"
            "4. gaslimit           (numeric, optional, default=" + std::to_string(DEFAULT_GAS_LIMIT_OP_SEND) + ") Gas limit\n"
            "5. gasprice           (numeric, optional, default=" + FormatMoney(DEFAULT_GAS_PRICE) + ") Gas price\n"
            "6. \"senderaddress\"   (string, optional) The sender address\n"
            "7. broadcast          (boolean, optional, default=true) Whether to broadcast\n"
            "\nResult:\n"
            "{\n"
            "  \"txid\" : \"hex\",       (string) The transaction id\n"
            "  \"sender\" : \"address\", (string) Sender address\n"
            "  \"hash160\" : \"hex\"     (string) RIPEMD-160 hash of sender\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("sendtocontract", "\"eb23c0b3e6042821da281a2e2364feb22dd543e3\" \"a9059cbb000000...\"")
            + HelpExampleRpc("sendtocontract", "\"eb23c0b3e6042821da281a2e2364feb22dd543e3\", \"a9059cbb000000...\"")
        );

    if (!pwallet) {
        throw JSONRPCError(RPC_WALLET_ERROR, "Wallet not available");
    }

    LOCK2(cs_main, pwallet->cs_wallet);

    // Check if EVM is enabled
    const Consensus::Params& params = Params().GetConsensus();
    int currentHeight = chainActive.Height();
    if (!IsEVMEnabled(currentHeight + 1, params)) {
        throw JSONRPCError(RPC_MISC_ERROR, "EVM is not enabled at current height");
    }

    EnsureWalletIsUnlocked(pwallet);

    // Parse contract address
    std::string contractAddress = request.params[0].get_str();
    if (contractAddress.size() != 40 || !IsHex(contractAddress)) {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid contract address (must be 40 hex characters)");
    }

    // Parse data
    std::string datahex = request.params[1].get_str();
    if (datahex.size() % 2 != 0 || !IsHex(datahex)) {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid data (must be hex encoded)");
    }

    // Parse amount
    CAmount nAmount = 0;
    if (request.params.size() > 2 && !request.params[2].isNull()) {
        nAmount = AmountFromValue(request.params[2]);
        if (nAmount < 0) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid amount");
        }
    }

    // Parse gas limit
    uint64_t nGasLimit = DEFAULT_GAS_LIMIT_OP_SEND;
    if (request.params.size() > 3 && !request.params[3].isNull()) {
        nGasLimit = request.params[3].get_int64();
    }

    // Parse gas price
    CAmount nGasPrice = DEFAULT_GAS_PRICE;
    if (request.params.size() > 4 && !request.params[4].isNull()) {
        nGasPrice = AmountFromValue(request.params[4]);
    }

    // Parse sender address
    CBitcoinAddress senderAddress;
    bool hasSender = false;
    if (request.params.size() > 5 && !request.params[5].isNull() && !request.params[5].get_str().empty()) {
        senderAddress.SetString(request.params[5].get_str());
        if (!senderAddress.IsValid()) {
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid sender address");
        }
        hasSender = true;
    }

    // Parse broadcast flag
    bool fBroadcast = true;
    if (request.params.size() > 6 && !request.params[6].isNull()) {
        fBroadcast = request.params[6].get_bool();
    }

    // Calculate fees
    CAmount nGasFee = nGasPrice * nGasLimit;
    CAmount nTotalNeeded = nAmount + nGasFee;

    // Find suitable UTXO
    std::vector<COutput> vecOutputs;
    pwallet->AvailableCoins(vecOutputs, true, nullptr, false, ALL_COINS, false);

    COutPoint selectedOutpoint;
    CAmount selectedAmount = 0;
    CScript selectedScript;
    CBitcoinAddress selectedAddr;
    CKeyID selectedKeyID;

    for (const COutput& out : vecOutputs) {
        CTxDestination dest;
        if (!ExtractDestination(out.tx->tx->vout[out.i].scriptPubKey, dest)) {
            continue;
        }

        if (dest.type() != typeid(CKeyID)) {
            continue;
        }

        CKeyID keyID = boost::get<CKeyID>(dest);
        CBitcoinAddress addr(dest);

        if (hasSender && addr.ToString() != senderAddress.ToString()) {
            continue;
        }

        CKey key;
        if (!pwallet->GetKey(keyID, key)) {
            continue;
        }

        CAmount utxoAmount = out.tx->tx->vout[out.i].nValue;
        if (utxoAmount >= nTotalNeeded + 10000) {
            selectedOutpoint = COutPoint(out.tx->tx->GetHash(), out.i);
            selectedAmount = utxoAmount;
            selectedScript = out.tx->tx->vout[out.i].scriptPubKey;
            selectedAddr = addr;
            selectedKeyID = keyID;
            break;
        }
    }

    if (selectedAmount == 0) {
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "No suitable UTXO found");
    }

    // Build the OP_CALL script
    // Format: <version> <gasLimit> <gasPrice> <data> <contractAddress> OP_CALL
    CScript contractScript;
    contractScript << CScriptNum(1);  // Version 1
    contractScript << CScriptNum(nGasLimit);
    contractScript << CScriptNum(nGasPrice);
    contractScript << ParseHex(datahex);
    contractScript << ParseHex(contractAddress);
    contractScript << OP_CALL;

    // Create transaction
    CMutableTransaction mtx;
    mtx.nVersion = 2;

    mtx.vin.push_back(CTxIn(selectedOutpoint));

    // Contract output with optional value
    mtx.vout.push_back(CTxOut(nAmount, contractScript));

    // Change output
    CAmount nTxFee = 10000;
    CAmount nChange = selectedAmount - nAmount - nGasFee - nTxFee;
    if (nChange > 546) {
        CScript changeScript = GetScriptForDestination(CTxDestination(selectedKeyID));
        mtx.vout.push_back(CTxOut(nChange, changeScript));
    }

    // Sign
    const CTransaction txToSign(mtx);
    if (!ProduceSignature(TransactionSignatureCreator(pwallet, &txToSign, 0, SIGHASH_ALL),
                          selectedScript, mtx.vin[0].scriptSig)) {
        throw JSONRPCError(RPC_WALLET_ERROR, "Failed to sign transaction");
    }

    CTransaction finalTx(mtx);

    UniValue result(UniValue::VOBJ);

    if (fBroadcast) {
        CValidationState state;
        bool fMissingInputs;

        if (!AcceptToMemoryPool(mempool, state, MakeTransactionRef(finalTx), true, &fMissingInputs, false, 0)) {
            throw JSONRPCError(RPC_TRANSACTION_REJECTED, strprintf("Transaction rejected: %s", state.GetRejectReason()));
        }

        g_connman->RelayTransaction(finalTx);

        result.push_back(Pair("txid", finalTx.GetHash().GetHex()));
    } else {
        result.push_back(Pair("raw transaction", EncodeHexTx(finalTx)));
    }

    result.push_back(Pair("sender", selectedAddr.ToString()));
    result.push_back(Pair("hash160", HexStr(std::vector<unsigned char>(selectedKeyID.begin(), selectedKeyID.end()))));

    return result;

#else
    throw JSONRPCError(RPC_WALLET_ERROR, "Wallet support not compiled in");
#endif
}

UniValue callcontract(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 2 || request.params.size() > 4)
        throw std::runtime_error(
            "callcontract \"contractaddress\" \"data\" ( \"senderaddress\" gaslimit )\n"
            "\nCall a contract function (read-only, no state changes).\n"
            "\nArguments:\n"
            "1. \"contractaddress\" (string, required) The contract address (20 bytes hex)\n"
            "2. \"data\"            (string, required) The data to send (hex encoded)\n"
            "3. \"senderaddress\"   (string, optional) The sender address for context\n"
            "4. gaslimit           (numeric, optional, default=" + std::to_string(DEFAULT_GAS_LIMIT_OP_CALL) + ") Gas limit\n"
            "\nResult:\n"
            "{\n"
            "  \"address\" : \"hex\",    (string) The contract address\n"
            "  \"executionResult\" : {   (object) Execution result\n"
            "    \"gasUsed\" : n,        (numeric) Gas used\n"
            "    \"excepted\" : \"str\", (string) Exception if any\n"
            "    \"output\" : \"hex\"    (string) Return value (hex)\n"
            "  }\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("callcontract", "\"eb23c0b3e6042821da281a2e2364feb22dd543e3\" \"06fdde03\"")
            + HelpExampleRpc("callcontract", "\"eb23c0b3e6042821da281a2e2364feb22dd543e3\", \"06fdde03\"")
        );

    LOCK(cs_main);

    // Validate contract address
    std::string contractAddress = request.params[0].get_str();
    if (contractAddress.size() != 40 || !IsHex(contractAddress)) {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid contract address (must be 40 hex characters)");
    }

    // Validate data
    std::string data = request.params[1].get_str();
    if (data.size() % 2 != 0 || !IsHex(data)) {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid data (must be hex encoded)");
    }

    // Parse sender address (optional)
    std::vector<unsigned char> senderAddr(20, 0);
    if (request.params.size() > 2 && !request.params[2].isNull() && !request.params[2].get_str().empty()) {
        std::string senderHex = request.params[2].get_str();
        if (senderHex.size() == 40 && IsHex(senderHex)) {
            senderAddr = ParseHex(senderHex);
        }
    }

    // Parse gas limit (optional)
    uint64_t gasLimit = DEFAULT_GAS_LIMIT_OP_CALL;
    if (request.params.size() > 3 && !request.params[3].isNull()) {
        gasLimit = request.params[3].get_int64();
    }

    // Get contract code
    std::vector<unsigned char> contractAddr = ParseHex(contractAddress);
    std::vector<unsigned char> code = GetContractCode(contractAddr);

    if (code.empty()) {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Contract not found at address");
    }

    // Parse call data
    std::vector<unsigned char> callData = ParseHex(data);

    // Create storage interface
    ContractStorage storage(contractAddr);

    // Execute using SimpleVM
    SimpleVM::ExecutionResult execResult = SimpleVM::CallContract(
        contractAddr,
        code,
        callData,
        &storage,
        gasLimit
    );

    // Build result
    UniValue result(UniValue::VOBJ);
    result.push_back(Pair("address", contractAddress));

    UniValue executionResult(UniValue::VOBJ);
    executionResult.push_back(Pair("gasUsed", (int64_t)execResult.gasUsed));
    executionResult.push_back(Pair("excepted", execResult.success ? "None" : execResult.error));
    executionResult.push_back(Pair("output", HexStr(execResult.output)));

    result.push_back(Pair("executionResult", executionResult));

    return result;
}

UniValue getcontractcode(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 1)
        throw std::runtime_error(
            "getcontractcode \"contractaddress\"\n"
            "\nGet the bytecode at a contract address.\n"
            "\nArguments:\n"
            "1. \"contractaddress\" (string, required) The contract address (20 bytes hex)\n"
            "\nResult:\n"
            "\"bytecode\"           (string) The contract bytecode (hex)\n"
            "\nExamples:\n"
            + HelpExampleCli("getcontractcode", "\"eb23c0b3e6042821da281a2e2364feb22dd543e3\"")
            + HelpExampleRpc("getcontractcode", "\"eb23c0b3e6042821da281a2e2364feb22dd543e3\"")
        );

    std::string contractAddress = request.params[0].get_str();
    if (contractAddress.size() != 40 || !IsHex(contractAddress)) {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid contract address (must be 40 hex characters)");
    }

    // TODO: Query globalState for contract code when implemented
    std::vector<unsigned char> code = GetContractCode(ParseHex(contractAddress));

    return HexStr(code);
}

static const CRPCCommand commands[] =
{ //  category              name                      actor (function)         okSafeMode
  //  --------------------- ------------------------  -----------------------  ----------
    { "contracts",          "createcontract",         &createcontract,         false, {"bytecode","gaslimit","gasprice","senderaddress","broadcast"} },
    { "contracts",          "sendtocontract",         &sendtocontract,         false, {"contractaddress","datahex","amount","gaslimit","gasprice","senderaddress","broadcast"} },
    { "contracts",          "callcontract",           &callcontract,           true,  {"contractaddress","data","senderaddress"} },
    { "contracts",          "getcontractcode",        &getcontractcode,        true,  {"contractaddress"} },
};

void RegisterContractRPCCommands(CRPCTable &t)
{
    for (unsigned int vcidx = 0; vcidx < ARRAYLEN(commands); vcidx++)
        t.appendCommand(commands[vcidx].name, &commands[vcidx]);
}
