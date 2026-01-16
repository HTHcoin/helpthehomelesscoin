// Copyright (c) 2019-2024 The HTH Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "qtum/hthcontract.h"
#include "qtum/contractdb.h"
#include "qtum/simplevm.h"

#include "chainparams.h"
#include "coins.h"
#include "consensus/validation.h"
#include "crypto/ripemd160.h"
#include "crypto/sha256.h"
#include "primitives/block.h"
#include "script/script.h"
#include "script/standard.h"
#include "util.h"
#include "validation.h"

// Hard fork activation height (legacy, now use consensus.nEVMStartHeight)
int nEVMStartHeight = 0;

// Storage interface for contract execution during block validation
class BlockContractStorage : public SimpleVM::StorageInterface {
private:
    std::vector<unsigned char> contractAddr;
public:
    BlockContractStorage(const std::vector<unsigned char>& addr) : contractAddr(addr) {}

    std::vector<unsigned char> getStorage(const std::vector<unsigned char>& key) override {
        return GetContractStorage(contractAddr, key);
    }

    void setStorage(const std::vector<unsigned char>& key, const std::vector<unsigned char>& value) override {
        SetContractStorage(contractAddr, key, value);
    }
};

// Extract sender address from transaction for use as msg.sender in EVM
// Returns a 20-byte address derived from the transaction sender
std::vector<unsigned char> GetSenderAddress(const CTransaction& tx, const CCoinsViewCache& view)
{
    std::vector<unsigned char> senderAddr(20, 0);

    // Coinbase transactions have no sender
    if (tx.IsCoinBase()) {
        return senderAddr;
    }

    if (tx.vin.empty()) {
        return senderAddr;
    }

    // Strategy 1: Try to get from the change output (most reliable)
    // Look for a P2PKH output that is likely the change back to sender
    for (const CTxOut& out : tx.vout) {
        // Skip contract outputs (OP_CREATE, OP_CALL)
        if (out.scriptPubKey.HasOpCreate() || out.scriptPubKey.HasOpCall()) {
            continue;
        }

        txnouttype type;
        std::vector<std::vector<unsigned char>> solutions;
        if (Solver(out.scriptPubKey, type, solutions)) {
            if (type == TX_PUBKEYHASH && solutions.size() >= 1 && solutions[0].size() == 20) {
                // Found a P2PKH output - this is likely the change address (sender)
                senderAddr = solutions[0];
                return senderAddr;
            }
        }
    }

    // Strategy 2: Try to extract pubkey from P2PKH scriptSig: <sig> <pubkey>
    const CScript& scriptSig = tx.vin[0].scriptSig;
    if (scriptSig.size() >= 33) {
        CScript::const_iterator it = scriptSig.begin();
        opcodetype opcode;
        std::vector<unsigned char> data;

        // Skip signature
        if (scriptSig.GetOp(it, opcode, data)) {
            // Get pubkey
            if (scriptSig.GetOp(it, opcode, data)) {
                if (data.size() == 33 || data.size() == 65) {
                    // Hash pubkey: SHA256 then RIPEMD160
                    std::vector<unsigned char> sha256Hash(32);
                    CSHA256().Write(data.data(), data.size()).Finalize(sha256Hash.data());
                    CRIPEMD160().Write(sha256Hash.data(), sha256Hash.size()).Finalize(senderAddr.data());
                    return senderAddr;
                }
            }
        }
    }

    // Strategy 3: Try to get from the input's previous output (if available)
    const COutPoint& prevout = tx.vin[0].prevout;
    const Coin& coin = view.AccessCoin(prevout);

    if (!coin.IsSpent()) {
        const CScript& scriptPubKey = coin.out.scriptPubKey;
        txnouttype type;
        std::vector<std::vector<unsigned char>> solutions;

        if (Solver(scriptPubKey, type, solutions)) {
            if (type == TX_PUBKEYHASH && solutions.size() >= 1 && solutions[0].size() == 20) {
                senderAddr = solutions[0];
                return senderAddr;
            } else if (type == TX_SCRIPTHASH && solutions.size() >= 1 && solutions[0].size() == 20) {
                senderAddr = solutions[0];
                return senderAddr;
            } else if (type == TX_PUBKEY && solutions.size() >= 1) {
                // P2PK: solutions[0] is the pubkey, hash it
                std::vector<unsigned char> sha256Hash(32);
                CSHA256().Write(solutions[0].data(), solutions[0].size()).Finalize(sha256Hash.data());
                CRIPEMD160().Write(sha256Hash.data(), sha256Hash.size()).Finalize(senderAddr.data());
                return senderAddr;
            }
        }
    }

    return senderAddr;
}

bool IsEVMEnabled(int nHeight, const Consensus::Params& params)
{
    // EVM is enabled after the hard fork activation height
    // Use consensus param if set, otherwise fall back to global variable
    int activationHeight = params.nEVMStartHeight > 0 ? params.nEVMStartHeight : nEVMStartHeight;
    return activationHeight > 0 && nHeight >= activationHeight;
}

bool HasContractOps(const CTransaction& tx)
{
    for (const CTxOut& out : tx.vout) {
        if (out.scriptPubKey.HasOpCreate() || out.scriptPubKey.HasOpCall()) {
            return true;
        }
    }
    return false;
}

bool CheckContractTx(const CTransaction& tx, CValidationState& state, int nHeight)
{
    const Consensus::Params& params = Params().GetConsensus();

    // If EVM not enabled, reject contract transactions
    if (!IsEVMEnabled(nHeight, params)) {
        if (HasContractOps(tx)) {
            return state.DoS(100, false, REJECT_INVALID, "contract-before-hardfork",
                            false, "Contract transactions not allowed before EVM hard fork");
        }
        return true;
    }

    // Validate contract output structure
    for (size_t i = 0; i < tx.vout.size(); i++) {
        const CScript& scriptPubKey = tx.vout[i].scriptPubKey;

        // Check OP_CREATE format: version gasLimit gasPrice data OP_CREATE
        // Uses CScriptNum encoding (variable length), minimum ~5 bytes:
        // - version: 1 byte (OP_0-OP_16 or push)
        // - gasLimit: 1+ bytes
        // - gasPrice: 1+ bytes
        // - data: 1+ bytes (at least push opcode)
        // - OP_CREATE: 1 byte
        if (scriptPubKey.HasOpCreate()) {
            if (scriptPubKey.size() < 5) {
                return state.DoS(100, false, REJECT_INVALID, "bad-contract-create",
                                false, "OP_CREATE output too small");
            }
        }

        // Check OP_CALL format: version gasLimit gasPrice data contractAddr(20) OP_CALL
        // Minimum ~25 bytes due to 20-byte contract address
        if (scriptPubKey.HasOpCall()) {
            if (scriptPubKey.size() < 25) {
                return state.DoS(100, false, REJECT_INVALID, "bad-contract-call",
                                false, "OP_CALL output too small");
            }
        }

        // Check OP_SENDER usage (must be with OP_CREATE or OP_CALL)
        if (scriptPubKey.HasOpSender()) {
            if (!scriptPubKey.HasOpCreate() && !scriptPubKey.HasOpCall()) {
                return state.DoS(100, false, REJECT_INVALID, "bad-op-sender",
                                false, "OP_SENDER without OP_CREATE or OP_CALL");
            }
        }
    }

    // Validate contract input spends (must use OP_SPEND)
    // This is checked in ConnectBlock with access to UTXO set

    return true;
}

// Parse contract script parameters
// Script format: version gasLimit gasPrice data [contractAddr] OP_CREATE/OP_CALL
bool ParseContractScript(const CScript& script, bool isCreate,
                         uint64_t& version, uint64_t& gasLimit, uint64_t& gasPrice,
                         std::vector<unsigned char>& data, std::vector<unsigned char>& contractAddr)
{
    CScript::const_iterator pc = script.begin();
    opcodetype opcode;
    std::vector<unsigned char> vchPushValue;
    std::vector<std::vector<unsigned char>> stack;

    // Parse all push operations
    while (pc < script.end()) {
        if (!script.GetOp(pc, opcode, vchPushValue)) {
            return false;
        }

        // Stop at the final opcode (OP_CREATE or OP_CALL)
        if (opcode == OP_CREATE || opcode == OP_CALL) {
            break;
        }

        // If GetOp gave us push data, add it to stack
        if (vchPushValue.size() > 0) {
            stack.push_back(vchPushValue);
        }
        // Handle small integer opcodes (OP_0 through OP_16)
        else if (opcode == OP_0) {
            stack.push_back(std::vector<unsigned char>(1, 0));
        }
        else if (opcode >= OP_1 && opcode <= OP_16) {
            stack.push_back(std::vector<unsigned char>(1, opcode - OP_1 + 1));
        }
        else if (opcode == OP_1NEGATE) {
            stack.push_back(std::vector<unsigned char>(1, 0x81));  // -1 in script encoding
        }
    }

    // Expected stack for OP_CREATE: version gasLimit gasPrice data (4 items)
    // Expected stack for OP_CALL: version gasLimit gasPrice data contractAddr (5 items)
    size_t expectedSize = isCreate ? 4 : 5;
    if (stack.size() < expectedSize) {
        return false;
    }

    // Parse version (first item)
    version = 0;
    if (stack[0].size() == 1) {
        version = stack[0][0];
    } else if (stack[0].size() > 1) {
        // CScriptNum decoding for multi-byte integers
        for (size_t i = 0; i < stack[0].size() && i < 8; i++) {
            version |= ((uint64_t)stack[0][i]) << (8 * i);
        }
    }

    // Parse gasLimit (second item)
    gasLimit = 0;
    for (size_t i = 0; i < stack[1].size() && i < 8; i++) {
        gasLimit |= ((uint64_t)stack[1][i]) << (8 * i);
    }

    // Parse gasPrice (third item)
    gasPrice = 0;
    for (size_t i = 0; i < stack[2].size() && i < 8; i++) {
        gasPrice |= ((uint64_t)stack[2][i]) << (8 * i);
    }

    // Parse data (bytecode for create, call data for call)
    data = stack[3];

    // Parse contract address for OP_CALL
    if (!isCreate && stack.size() >= 5) {
        contractAddr = stack[4];
        if (contractAddr.size() != 20) {
            return false;  // Contract address must be 20 bytes
        }
    }

    return true;
}

bool ExecuteBlockContracts(const CBlock& block, const CBlockIndex* pindex,
                           CCoinsViewCache& view, CValidationState& state)
{
    const Consensus::Params& params = Params().GetConsensus();

    // If EVM not enabled, nothing to execute
    if (!IsEVMEnabled(pindex->nHeight, params)) {
        return true;
    }

    // Process contract transactions
    for (const auto& ptx : block.vtx) {
        const CTransaction& tx = *ptx;
        if (!HasContractOps(tx)) {
            continue;
        }

        if (!CheckContractTx(tx, state, pindex->nHeight)) {
            return false;
        }

        // Process each output that has contract operations
        for (size_t i = 0; i < tx.vout.size(); i++) {
            const CTxOut& out = tx.vout[i];
            const CScript& script = out.scriptPubKey;

            if (script.HasOpCreate()) {
                // Parse the OP_CREATE script
                uint64_t version, gasLimit, gasPrice;
                std::vector<unsigned char> initCode, dummy;

                if (!ParseContractScript(script, true, version, gasLimit, gasPrice, initCode, dummy)) {
                    LogPrintf("HTH-EVM: Failed to parse OP_CREATE script in tx %s\n", tx.GetHash().ToString());
                    continue;
                }

                // Calculate contract address
                std::vector<unsigned char> contractAddr = CalculateContractAddress(tx.GetHash(), i);

                // Execute the init code (constructor) to get the deployed bytecode
                // Create writable storage interface for the constructor
                BlockContractStorage constructorStorage(contractAddr);

                // Extract sender address from transaction
                std::vector<unsigned char> senderAddr = GetSenderAddress(tx, view);

                SimpleVM::VMContext ctx;
                ctx.contractAddress = contractAddr;
                ctx.callerAddress = senderAddr;
                ctx.originAddress = senderAddr;
                ctx.callData = std::vector<unsigned char>(); // No call data for constructor
                ctx.code = initCode;
                ctx.gasLimit = gasLimit;
                ctx.value = out.nValue;
                ctx.blockNumber = pindex->nHeight;
                ctx.timestamp = pindex->nTime;
                ctx.gasPrice = gasPrice;
                ctx.storage = &constructorStorage; // Allow storage access during constructor
                ctx.readOnly = false; // Allow state changes during constructor

                SimpleVM::ExecutionResult result = SimpleVM::Execute(ctx);

                std::vector<unsigned char> deployedCode;
                if (result.success && !result.output.empty()) {
                    // The returned output is the deployed bytecode
                    deployedCode = result.output;
                    LogPrintf("HTH-EVM: Constructor executed successfully, deployed code size=%d\n", deployedCode.size());
                } else {
                    // Constructor failed or returned empty - use init code as fallback
                    // This handles simpler contracts or constructor errors
                    deployedCode = initCode;
                    if (!result.success) {
                        LogPrintf("HTH-EVM: Constructor execution failed: %s, using init code\n", result.error);
                    } else {
                        LogPrintf("HTH-EVM: Constructor returned empty, using init code\n");
                    }
                }

                // Store the deployed contract code
                if (SetContractCode(contractAddr, deployedCode)) {
                    LogPrintf("HTH-EVM: Contract created at %s (txid=%s, vout=%d, codesize=%d, height=%d)\n",
                             HexStr(contractAddr), tx.GetHash().ToString(), i, deployedCode.size(), pindex->nHeight);
                } else {
                    LogPrintf("HTH-EVM: Failed to store contract code for %s\n", HexStr(contractAddr));
                }
            }

            if (script.HasOpCall()) {
                // Parse the OP_CALL script
                uint64_t version, gasLimit, gasPrice;
                std::vector<unsigned char> callData, contractAddr;

                if (!ParseContractScript(script, false, version, gasLimit, gasPrice, callData, contractAddr)) {
                    LogPrintf("HTH-EVM: Failed to parse OP_CALL script in tx %s\n", tx.GetHash().ToString());
                    continue;
                }

                // Check if contract exists
                if (!ContractExists(contractAddr)) {
                    LogPrintf("HTH-EVM: Contract %s does not exist\n", HexStr(contractAddr));
                    continue;
                }

                // Load contract code
                std::vector<unsigned char> code = GetContractCode(contractAddr);
                if (code.empty()) {
                    LogPrintf("HTH-EVM: Contract %s has no code\n", HexStr(contractAddr));
                    continue;
                }

                // Create writable storage interface
                BlockContractStorage storage(contractAddr);

                // Extract sender address from transaction
                std::vector<unsigned char> senderAddr = GetSenderAddress(tx, view);

                // Execute contract call with SimpleVM (writable mode)
                SimpleVM::VMContext ctx;
                ctx.contractAddress = contractAddr;
                ctx.callerAddress = senderAddr;
                ctx.originAddress = senderAddr;
                ctx.callData = callData;
                ctx.code = code;
                ctx.gasLimit = gasLimit;
                ctx.value = out.nValue;
                ctx.blockNumber = pindex->nHeight;
                ctx.timestamp = pindex->nTime;
                ctx.gasPrice = gasPrice;
                ctx.storage = &storage;
                ctx.readOnly = false; // Allow state changes

                SimpleVM::ExecutionResult result = SimpleVM::Execute(ctx);

                if (result.success) {
                    LogPrintf("HTH-EVM: Contract call to %s succeeded (gasUsed=%d, outputSize=%d)\n",
                             HexStr(contractAddr), result.gasUsed, result.output.size());
                } else {
                    LogPrintf("HTH-EVM: Contract call to %s failed: %s (gasUsed=%d)\n",
                             HexStr(contractAddr), result.error, result.gasUsed);
                }
            }
        }
    }

    // Flush contract database changes
    if (pcontractdb) {
        pcontractdb->Flush();
    }

    return true;
}

bool InitializeEVMState(const std::string& path, bool fReset)
{
    // Initialize the contract database
    if (!InitContractDB(path)) {
        LogPrintf("HTH-EVM: Failed to initialize contract database\n");
        return false;
    }

    LogPrintf("HTH-EVM: State initialization complete at path: %s\n", path);
    return true;
}

void ShutdownEVMState()
{
    ShutdownContractDB();
    LogPrintf("HTH-EVM: State shutdown complete\n");
}

uint256 GetEVMStateRoot()
{
    // For now, return empty hash - proper state root requires Merkle Patricia Trie
    // This would be implemented with the full eth_client integration
    return uint256();
}

std::vector<unsigned char> GetContractCode(const std::vector<unsigned char>& address)
{
    if (!pcontractdb) {
        return std::vector<unsigned char>();
    }

    std::vector<unsigned char> code;
    if (pcontractdb->ReadCode(address, code)) {
        return code;
    }
    return std::vector<unsigned char>();
}

std::vector<unsigned char> GetContractStorage(const std::vector<unsigned char>& address,
                                               const std::vector<unsigned char>& key)
{
    if (!pcontractdb) {
        return std::vector<unsigned char>(32, 0);
    }

    std::vector<unsigned char> value;
    if (pcontractdb->ReadStorage(address, key, value)) {
        return value;
    }
    return std::vector<unsigned char>(32, 0);
}

bool SetContractCode(const std::vector<unsigned char>& address, const std::vector<unsigned char>& code)
{
    if (!pcontractdb) {
        return false;
    }
    return pcontractdb->WriteCode(address, code);
}

bool SetContractStorage(const std::vector<unsigned char>& address,
                        const std::vector<unsigned char>& key,
                        const std::vector<unsigned char>& value)
{
    if (!pcontractdb) {
        return false;
    }
    return pcontractdb->WriteStorage(address, key, value);
}

bool ContractExists(const std::vector<unsigned char>& address)
{
    if (!pcontractdb) {
        return false;
    }
    return pcontractdb->HasCode(address);
}

// Calculate contract address from txid and vout index
std::vector<unsigned char> CalculateContractAddress(const uint256& txid, uint32_t voutIndex)
{
    std::vector<unsigned char> txIdAndVout(txid.begin(), txid.end());

    // Append vout as little-endian 4 bytes
    std::vector<unsigned char> voutBytes(4);
    memcpy(voutBytes.data(), &voutIndex, sizeof(voutIndex));
    txIdAndVout.insert(txIdAndVout.end(), voutBytes.begin(), voutBytes.end());

    // SHA256 then RIPEMD160 to get 20-byte address
    std::vector<unsigned char> sha256Hash(32);
    CSHA256().Write(txIdAndVout.data(), txIdAndVout.size()).Finalize(sha256Hash.data());

    std::vector<unsigned char> contractAddress(20);
    CRIPEMD160().Write(sha256Hash.data(), sha256Hash.size()).Finalize(contractAddress.data());

    return contractAddress;
}
