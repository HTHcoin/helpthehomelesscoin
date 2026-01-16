// Copyright (c) 2024 The HTH Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "qtum/contractdb.h"
#include "util.h"

#include <boost/filesystem.hpp>

std::unique_ptr<CContractDB> pcontractdb;

CContractDB::CContractDB(const boost::filesystem::path& path, size_t nCacheSize, bool fMemory, bool fWipe)
    : db(path, nCacheSize, fMemory, fWipe)
{
}

// Contract code operations

bool CContractDB::WriteCode(const std::vector<unsigned char>& address, const std::vector<unsigned char>& code)
{
    return db.Write(std::make_pair(DB_CODE, address), code);
}

bool CContractDB::ReadCode(const std::vector<unsigned char>& address, std::vector<unsigned char>& code)
{
    return db.Read(std::make_pair(DB_CODE, address), code);
}

bool CContractDB::HasCode(const std::vector<unsigned char>& address)
{
    return db.Exists(std::make_pair(DB_CODE, address));
}

bool CContractDB::EraseCode(const std::vector<unsigned char>& address)
{
    return db.Erase(std::make_pair(DB_CODE, address));
}

// Contract storage operations

bool CContractDB::WriteStorage(const std::vector<unsigned char>& address,
                               const std::vector<unsigned char>& key,
                               const std::vector<unsigned char>& value)
{
    // Create combined key: address + storage key
    std::vector<unsigned char> combinedKey;
    combinedKey.insert(combinedKey.end(), address.begin(), address.end());
    combinedKey.insert(combinedKey.end(), key.begin(), key.end());
    return db.Write(std::make_pair(DB_STORAGE, combinedKey), value);
}

bool CContractDB::ReadStorage(const std::vector<unsigned char>& address,
                              const std::vector<unsigned char>& key,
                              std::vector<unsigned char>& value)
{
    std::vector<unsigned char> combinedKey;
    combinedKey.insert(combinedKey.end(), address.begin(), address.end());
    combinedKey.insert(combinedKey.end(), key.begin(), key.end());
    return db.Read(std::make_pair(DB_STORAGE, combinedKey), value);
}

bool CContractDB::EraseStorage(const std::vector<unsigned char>& address,
                               const std::vector<unsigned char>& key)
{
    std::vector<unsigned char> combinedKey;
    combinedKey.insert(combinedKey.end(), address.begin(), address.end());
    combinedKey.insert(combinedKey.end(), key.begin(), key.end());
    return db.Erase(std::make_pair(DB_STORAGE, combinedKey));
}

// Nonce operations

bool CContractDB::WriteNonce(const std::vector<unsigned char>& address, uint64_t nonce)
{
    return db.Write(std::make_pair(DB_NONCE, address), nonce);
}

bool CContractDB::ReadNonce(const std::vector<unsigned char>& address, uint64_t& nonce)
{
    return db.Read(std::make_pair(DB_NONCE, address), nonce);
}

// Balance operations

bool CContractDB::WriteBalance(const std::vector<unsigned char>& address, int64_t balance)
{
    return db.Write(std::make_pair(DB_BALANCE, address), balance);
}

bool CContractDB::ReadBalance(const std::vector<unsigned char>& address, int64_t& balance)
{
    return db.Read(std::make_pair(DB_BALANCE, address), balance);
}

// Batch operations

bool CContractDB::Flush()
{
    return db.Sync();
}

bool CContractDB::Sync()
{
    return db.Sync();
}

// Global initialization functions

bool InitContractDB(const std::string& path)
{
    try {
        pcontractdb.reset(new CContractDB(path + "/contracts", 1 << 22, false, false));
        LogPrintf("HTH-EVM: Contract database initialized at %s\n", path + "/contracts");
        return true;
    } catch (const std::exception& e) {
        LogPrintf("HTH-EVM: Failed to initialize contract database: %s\n", e.what());
        return false;
    }
}

void ShutdownContractDB()
{
    if (pcontractdb) {
        pcontractdb->Flush();
        pcontractdb.reset();
        LogPrintf("HTH-EVM: Contract database shutdown\n");
    }
}
