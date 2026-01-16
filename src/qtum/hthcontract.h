// Copyright (c) 2019-2024 The HTH Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef HTH_QTUM_HTHCONTRACT_H
#define HTH_QTUM_HTHCONTRACT_H

#include "primitives/transaction.h"
#include "uint256.h"

#include <vector>
#include <memory>

// Forward declarations
class CBlock;
class CBlockIndex;
class CCoinsViewCache;
class CValidationState;

namespace Consensus {
    struct Params;
}

/**
 * HTH EVM Contract Interface
 *
 * This interface bridges HTH's UTXO-based validation with the EVM execution engine.
 * Contracts are executed during block connection and results are committed to state.
 */

// Hard fork activation height for EVM support
extern int nEVMStartHeight;

// Check if EVM is active at given height
bool IsEVMEnabled(int nHeight, const Consensus::Params& params);

/**
 * Check if a transaction contains contract operations
 * Returns true if any output has OP_CREATE or OP_CALL
 */
bool HasContractOps(const CTransaction& tx);

/**
 * Validate contract transaction format
 * Checks that contract transactions have proper structure:
 * - OP_CREATE: version, gasLimit, gasPrice, data, OP_CREATE
 * - OP_CALL: version, gasLimit, gasPrice, data, contractAddr, OP_CALL
 */
bool CheckContractTx(const CTransaction& tx, CValidationState& state, int nHeight);

/**
 * Execute all contract transactions in a block
 * This is called during ConnectBlock after all regular validations pass.
 * Contract execution results are applied to the global EVM state.
 *
 * @param block The block containing contract transactions
 * @param pindex Block index for the block
 * @param view Coins view for UTXO lookups
 * @param state Validation state for error reporting
 * @return true if all contract executions succeeded
 */
bool ExecuteBlockContracts(const CBlock& block, const CBlockIndex* pindex,
                           CCoinsViewCache& view, CValidationState& state);

/**
 * Initialize the global EVM state
 * Called during startup to load or create the EVM state database
 *
 * @param path Path to the EVM state database
 * @param fReset If true, reset the state to empty
 * @return true if initialization succeeded
 */
bool InitializeEVMState(const std::string& path, bool fReset);

/**
 * Shutdown the EVM state
 * Called during shutdown to properly close the state database
 */
void ShutdownEVMState();

/**
 * Get the current EVM state root hash
 * This is included in block headers after the hard fork activation
 */
uint256 GetEVMStateRoot();

/**
 * Get contract bytecode at address
 *
 * @param address Contract address (20 bytes)
 * @return Contract bytecode, empty if not found
 */
std::vector<unsigned char> GetContractCode(const std::vector<unsigned char>& address);

/**
 * Get contract storage value at key
 *
 * @param address Contract address
 * @param key Storage key (32 bytes)
 * @return Storage value (32 bytes), zeros if not found
 */
std::vector<unsigned char> GetContractStorage(const std::vector<unsigned char>& address,
                                               const std::vector<unsigned char>& key);

/**
 * Set contract bytecode at address
 *
 * @param address Contract address (20 bytes)
 * @param code Contract bytecode
 * @return true if successful
 */
bool SetContractCode(const std::vector<unsigned char>& address, const std::vector<unsigned char>& code);

/**
 * Set contract storage value at key
 *
 * @param address Contract address
 * @param key Storage key (32 bytes)
 * @param value Storage value (32 bytes)
 * @return true if successful
 */
bool SetContractStorage(const std::vector<unsigned char>& address,
                        const std::vector<unsigned char>& key,
                        const std::vector<unsigned char>& value);

/**
 * Check if a contract exists at address
 *
 * @param address Contract address (20 bytes)
 * @return true if contract exists
 */
bool ContractExists(const std::vector<unsigned char>& address);

/**
 * Calculate contract address from transaction ID and vout index
 *
 * @param txid Transaction hash
 * @param voutIndex Index of the vout creating the contract
 * @return Contract address (20 bytes)
 */
std::vector<unsigned char> CalculateContractAddress(const uint256& txid, uint32_t voutIndex);

#endif // HTH_QTUM_HTHCONTRACT_H
