/**
 * Configure trusted remotes between deployed OFT contracts
 *
 * This script connects all deployed OFT contracts so they can communicate
 * via LayerZero. Each contract must trust the others for cross-chain transfers.
 *
 * Usage:
 *   npx hardhat run scripts/configure-trusted-remotes.js --network ethereum
 *
 * This will configure the contract on ethereum to trust all other deployed chains.
 * Run on each chain to complete the mesh.
 *
 * Environment variables:
 *   PRIVATE_KEY - Owner private key (must be contract owner)
 */

const hre = require("hardhat");
const { CHAINS, DEPLOYMENTS, getChainConfig } = require("./config");

// ABI for setTrustedRemoteAddress function
const OFT_ABI = [
  "function setTrustedRemoteAddress(uint16 _remoteChainId, bytes calldata _remoteAddress) external",
  "function trustedRemoteLookup(uint16) external view returns (bytes)",
  "function owner() external view returns (address)"
];

async function main() {
  const networkName = hre.network.name;
  console.log(`\n========================================`);
  console.log(`Configuring Trusted Remotes on ${networkName}`);
  console.log(`========================================\n`);

  const chainConfig = getChainConfig(networkName);
  const [signer] = await hre.ethers.getSigners();

  console.log(`Signer: ${signer.address}\n`);

  // Get local contract address
  let localAddress;
  if (chainConfig.isNative) {
    localAddress = DEPLOYMENTS.hth.proxyOFT || DEPLOYMENTS.hth.hthOFT;
  } else {
    localAddress = DEPLOYMENTS[networkName]?.oft;
  }

  if (!localAddress) {
    console.error(`Error: No deployment found for ${networkName}`);
    console.error(`Please update DEPLOYMENTS in scripts/config.js first.`);
    process.exit(1);
  }

  console.log(`Local contract: ${localAddress}`);

  // Connect to local contract
  const localOFT = new hre.ethers.Contract(localAddress, OFT_ABI, signer);

  // Verify ownership
  const owner = await localOFT.owner();
  if (owner.toLowerCase() !== signer.address.toLowerCase()) {
    console.error(`Error: Signer ${signer.address} is not the owner.`);
    console.error(`Contract owner: ${owner}`);
    process.exit(1);
  }

  console.log(`Ownership verified.\n`);

  // Configure trusted remotes for all other chains
  const results = [];

  for (const [chainKey, remoteChainConfig] of Object.entries(CHAINS)) {
    // Skip self
    if (chainKey === networkName || remoteChainConfig.networkName === networkName) {
      continue;
    }

    // Get remote contract address
    let remoteAddress;
    if (remoteChainConfig.isNative) {
      remoteAddress = DEPLOYMENTS.hth?.proxyOFT || DEPLOYMENTS.hth?.hthOFT;
    } else {
      remoteAddress = DEPLOYMENTS[remoteChainConfig.networkName]?.oft;
    }

    if (!remoteAddress) {
      console.log(`⏭️  Skipping ${remoteChainConfig.name} (not deployed)`);
      continue;
    }

    const lzChainId = remoteChainConfig.lzChainId;

    // Check if already configured
    const existingRemote = await localOFT.trustedRemoteLookup(lzChainId);
    if (existingRemote && existingRemote !== "0x") {
      console.log(`✓  ${remoteChainConfig.name} (${lzChainId}) already configured`);
      results.push({
        chain: remoteChainConfig.name,
        lzChainId,
        status: "already_configured"
      });
      continue;
    }

    // Set trusted remote
    console.log(`Setting trusted remote for ${remoteChainConfig.name} (${lzChainId})...`);
    console.log(`   Remote address: ${remoteAddress}`);

    try {
      const tx = await localOFT.setTrustedRemoteAddress(
        lzChainId,
        remoteAddress
      );
      console.log(`   TX: ${tx.hash}`);
      await tx.wait();
      console.log(`✅ ${remoteChainConfig.name} configured successfully\n`);

      results.push({
        chain: remoteChainConfig.name,
        lzChainId,
        remoteAddress,
        txHash: tx.hash,
        status: "configured"
      });
    } catch (error) {
      console.error(`❌ Failed to configure ${remoteChainConfig.name}: ${error.message}\n`);
      results.push({
        chain: remoteChainConfig.name,
        lzChainId,
        status: "failed",
        error: error.message
      });
    }
  }

  console.log(`\n========================================`);
  console.log(`Configuration Summary`);
  console.log(`========================================\n`);

  const configured = results.filter(r => r.status === "configured").length;
  const alreadyConfigured = results.filter(r => r.status === "already_configured").length;
  const failed = results.filter(r => r.status === "failed").length;

  console.log(`Newly configured: ${configured}`);
  console.log(`Already configured: ${alreadyConfigured}`);
  console.log(`Failed: ${failed}`);

  if (failed > 0) {
    console.log(`\nFailed chains:`);
    results.filter(r => r.status === "failed").forEach(r => {
      console.log(`  - ${r.chain}: ${r.error}`);
    });
  }

  console.log(`\nNext steps:`);
  console.log(`1. Run this script on all other deployed chains`);
  console.log(`2. Test bridging with a small amount first`);
}

main()
  .then(() => process.exit(0))
  .catch((error) => {
    console.error(error);
    process.exit(1);
  });
