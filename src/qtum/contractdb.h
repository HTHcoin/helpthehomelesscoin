// Copyright (c) 2024 The HTH Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef HTH_CONTRACTDB_H
#define HTH_CONTRACTDB_H

#include "dbwrapper.h"
#include "uint256.h"
#include "serialize.h"

#include <boost/filesystem.hpp>
#include <string>
#include <vector>
#include <memory>

/**
 * Simple contract database for storing:
 * - Contract bytecode (address -> code)
 * - Contract storage (address + key -> value)
 * - Account nonces (address -> nonce)
 * - Account balances (address -> balance)
 */
class CContractDB
{
private:
    CDBWrapper db;

    // Database key prefixes
    static const char DB_CODE = 'C';        // Contract code
    static const char DB_STORAGE = 'S';     // Contract storage
    static const char DB_NONCE = 'N';       // Account nonces
    static const char DB_BALANCE = 'B';     // Account balances

public:
    CContractDB(const boost::filesystem::path& path, size_t nCacheSize, bool fMemory = false, bool fWipe = false);

    // Contract code operations
    bool WriteCode(const std::vector<unsigned char>& address, const std::vector<unsigned char>& code);
    bool ReadCode(const std::vector<unsigned char>& address, std::vector<unsigned char>& code);
    bool HasCode(const std::vector<unsigned char>& address);
    bool EraseCode(const std::vector<unsigned char>& address);

    // Contract storage operations
    bool WriteStorage(const std::vector<unsigned char>& address,
                      const std::vector<unsigned char>& key,
                      const std::vector<unsigned char>& value);
    bool ReadStorage(const std::vector<unsigned char>& address,
                     const std::vector<unsigned char>& key,
                     std::vector<unsigned char>& value);
    bool EraseStorage(const std::vector<unsigned char>& address,
                      const std::vector<unsigned char>& key);

    // Nonce operations
    bool WriteNonce(const std::vector<unsigned char>& address, uint64_t nonce);
    bool ReadNonce(const std::vector<unsigned char>& address, uint64_t& nonce);

    // Balance operations
    bool WriteBalance(const std::vector<unsigned char>& address, int64_t balance);
    bool ReadBalance(const std::vector<unsigned char>& address, int64_t& balance);

    // Batch operations
    bool Flush();
    bool Sync();
};

// Global contract database instance
extern std::unique_ptr<CContractDB> pcontractdb;

// Initialize and shutdown functions
bool InitContractDB(const std::string& path);
void ShutdownContractDB();

#endif // HTH_CONTRACTDB_H
