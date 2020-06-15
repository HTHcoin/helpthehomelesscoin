// Copyright (c) 2014-2019 The Dash Core Developers, The DAC Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "smartcontract-server.h"
#include "util.h"
#include "utilmoneystr.h"
#include "rpcpog.h"
#include "rpcpodc.h"
#include "smartcontract-client.h"
#include "init.h"
#include "activemasternode.h"
#include "governance-classes.h"
#include "governance.h"
#include "masternode-sync.h"
#include "masternode-payments.h"
#include "messagesigner.h"
#include "spork.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/case_conv.hpp> // for to_lower()
#include <boost/algorithm/string.hpp> // for trim(), and case insensitive compare
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
#include <cmath>
#ifdef ENABLE_WALLET
extern CWallet* pwalletMain;
#endif // ENABLE_WALLET



//////////////////////////////////////////////////////////////////////////////// Watchman-On-The-Wall /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//								                          			DAC's version of The Sentinel, March 31st, 2019                                                                                                  //

//                                                                                                                                                                                                                   //



std::string GetCPIDByCPK(std::string sCPK)
{
	std::string sData = ReadCache("CPK-WCG", sCPK);
	std::vector<std::string> vP = Split(sData.c_str(), "|");
	if (vP.size() < 10)
		return std::string();
	std::string cpid = vP[8];
	return cpid;
}

DACProposal GetProposalByHash(uint256 govObj, int nLastSuperblock)
{
	int nMinPassing = 500 * .10;
	if (nMinPassing < 1) nMinPassing = 1;
	CGovernanceObject* myGov = governance.FindGovernanceObject(govObj);
	UniValue obj = myGov->GetJSONObject();
	DACProposal dacProposal;
	dacProposal.sName = obj["name"].getValStr();
	dacProposal.nStartEpoch = cdbl(obj["start_epoch"].getValStr(), 0);
	dacProposal.nEndEpoch = cdbl(obj["end_epoch"].getValStr(), 0);
	dacProposal.sURL = obj["url"].getValStr();
	dacProposal.sExpenseType = obj["expensetype"].getValStr();
	dacProposal.nAmount = cdbl(obj["payment_amount"].getValStr(), 2);
	dacProposal.sAddress = obj["payment_address"].getValStr();
	dacProposal.uHash = myGov->GetHash();
	dacProposal.nHeight = GetHeightByEpochTime(dacProposal.nStartEpoch);
	dacProposal.nMinPassing = 1;
	dacProposal.nYesVotes = myGov->GetYesCount(VOTE_SIGNAL_FUNDING);
	dacProposal.nNoVotes = myGov->GetNoCount(VOTE_SIGNAL_FUNDING);
	dacProposal.nAbstainVotes = myGov->GetAbstainCount(VOTE_SIGNAL_FUNDING);
	dacProposal.nNetYesVotes = myGov->GetAbsoluteYesCount(VOTE_SIGNAL_FUNDING);
	dacProposal.nLastSuperblock = nLastSuperblock;
	dacProposal.sProposalHRTime = TimestampToHRDate(dacProposal.nStartEpoch);
	dacProposal.fPassing = dacProposal.nNetYesVotes >= nMinPassing;
	dacProposal.fIsPaid = dacProposal.nHeight < nLastSuperblock;
	return dacProposal;
}

std::string DescribeProposal(DACProposal dacProposal)
{
	std::string sReport = "Proposal StartDate: " + dacProposal.sProposalHRTime + ", Hash: " + dacProposal.uHash.GetHex() + " for Amount: " + RoundToString(dacProposal.nAmount, 2) + "IMAGECOIN, Name: "
				+ dacProposal.sName + ", ExpType: " + dacProposal.sExpenseType + ", PAD: " + dacProposal.sAddress 
				+ ", Height: " + RoundToString(dacProposal.nHeight, 0) 
				+ ", Votes: " + RoundToString(dacProposal.nNetYesVotes, 0) + ", LastSB: " 
				+ RoundToString(dacProposal.nLastSuperblock, 0);
	return sReport;
}

std::string GetCPIDElementByData(std::string sData, int iElement)
{
	std::vector<std::string> vP = Split(sData.c_str(), "|");
	if (vP.size() < 10)
		return std::string();
	return vP[iElement];
}

