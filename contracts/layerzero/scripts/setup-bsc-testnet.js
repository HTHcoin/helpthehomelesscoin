/**
 * Configure trusted remotes for BSC Testnet
 * Sets up bidirectional trust with Sepolia and Arbitrum Sepolia
 */

const hre = require("hardhat");
const { CHAINS, DEPLOYMENTS } = require("./config");

const OFT_ABI = [
  "function setTrustedRemoteAddress(uint16 _remoteChainId, bytes calldata _remoteAddress) external",
  "function trustedRemoteLookup(uint16) external view returns (bytes)",
  "function owner() external view returns (address)"
];

async function setupTrustedRemote(provider, contractAddress, remoteLzChainId, remoteAddress, chainName) {
  const wallet = new hre.ethers.Wallet(process.env.PRIVATE_KEY, provider);
  const contract = new hre.ethers.Contract(contractAddress, OFT_ABI, wallet);

  // Check if already configured
  const existing = await contract.trustedRemoteLookup(remoteLzChainId);
  if (existing && existing !== "0x") {
    console.log(`  ✓ Already trusts ${chainName}`);
    return;
  }

  console.log(`  Setting trusted remote for ${chainName} (LZ ID: ${remoteLzChainId})...`);
  const tx = await contract.setTrustedRemoteAddress(remoteLzChainId, remoteAddress);
  console.log(`  TX: ${tx.hash}`);
  await tx.wait();
  console.log(`  ✅ Done`);
}

async function main() {
  console.log(`\n========================================`);
  console.log(`Setting up BSC Testnet trusted remotes`);
  console.log(`========================================\n`);

  const bscProvider = new hre.ethers.JsonRpcProvider("https://data-seed-prebsc-1-s1.binance.org:8545/");
  const sepoliaProvider = new hre.ethers.JsonRpcProvider("https://ethereum-sepolia-rpc.publicnode.com");
  const arbSepoliaProvider = new hre.ethers.JsonRpcProvider("https://sepolia-rollup.arbitrum.io/rpc");

  const bscOFT = DEPLOYMENTS.bscTestnet.oft;
  const sepoliaOFT = DEPLOYMENTS.sepolia.oft;
  const arbSepoliaOFT = DEPLOYMENTS.arbitrumSepolia.oft;

  // 1. BSC Testnet trusts Sepolia and Arbitrum Sepolia
  console.log(`1. Configuring BSC Testnet (${bscOFT}):`);
  await setupTrustedRemote(bscProvider, bscOFT, CHAINS.sepolia.lzChainId, sepoliaOFT, "Sepolia");
  await setupTrustedRemote(bscProvider, bscOFT, CHAINS.arbitrumSepolia.lzChainId, arbSepoliaOFT, "Arbitrum Sepolia");

  // 2. Sepolia trusts BSC Testnet
  console.log(`\n2. Configuring Sepolia (${sepoliaOFT}):`);
  await setupTrustedRemote(sepoliaProvider, sepoliaOFT, CHAINS.bscTestnet.lzChainId, bscOFT, "BSC Testnet");

  // 3. Arbitrum Sepolia trusts BSC Testnet
  console.log(`\n3. Configuring Arbitrum Sepolia (${arbSepoliaOFT}):`);
  await setupTrustedRemote(arbSepoliaProvider, arbSepoliaOFT, CHAINS.bscTestnet.lzChainId, bscOFT, "BSC Testnet");

  console.log(`\n========================================`);
  console.log(`All trusted remotes configured!`);
  console.log(`========================================`);
  console.log(`\nBSC Testnet is now connected to:`);
  console.log(`  - Sepolia (Native)`);
  console.log(`  - Arbitrum Sepolia`);
}

main()
  .then(() => process.exit(0))
  .catch((error) => {
    console.error(error);
    process.exit(1);
  });
