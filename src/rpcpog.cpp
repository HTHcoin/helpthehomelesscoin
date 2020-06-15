// Copyright (c) 2014-2019 The Dash Core Developers, The DAC Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpcpog.h"
#include "spork.h"
#include "util.h"
#include "utilmoneystr.h"
#include "rpcpodc.h"
#include "init.h"
#include "bbpsocket.h"
#include "activemasternode.h"
#include "governance.h"
#include "governance-vote.h"
#include "governance-classes.h"
#include "governance-validators.h"
#include "masternode-utils.h"
#include "masternode-sync.h"
#include "masternodeconfig.h"
#include "masternodeman.h"
#include "masternode-payments.h"
#include "messagesigner.h"
#include "smartcontract-server.h"
#include "smartcontract-client.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/case_conv.hpp> // for to_lower()
#include <boost/algorithm/string.hpp> // for trim()
#include <boost/date_time/posix_time/posix_time.hpp> // for StringToUnixTime()
#include <math.h>       /* round, floor, ceil, trunc */
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <openssl/crypto.h>
#include <stdint.h>
#include <univalue.h>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <openssl/md5.h>
#include "txmempool.h"
// For HTTPS (for the pool communication)
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <boost/asio.hpp>
#include "net.h" // for CService
#include "netaddress.h"
#include "netbase.h" // for LookupHost
#include "wallet/wallet.h"
#include <sstream>
//#include "randomx.h"

#ifdef ENABLE_WALLET
extern CWallet* pwalletMain;

#endif // ENABLE_WALLET

CValidationState state;

UniValue VoteWithMasternodes(const std::map<uint256, CKey>& keys,	
                             const uint256& hash, vote_signal_enum_t eVoteSignal,	
                             vote_outcome_enum_t eVoteOutcome);

std::string GenerateNewAddress(std::string& sError, std::string sName)
{
    LOCK2(cs_main, pwalletMain->cs_wallet);
	{
		if (!pwalletMain->IsLocked(true))
			pwalletMain->TopUpKeyPool();
		// Generate a new key that is added to wallet
		CPubKey newKey;
		if (!pwalletMain->GetKeyFromPool(newKey, false))
		{
			sError = "Keypool ran out, please call keypoolrefill first";
			return std::string();
		}
		CKeyID keyID = newKey.GetID();
		pwalletMain->SetAddressBook(keyID, sName, "receive"); //receive == visible in address book, hidden = non-visible
		LogPrintf(" created new address %s ", CBitcoinAddress(keyID).ToString().c_str());
		return CBitcoinAddress(keyID).ToString();
	}
}

std::string GJE(std::string sKey, std::string sValue, bool bIncludeDelimiter, bool bQuoteValue)
{
	// This is a helper for the Governance gobject create method
	std::string sQ = "\"";
	std::string sOut = sQ + sKey + sQ + ":";
	if (bQuoteValue)
	{
		sOut += sQ + sValue + sQ;
	}
	else
	{
		sOut += sValue;
	}
	if (bIncludeDelimiter) sOut += ",";
	return sOut;
}

std::string RoundToString(double d, int place)
{
	std::ostringstream ss;
    ss << std::fixed << std::setprecision(place) << d ;
    return ss.str() ;
}

double Round(double d, int place)
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(place) << d ;
	double r = 0;
	try
	{
		r = boost::lexical_cast<double>(ss.str());
		return r;
	}
	catch(boost::bad_lexical_cast const& e)
	{
		LogPrintf("caught bad lexical cast %f", 1);
		return 0;
	}
	catch(...)
	{
		LogPrintf("caught bad lexical cast %f", 2);
	}
	return r;
}

std::vector<std::string> Split(std::string s, std::string delim)
{
	size_t pos = 0;
	std::string token;
	std::vector<std::string> elems;
	while ((pos = s.find(delim)) != std::string::npos)
	{
		token = s.substr(0, pos);
		elems.push_back(token);
		s.erase(0, pos + delim.length());
	}
	elems.push_back(s);
	return elems;
}

double cdbl(std::string s, int place)
{
	if (s=="") s = "0";
	if (s.length() > 255) return 0;
	s = strReplace(s, "\r","");
	s = strReplace(s, "\n","");
	std::string t = "";
	for (int i = 0; i < (int)s.length(); i++)
	{
		std::string u = s.substr(i,1);
		if (u=="0" || u=="1" || u=="2" || u=="3" || u=="4" || u=="5" || u=="6" || u == "7" || u=="8" || u=="9" || u=="." || u=="-") 
		{
			t += u;
		}
	}
	double r= 0;
	try
	{
	    r = boost::lexical_cast<double>(t);
	}
	catch(boost::bad_lexical_cast const& e)
	{
		LogPrintf("caught cdbl bad lexical cast %f from %s with %f", 1, s, (double)place);
		return 0;
	}
	catch(...)
	{
		LogPrintf("caught cdbl bad lexical cast %f", 2);
	}
	double d = Round(r, place);
	return d;
}

bool Contains(std::string data, std::string instring)
{
	std::size_t found = 0;
	found = data.find(instring);
	if (found != std::string::npos) return true;
	return false;
}

std::string GetElement(std::string sIn, std::string sDelimiter, int iPos)
{
	if (sIn.empty())
		return std::string();
	std::vector<std::string> vInput = Split(sIn.c_str(), sDelimiter);
	if (iPos < (int)vInput.size())
	{
		return vInput[iPos];
	}
	return std::string();
}

std::string GetSporkValue(std::string sKey)
{
	boost::to_upper(sKey);
    std::pair<std::string, int64_t> v = mvApplicationCache[std::make_pair("SPORK", sKey)];
	return v.first;
}

double GetSporkDouble(std::string sName, double nDefault)
{
	double dSetting = cdbl(GetSporkValue(sName), 2);
	if (dSetting == 0) return nDefault;
	return dSetting;
}

std::map<std::string, std::string> GetSporkMap(std::string sPrimaryKey, std::string sSecondaryKey)
{
	boost::to_upper(sPrimaryKey);
	boost::to_upper(sSecondaryKey);
	std::string sDelimiter = "|";
    std::pair<std::string, int64_t> v = mvApplicationCache[std::make_pair(sPrimaryKey, sSecondaryKey)];
	std::vector<std::string> vSporks = Split(v.first, sDelimiter);
	std::map<std::string, std::string> mSporkMap;
	for (int i = 0; i < vSporks.size(); i++)
	{
		std::string sMySpork = vSporks[i];
		if (!sMySpork.empty())
			mSporkMap.insert(std::make_pair(sMySpork, RoundToString(i, 0)));
	}
	return mSporkMap;
}

std::string Left(std::string sSource, int bytes)
{
	if (sSource.length() >= bytes)
	{
		return sSource.substr(0, bytes);
	}
	return std::string();
}	

bool CheckStakeSignature(std::string sBitcoinAddress, std::string sSignature, std::string strMessage, std::string& strError)
{
	CBitcoinAddress addr2(sBitcoinAddress);
	if (!addr2.IsValid()) 
	{
		strError = "Invalid address";
		return false;
	}
	CKeyID keyID2;
	if (!addr2.GetKeyID(keyID2)) 
	{
		strError = "Address does not refer to key";
		return false;
	}
	bool fInvalid = false;
	std::vector<unsigned char> vchSig2 = DecodeBase64(sSignature.c_str(), &fInvalid);
	if (fInvalid)
	{
		strError = "Malformed base64 encoding";
		return false;
	}
	CHashWriter ss2(SER_GETHASH, 0);
	ss2 << strMessageMagic;
	ss2 << strMessage;
	CPubKey pubkey2;
    if (!pubkey2.RecoverCompact(ss2.GetHash(), vchSig2)) 
	{
		strError = "Unable to recover public key.";
		return false;
	}
	bool fSuccess = (pubkey2.GetID() == keyID2);
	return fSuccess;
}


CPK GetCPK(std::string sData)
{
	// CPK DATA FORMAT: sCPK + "|" + Sanitized NickName + "|" + LockTime + "|" + SecurityHash + "|" + CPK Signature + "|" + Email + "|" + VendorType + "|" + OptData
	CPK k;
	std::vector<std::string> vDec = Split(sData.c_str(), "|");
	if (vDec.size() < 5) return k;
	std::string sSecurityHash = vDec[3];
	std::string sSig = vDec[4];
	std::string sCPK = vDec[0];
	if (sCPK.empty()) return k;
	if (vDec.size() >= 6)
		k.sEmail = vDec[5];
	if (vDec.size() >= 7)
		k.sVendorType = vDec[6];
	if (vDec.size() >= 8)
		k.sOptData = vDec[7];

	k.fValid = CheckStakeSignature(sCPK, sSig, sSecurityHash, k.sError);
	if (!k.fValid) 
	{
		LogPrintf("GetCPK::Error Sig %s, SH %s, Err %s, CPK %s, NickName %s ", sSig, sSecurityHash, k.sError, sCPK, vDec[1]);
		return k;
	}

	k.sAddress = sCPK;
	k.sNickName = vDec[1];
	k.nLockTime = (int64_t)cdbl(vDec[2], 0);

	return k;

} 

std::map<std::string, CPK> GetChildMap(std::string sGSCObjType)
{
	std::map<std::string, CPK> mCPKMap;
	boost::to_upper(sGSCObjType);
	int i = 0;
	for (auto ii : mvApplicationCache)
	{
		if (Contains(ii.first.first, sGSCObjType))
		{
			CPK k = GetCPK(ii.second.first);
			i++;
			mCPKMap.insert(std::make_pair(k.sAddress + "-" + RoundToString(i, 0), k));
		}
	}
	return mCPKMap;
}


std::map<std::string, CPK> GetGSCMap(std::string sGSCObjType, std::string sSearch, bool fRequireSig)
{
	std::map<std::string, CPK> mCPKMap;
	boost::to_upper(sGSCObjType);
	for (auto ii : mvApplicationCache)
	{
    	if (ii.first.first == sGSCObjType)
		{
			CPK k = GetCPK(ii.second.first);
			if (!k.sAddress.empty() && k.fValid)
			{
				if ((!sSearch.empty() && (sSearch == k.sAddress || sSearch == k.sNickName)) || sSearch.empty())
				{
					mCPKMap.insert(std::make_pair(k.sAddress, k));
				}
			}
		}
	}
	return mCPKMap;
}

CAmount CAmountFromValue(const UniValue& value)
{
    if (!value.isNum() && !value.isStr()) return 0;
    CAmount amount;
    if (!ParseFixedPoint(value.getValStr(), 8, &amount)) return 0;
    if (!MoneyRange(amount)) return 0;
    return amount;
}

static CCriticalSection csReadWait;
std::string ReadCache(std::string sSection, std::string sKey)
{
	LOCK(csReadWait);
	std::string sLookupSection = sSection;
	std::string sLookupKey = sKey;
	boost::to_upper(sLookupSection);
	boost::to_upper(sLookupKey);
	// NON-CRITICAL TODO : Find a way to eliminate this to_upper while we transition to non-financial transactions
	if (sLookupSection.empty() || sLookupKey.empty())
		return std::string();
	std::pair<std::string, int64_t> t = mvApplicationCache[std::make_pair(sLookupSection, sLookupKey)];
	return t.first;
}

std::string TimestampToHRDate(double dtm)
{
	if (dtm == 0) return "1-1-1970 00:00:00";
	if (dtm > 9888888888) return "1-1-2199 00:00:00";
	std::string sDt = DateTimeStrFormat("%m-%d-%Y %H:%M:%S",dtm);
	return sDt;
}

std::string ExtractXML(std::string XMLdata, std::string key, std::string key_end)
{
	std::string extraction = "";
	std::string::size_type loc = XMLdata.find( key, 0 );
	if( loc != std::string::npos )
	{
		std::string::size_type loc_end = XMLdata.find( key_end, loc+3);
		if (loc_end != std::string::npos )
		{
			extraction = XMLdata.substr(loc+(key.length()),loc_end-loc-(key.length()));
		}
	}
	return extraction;
}


std::string AmountToString(const CAmount& amount)
{
    bool sign = amount < 0;
    int64_t n_abs = (sign ? -amount : amount);
    int64_t quotient = n_abs / COIN;
    int64_t remainder = n_abs % COIN;
	std::string sAmount = strprintf("%s%d.%08d", sign ? "-" : "", quotient, remainder);
	return sAmount;
}

static CBlockIndex* pblockindexFBBHLast;
CBlockIndex* FindBlockByHeight(int nHeight)
{
    CBlockIndex *pblockindex;
	if (nHeight < 0 || nHeight > chainActive.Tip()->nHeight) return NULL;

    if (nHeight < chainActive.Tip()->nHeight / 2)
		pblockindex = mapBlockIndex[chainActive.Genesis()->GetBlockHash()];
    else
        pblockindex = chainActive.Tip();
    if (pblockindexFBBHLast && abs(nHeight - pblockindex->nHeight) > abs(nHeight - pblockindexFBBHLast->nHeight))
        pblockindex = pblockindexFBBHLast;
    while (pblockindex->nHeight > nHeight)
        pblockindex = pblockindex->pprev;
    while (pblockindex->nHeight < nHeight)
		pblockindex = chainActive.Next(pblockindex);
    pblockindexFBBHLast = pblockindex;
    return pblockindex;
}

std::string DefaultRecAddress(std::string sType)
{
	std::string sDefaultRecAddress;
	for (auto item : pwalletMain->mapAddressBook)
    {
        const CBitcoinAddress& address = item.first;
        std::string strName = item.second.name;
        bool fMine = IsMine(*pwalletMain, address.Get());
        if (fMine)
		{
		    sDefaultRecAddress=CBitcoinAddress(address).ToString();
			boost::to_upper(strName);
			boost::to_upper(sType);
			if (strName == sType) 
			{
				sDefaultRecAddress = CBitcoinAddress(address).ToString();
				return sDefaultRecAddress;
			}
		}
    }

	if (!sType.empty())
	{
		std::string sError;
		sDefaultRecAddress = GenerateNewAddress(sError, sType);
		if (sError.empty()) return sDefaultRecAddress;
	}
	
	return sDefaultRecAddress;
}


std::string RetrieveMd5(std::string s1)
{
	try
	{
		const char* chIn = s1.c_str();
		unsigned char digest2[16];
		MD5((unsigned char*)chIn, strlen(chIn), (unsigned char*)&digest2);
		char mdString2[33];
		for(int i = 0; i < 16; i++) sprintf(&mdString2[i*2], "%02x", (unsigned int)digest2[i]);
 		std::string xmd5(mdString2);
		return xmd5;
	}
    catch (std::exception &e)
	{
		return std::string();
	}
}


std::string PubKeyToAddress(const CScript& scriptPubKey)
{
	CTxDestination address1;
    ExtractDestination(scriptPubKey, address1);
    CBitcoinAddress address2(address1);
    return address2.ToString();
}    


void GetTxTimeAndAmountAndHeight(uint256 hashInput, int hashInputOrdinal, int64_t& out_nTime, CAmount& out_caAmount, int& out_height)
{
	CTransaction tx1;
	uint256 hashBlock1;
	if (GetTransaction(hashInput, tx1, Params().GetConsensus(), hashBlock1, true))
	{
		out_caAmount = tx1.vout[hashInputOrdinal].nValue;
		BlockMap::iterator mi = mapBlockIndex.find(hashBlock1);
		if (mi != mapBlockIndex.end())
		{
			CBlockIndex* pindexHistorical = mapBlockIndex[hashBlock1];              
			out_nTime = pindexHistorical->GetBlockTime();
			out_height = pindexHistorical->nHeight;
			return;
		}
		else
		{
			LogPrintf("\nUnable to find hashBlock %s", hashBlock1.GetHex().c_str());
		}
	}
	else
	{
		LogPrintf("\nUnable to find hashblock1 in GetTransaction %s ",hashInput.GetHex().c_str());
	}
}



CAmount GetRPCBalance()
{
	return pwalletMain->GetBalance();
}

bool RPCSendMoney(std::string& sError, const CTxDestination &address, CAmount nValue, bool fSubtractFeeFromAmount, CWalletTx& wtxNew, bool fUseInstantSend, std::string sOptionalData, double nCoinAge)
{
    CAmount curBalance = pwalletMain->GetBalance();

    // Check amount
    if (nValue <= 0)
	{
        sError = "Invalid amount";
		return false;
	}
	
	if (pwalletMain->IsLocked())
	{
		sError = "Wallet unlock required";
		return false;
	}

	if (nValue > curBalance)
	{
		sError = "Insufficient funds";
		return false;
	}
    // Parse address
    CScript scriptPubKey = GetScriptForDestination(address);

    // Create and send the transaction
    CReserveKey reservekey(pwalletMain);
    CAmount nFeeRequired;
    std::string strError;
    std::vector<CRecipient> vecSend;
    int nChangePosRet = -1;
	bool fForce = false;
    CRecipient recipient = {scriptPubKey, nValue, "", fSubtractFeeFromAmount};
	vecSend.push_back(recipient);
	
    int nMinConfirms = 0;
    if (!pwalletMain->CreateTransaction(vecSend, wtxNew, reservekey, nFeeRequired, nChangePosRet, strError, NULL, true,  ONLY_DENOMINATED, fUseInstantSend))
	{
        if (!fSubtractFeeFromAmount && nValue + nFeeRequired > pwalletMain->GetBalance())
		{
            sError = strprintf("Error: This transaction requires a transaction fee of at least %s because of its amount, complexity, or use of recently received funds!", FormatMoney(nFeeRequired));
			return false;
		}
		sError = "Unable to Create Transaction: " + strError;
		return false;
    }
    //CValidationState state;
        
    if (!pwalletMain->CommitTransaction(wtxNew, reservekey, g_connman.get(),  fUseInstantSend ? NetMsgType::TXLOCKREQUEST : NetMsgType::TX))
	{
        sError = "Error: The transaction was rejected! This might happen if some of the coins in your wallet were already spent, such as if you used a copy of wallet.dat and coins were spent in the copy but not marked as spent here.";
		return false;
	}
	return true;
}

CAmount R20(CAmount amount)
{
	double nAmount = amount / COIN; 
	nAmount = nAmount + 0.5 - (nAmount < 0); 
	int iAmount = (int)nAmount;
	return (iAmount * COIN);
}

double R2X(double var) 
{ 
    double value = (int)(var * 100 + .5); 
    return (double)value / 100; 
} 

double Quantize(double nFloor, double nCeiling, double nValue)
{
	double nSpan = nCeiling - nFloor;
	double nLevel = nSpan * nValue;
	double nOut = nFloor + nLevel;
	if (nOut > std::max(nFloor, nCeiling)) nOut = std::max(nFloor, nCeiling);
	if (nOut < std::min(nCeiling, nFloor)) nOut = std::min(nCeiling, nFloor);
	return nOut;
}

int GetHeightByEpochTime(int64_t nEpoch)
{
	if (!chainActive.Tip()) return 0;
	int nLast = chainActive.Tip()->nHeight;
	if (nLast < 1) return 0;
	for (int nHeight = nLast; nHeight > 0; nHeight--)
	{
		CBlockIndex* pindex = FindBlockByHeight(nHeight);
		if (pindex)
		{
			int64_t nTime = pindex->GetBlockTime();
			if (nEpoch > nTime) return nHeight;
		}
	}
	return -1;
}

void GetGovSuperblockHeights(int& nNextSuperblock, int& nLastSuperblock)
{
	
    int nBlockHeight = 0;
    {
        LOCK(cs_main);
        nBlockHeight = (int)chainActive.Height();
    }
    int nSuperblockStartBlock = Params().GetConsensus().nSuperblockStartBlock;
    int nSuperblockCycle = Params().GetConsensus().nSuperblockCycle;
    int nFirstSuperblockOffset = (nSuperblockCycle - nSuperblockStartBlock % nSuperblockCycle) % nSuperblockCycle;
    int nFirstSuperblock = nSuperblockStartBlock + nFirstSuperblockOffset;
    if(nBlockHeight < nFirstSuperblock)
	{
        nLastSuperblock = 0;
        nNextSuperblock = nFirstSuperblock;
    } else {
        nLastSuperblock = nBlockHeight - nBlockHeight % nSuperblockCycle;
        nNextSuperblock = nLastSuperblock + nSuperblockCycle;
    }
}

std::string GetActiveProposals()
{
    int nStartTime = GetAdjustedTime() - (86400 * 32);
    LOCK2(cs_main, governance.cs);
    std::vector<CGovernanceObject*> objs = governance.GetAllNewerThan(nStartTime);
	std::string sXML;
	int id = 0;
	std::string sDelim = "|";
	std::string sZero = "\0";
	int nLastSuperblock = 0;
	int nNextSuperblock = 0;
	GetGovSuperblockHeights(nNextSuperblock, nLastSuperblock);
	for (CGovernanceObject* pGovObj : objs)
    {
		if (pGovObj->GetObjectType() != GOVERNANCE_OBJECT_PROPOSAL) continue;
		int64_t nEpoch = 0;
		int64_t nStartEpoch = 0;
		CGovernanceObject* myGov = governance.FindGovernanceObject(pGovObj->GetHash());
		UniValue obj = myGov->GetJSONObject();
		std::string sURL;
		std::string sProposalType;
		nStartEpoch = cdbl(obj["start_epoch"].getValStr(), 0);
		nEpoch = cdbl(obj["end_epoch"].getValStr(), 0);
		sURL = obj["url"].getValStr();
		sProposalType = obj["expensetype"].getValStr();
		if (sProposalType.empty()) sProposalType = "N/A";
		DACProposal dProposal = GetProposalByHash(pGovObj->GetHash(), nLastSuperblock);
		std::string sHash = pGovObj->GetHash().GetHex();
		int nEpochHeight = GetHeightByEpochTime(nStartEpoch);
		// First ensure the proposals gov height has not passed yet
		bool bIsPaid = nEpochHeight < nLastSuperblock;
		std::string sReport = DescribeProposal(dProposal);
		if (fDebug)
			LogPrintf("\nGetActiveProposals::Proposal %s , epochHeight %f, nLastSuperblock %f, IsPaid %f ", 
					sReport, nEpochHeight, nLastSuperblock, (double)bIsPaid);
		if (!bIsPaid)
		{
			int iYes = pGovObj->GetYesCount(VOTE_SIGNAL_FUNDING);
			int iNo = pGovObj->GetNoCount(VOTE_SIGNAL_FUNDING);
			int iAbstain = pGovObj->GetAbstainCount(VOTE_SIGNAL_FUNDING);
			id++;
			if (sProposalType.empty()) sProposalType = "NA";
			std::string sProposalTime = TimestampToHRDate(nStartEpoch);
			if (id == 1) sURL += "&t=" + RoundToString(GetAdjustedTime(), 0);
			std::string sName;
			sName = obj["name"].getValStr();
			double dCharityAmount = 0;
			dCharityAmount = cdbl(obj["payment_amount"].getValStr(), 2);
			std::string sRow = "<proposal>" + sHash + sDelim 
				+ sName + sDelim 
				+ RoundToString(dCharityAmount, 2) + sDelim
				+ sProposalType + sDelim
				+ sProposalTime + sDelim
					+ RoundToString(iYes, 0) + sDelim
					+ RoundToString(iNo, 0) + sDelim + RoundToString(iAbstain,0) 
					+ sDelim + sURL;
				sXML += sRow;
		}
	}
	return sXML;
}

bool VoteManyForGobject(std::string govobj, std::string strVoteSignal, std::string strVoteOutcome,
	int iVotingLimit, int& nSuccessful, int& nFailed, std::string& sError)
{

	uint256 hash(uint256S(govobj));
	vote_signal_enum_t eVoteSignal = CGovernanceVoting::ConvertVoteSignal(strVoteSignal);
	if(eVoteSignal == VOTE_SIGNAL_NONE)
	{
		sError = "Invalid vote signal (funding).";
		return false;
	}
    vote_outcome_enum_t eVoteOutcome = CGovernanceVoting::ConvertVoteOutcome(strVoteOutcome);
    if(eVoteOutcome == VOTE_OUTCOME_NONE)
	{
        sError = "Invalid vote outcome (yes/no/abstain)";
		return false;
	}

#ifdef ENABLE_WALLET
    if (!pwalletMain)
    {
        sError = "Voting is not supported when wallet is disabled.";
        return false;
    }
#endif

	std::map<uint256, CKey> votingKeys;


//    auto mnList = deterministicMNManager->GetListAtChainTip();
//    mnList.ForEachMN(true, [&](const CDeterministicMNCPtr& dmn)
//	{
//        CKey votingKey;
//        if (pwalletMain->GetKey(dmn->pdmnState->keyIDVoting, votingKey))
//		{
//            votingKeys.emplace(dmn->proTxHash, votingKey);
//		}
//	});



	UniValue vOutcome;


	try
	{

		 std::vector<CMasternodeConfig::CMasternodeEntry> mnEntries;
		        mnEntries = masternodeConfig.getEntries();

		        UniValue resultsObj(UniValue::VOBJ);

		        BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
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
		                statusObj.push_back(Pair("errorMessage", exception.GetMessage()));
		            }

		            resultsObj.push_back(Pair(mne.getAlias(), statusObj));
		        }

		        UniValue returnObj(UniValue::VOBJ);
		        returnObj.push_back(Pair("overall", strprintf("Voted successfully %d time(s) and failed %d time(s).", nSuccessful, nFailed)));
		        returnObj.push_back(Pair("detail", resultsObj));
		        returnObj.push_back(Pair("success_count", nSuccessful));

		        vOutcome = returnObj;

		//vOutcome = VoteWithMasternodes(votingKeys, hash, eVoteSignal, eVoteOutcome);
	}
	catch(std::runtime_error& e)
	{
		sError = e.what();
		return false;
	}
	catch (...)
	{
		sError = "Voting failed.";
		return false;
	}

	nSuccessful = cdbl(vOutcome["success_count"].getValStr(), 0);
	bool fResult = nSuccessful > 0 ? true : false;
	return fResult;
}


std::string CreateGovernanceCollateral(uint256 GovObjHash, CAmount caFee, std::string& sError)
{
	CWalletTx wtx;
	if(!pwalletMain->GetBudgetSystemCollateralTX(wtx, GovObjHash, caFee, false)) 
	{
		sError = "Error creating collateral transaction for governance object.  Please check your wallet balance and make sure your wallet is unlocked.";
		return std::string();
	}
	if (sError.empty())
	{
		// -- make our change address
		CReserveKey reservekey(pwalletMain);
		//CValidationState state;
        pwalletMain->CommitTransaction(wtx, reservekey, g_connman.get(), NetMsgType::TX);
		DBG( cout << "gobject: prepare "
					<< " strData = " << govobj.GetDataAsString()
					<< ", hash = " << govobj.GetHash().GetHex()
					<< ", txidFee = " << wtx.GetHash().GetHex()
					<< endl; );
		return wtx.GetHash().ToString();
	}
	return std::string();
}

int GetNextSuperblock()
{
	int nLastSuperblock, nNextSuperblock;
    // Get current block height
    int nBlockHeight = 0;
    {
        LOCK(cs_main);
        nBlockHeight = (int)chainActive.Height();
    }

    // Get chain parameters
    int nSuperblockStartBlock = Params().GetConsensus().nSuperblockStartBlock;
    int nSuperblockCycle = Params().GetConsensus().nSuperblockCycle;

    // Get first superblock
    int nFirstSuperblockOffset = (nSuperblockCycle - nSuperblockStartBlock % nSuperblockCycle) % nSuperblockCycle;
    int nFirstSuperblock = nSuperblockStartBlock + nFirstSuperblockOffset;

    if(nBlockHeight < nFirstSuperblock)
	{
        nLastSuperblock = 0;
        nNextSuperblock = nFirstSuperblock;
    }
	else 
	{
        nLastSuperblock = nBlockHeight - nBlockHeight % nSuperblockCycle;
        nNextSuperblock = nLastSuperblock + nSuperblockCycle;
    }
	return nNextSuperblock;
}


bool SubmitProposalToNetwork(uint256 txidFee, int64_t nStartTime, std::string sHex, std::string& sError, std::string& out_sGovObj)
{
	if(!masternodeSync.IsBlockchainSynced())
	{
		sError = "Must wait for client to sync with masternode network. ";
		return false;
    }
    // ASSEMBLE NEW GOVERNANCE OBJECT FROM USER PARAMETERS
    uint256 hashParent = uint256();
    int nRevision = 1;
	CGovernanceObject govobj(hashParent, nRevision, nStartTime, txidFee, sHex);
    DBG( cout << "gobject: submit "
             << " strData = " << govobj.GetDataAsString()
             << ", hash = " << govobj.GetHash().GetHex()
             << ", txidFee = " << txidFee.GetHex()
             << endl; );

    std::string strHash = govobj.GetHash().ToString();
    if(!govobj.IsValidLocally(sError, true))
	{
		sError += "Object submission rejected because object is not valid.";
		LogPrintf("\n OBJECT REJECTED:\n gobject submit 0 1 %f %s %s \n", (double)nStartTime, sHex.c_str(), txidFee.GetHex().c_str());
		return false;
    }
    // RELAY THIS OBJECT - Reject if rate check fails but don't update buffer

	bool fRateCheckBypassed = false;
    if(!governance.MasternodeRateCheck(govobj, true, false, fRateCheckBypassed))
	{
        sError = "Object creation rate limit exceeded";
		return false;
	}
    //governance.AddSeenGovernanceObject(govobj.GetHash(), SEEN_OBJECT_IS_VALID);
    govobj.Relay(*g_connman);
    governance.AddGovernanceObject(govobj, *g_connman);
	out_sGovObj = govobj.GetHash().ToString();
	return true;
}


static double HTTP_PROTO_VERSION = 2.0;
//std::string HTTPSPost(bool bPost, int iThreadID, std::string sActionName, std::string sDistinctUser, std::string sPayload, std::string sBaseURL, std::string sPage, int iPort,
//	std::string sSolution, int iTimeoutSecs, int iMaxSize, int iBOE)
//{
//	std::string sData;
//	int iChunkSize = 1024;
//	if (iMaxSize > 512000)
//	{
//		sData.reserve(iMaxSize);
//		iChunkSize = 65536;
//	}
//
//	// The OpenSSL version of Post *only* works with SSL websites, hence the need for HTTPPost(2) (using BOOST).  The dev team is working on cleaning this up before the end of 2019 to have one standard version with cleaner code and less internal parts. //
//	try
//	{
//		double dDebugLevel = cdbl(GetArg("-devdebuglevel", "0"), 0);
//
//		std::map<std::string, std::string> mapRequestHeaders;
//		mapRequestHeaders["Miner"] = sDistinctUser;
//		mapRequestHeaders["Action"] = sPayload;
//		mapRequestHeaders["Solution"] = sSolution;
//		mapRequestHeaders["Agent"] = FormatFullVersion();
//		// Supported pool Network Chain modes: main, test, regtest
//		const CChainParams& chainparams = Params();
//		mapRequestHeaders["NetworkID"] = chainparams.NetworkIDString();
//		mapRequestHeaders["ThreadID"] = RoundToString(iThreadID, 0);
//		mapRequestHeaders["OS"] = sOS;
//
//		mapRequestHeaders["SessionID"] = msSessionID;
//		mapRequestHeaders["WorkerID1"] = GetArg("-workerid", "");
//		mapRequestHeaders["WorkerID2"] = GetArg("-workeridfunded", "");
//		mapRequestHeaders["HTTP_PROTO_VERSION"] = RoundToString(HTTP_PROTO_VERSION, 0);
//
//		BIO* bio;
//		SSL_CTX* ctx;
//		//   Registers the SSL/TLS ciphers and digests and starts the security layer.
//		SSL_library_init();
//		ctx = SSL_CTX_new(SSLv23_client_method());
//		if (ctx == NULL)
//		{
//			return "<ERROR>CTX_IS_NULL</ERROR>";
//		}
//		bio = BIO_new_ssl_connect(ctx);
//		std::string sDomain = GetDomainFromURL(sBaseURL);
//		std::string sDomainWithPort = sDomain + ":" + RoundToString(iPort, 0);
//
//		// Compatibility with strict d-dos prevention rules (like cloudflare)
//		SSL * ssl(nullptr);
//		BIO_get_ssl(bio, &ssl);
//		SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
//		SSL_set_tlsext_host_name(ssl, const_cast<char *>(sDomain.c_str()));
//		BIO_set_conn_int_port(bio, &iPort);
//
//		BIO_set_conn_hostname(bio, sDomainWithPort.c_str());
//		if(BIO_do_connect(bio) <= 0)
//		{
//			return "<ERROR>Failed connection to " + sDomainWithPort + "</ERROR>";
//		}
//
//		if (sDomain.empty()) return "<ERROR>DOMAIN_MISSING</ERROR>";
//		// Evo requires 2 args instead of 3, the last used to be true for DNS resolution=true
//
//		CNetAddr cnaMyHost;
//		LookupHost(sDomain.c_str(), cnaMyHost, true);
// 	    CService addrConnect = CService(cnaMyHost, 443);
//
//		if (!addrConnect.IsValid())
//		{
//  			return "<ERROR>DNS_ERROR</ERROR>";
//		}
//		std::string sPost = PrepareHTTPPost(bPost, sPage, sDomain, sPayload, mapRequestHeaders);
//		if (dDebugLevel == 1)
//			LogPrintf("Trying connection to %s ", sPost);
//		const char* write_buf = sPost.c_str();
//		if(BIO_write(bio, write_buf, strlen(write_buf)) <= 0)
//		{
//			return "<ERROR>FAILED_HTTPS_POST</ERROR>";
//		}
//		//  Variables used to read the response from the server
//		int size;
//		clock_t begin = clock();
//		char buf[65536];
//		for(;;)
//		{
//			//  Get chunks of the response
//			size = BIO_read(bio, buf, 65535);
//			if(size <= 0)
//			{
//				break;
//			}
//			buf[size] = 0;
//			std::string MyData(buf);
//			sData += MyData;
//			clock_t end = clock();
//			double elapsed_secs = double(end - begin) / (CLOCKS_PER_SEC + .01);
//			if (elapsed_secs > iTimeoutSecs) break;
//			if (TermPeekFound(sData, iBOE)) break;
//
//			if (sData.find("Content-Length:") != std::string::npos)
//			{
//				double dMaxSize = cdbl(ExtractXML(sData,"Content-Length: ","\n"),0);
//				std::size_t foundPos = sData.find("Content-Length:");
//				if (dMaxSize > 0)
//				{
//					iMaxSize = dMaxSize + (int)foundPos + 16;
//				}
//			}
//			if ((int)sData.size() >= (iMaxSize-1)) break;
//		}
//		// R ANDREW - JAN 4 2018: Free bio resources
//		BIO_free_all(bio);
//		if (dDebugLevel == 1)
//			LogPrintf("Received %s ", sData);
//		return sData;
//	}
//	catch (std::exception &e)
//	{
//        return "<ERROR>WEB_EXCEPTION</ERROR>";
//    }
//	catch (...)
//	{
//		return "<ERROR>GENERAL_WEB_EXCEPTION</ERROR>";
//	}
//}
