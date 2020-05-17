// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "randomx_bbp.h"
#include "hash.h"

static std::map<int, randomx_cache*> rxcache;
static std::map<int, randomx_vm*> myvm;
static std::map<int, bool> fInitialized;
static std::map<int, bool> fBusy;
static std::map<int, uint256> msGlobalKey;

void init(uint256 uKey, int iThreadID)
{
	std::vector<unsigned char> hashKey = std::vector<unsigned char>(uKey.begin(), uKey.end());
	randomx_flags flags = randomx_get_flags();
	rxcache[iThreadID] = randomx_alloc_cache(flags);
	randomx_init_cache(rxcache[iThreadID], hashKey.data(), hashKey.size());
	myvm[iThreadID] = randomx_create_vm(flags, rxcache[iThreadID], NULL);
	fInitialized[iThreadID] = true;
	msGlobalKey[iThreadID] = uKey;
}

void destroy(int iThreadID)
{
	randomx_destroy_vm(myvm[iThreadID]);
	randomx_release_cache(rxcache[iThreadID]);
	fInitialized[iThreadID] = false;
	fBusy[iThreadID] = false;
}

uint256 RandomX_Hash(uint256 hash, uint256 uKey, int iThreadID)
{
		if (fInitialized[iThreadID] && msGlobalKey[iThreadID] != uKey)
		{
			destroy(iThreadID);
		}

		if (!fInitialized[iThreadID] || uKey != msGlobalKey[iThreadID])
		{
			init(uKey, iThreadID);
		}
		std::vector<unsigned char> hashIn = std::vector<unsigned char>(hash.begin(), hash.end());
		char *hashOut1 = (char*)malloc(RANDOMX_HASH_SIZE + 1);
		fBusy[iThreadID] = true;
		randomx_calculate_hash(myvm[iThreadID], hashIn.data(), hashIn.size(), hashOut1);
		std::vector<unsigned char> data1(hashOut1, hashOut1 + RANDOMX_HASH_SIZE);
		free(hashOut1);
		fBusy[iThreadID] = false;
		return uint256(data1);
}

uint256 RandomX_Hash(std::vector<unsigned char> data0, uint256 uKey, int iThreadID)
{
		if (fInitialized[iThreadID] && msGlobalKey[iThreadID] != uKey)
		{
			destroy(iThreadID);
		}

		if (!fInitialized[iThreadID] || uKey != msGlobalKey[iThreadID])
		{
			init(uKey, iThreadID);
		}
		char *hashOut0 = (char*)malloc(RANDOMX_HASH_SIZE + 1);
		fBusy[iThreadID] = true;
		randomx_calculate_hash(myvm[iThreadID], data0.data(), data0.size(), hashOut0);
		std::vector<unsigned char> data1(hashOut0, hashOut0 + RANDOMX_HASH_SIZE);
		free(hashOut0);
		fBusy[iThreadID] = false;
		return uint256(data1);
}


uint256 RandomX_Hash(std::vector<unsigned char> data0, std::vector<unsigned char> datakey)
{
	int iThreadID = 101;
	randomx_flags flags = randomx_get_flags();
	rxcache[iThreadID] = randomx_alloc_cache(flags);
	randomx_init_cache(rxcache[iThreadID], datakey.data(), datakey.size());
	myvm[iThreadID] = randomx_create_vm(flags, rxcache[iThreadID], NULL);
	char *hashOut0 = (char*)malloc(RANDOMX_HASH_SIZE + 1);
	randomx_calculate_hash(myvm[iThreadID], data0.data(), data0.size(), hashOut0);
	std::vector<unsigned char> data1(hashOut0, hashOut0 + RANDOMX_HASH_SIZE);
	free(hashOut0);
	randomx_destroy_vm(myvm[iThreadID]);
	randomx_release_cache(rxcache[iThreadID]);
	return uint256(data1);
}


uint256 RandomX_SlowHash(std::vector<unsigned char> data0, uint256 uKey)
{
	randomx_cache* rxc;
	randomx_vm* vm1;
	std::vector<unsigned char> hashKey = std::vector<unsigned char>(uKey.begin(), uKey.end());
	randomx_flags flags = randomx_get_flags();
	rxc = randomx_alloc_cache(flags);
	randomx_init_cache(rxc, hashKey.data(), hashKey.size());
	vm1 = randomx_create_vm(flags, rxc, NULL);
	char *hashOut0 = (char*)malloc(RANDOMX_HASH_SIZE + 1);
	randomx_calculate_hash(vm1, data0.data(), data0.size(), hashOut0);
	std::vector<unsigned char> data1(hashOut0, hashOut0 + RANDOMX_HASH_SIZE);
	randomx_destroy_vm(vm1);
	randomx_release_cache(rxc);
	return uint256(data1);
}
