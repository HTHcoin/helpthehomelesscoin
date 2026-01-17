/**
 * Configure trusted remotes between Sepolia and Arbitrum Sepolia
 */

const hre = require("hardhat");
const { CHAINS, DEPLOYMENTS } = require("./config");

const OFT_ABI = [
  "function setTrustedRemoteAddress(uint16 _remoteChainId, bytes calldata _remoteAddress) external",
  "function trustedRemoteLookup(uint16) external view returns (bytes)",
  "function owner() external view returns (address)"
];

async function main() {
  const networkName = hre.network.name;
  const [signer] = await hre.ethers.getSigners();

  console.log(`\n========================================`);
  console.log(`Setting up trusted remotes on ${networkName}`);
  console.log(`========================================\n`);
  console.log(`Signer: ${signer.address}\n`);

  // Determine local and remote based on network
  let localAddress, remoteAddress, remoteLzChainId, remoteName;

  if (networkName === "sepolia") {
    localAddress = DEPLOYMENTS.sepolia.oft;
    remoteAddress = DEPLOYMENTS.arbitrumSepolia.oft;
    remoteLzChainId = CHAINS.arbitrumSepolia.lzChainId;
    remoteName = "Arbitrum Sepolia";
  } else if (networkName === "arbitrumSepolia") {
    localAddress = DEPLOYMENTS.arbitrumSepolia.oft;
    remoteAddress = DEPLOYMENTS.sepolia.oft;
    remoteLzChainId = CHAINS.sepolia.lzChainId;
    remoteName = "Sepolia";
  } else {
    console.error("This script is for sepolia or arbitrumSepolia only");
    process.exit(1);
  }

  console.log(`Local contract: ${localAddress}`);
  console.log(`Remote contract: ${remoteAddress} (${remoteName}, LZ ID: ${remoteLzChainId})\n`);

  const localOFT = new hre.ethers.Contract(localAddress, OFT_ABI, signer);

  // Check if already configured
  const existing = await localOFT.trustedRemoteLookup(remoteLzChainId);
  if (existing && existing !== "0x") {
    console.log(`✓ Already configured for ${remoteName}`);
    console.log(`  Existing path: ${existing}`);
    return;
  }

  // Set trusted remote
  console.log(`Setting trusted remote for ${remoteName}...`);
  const tx = await localOFT.setTrustedRemoteAddress(remoteLzChainId, remoteAddress);
  console.log(`TX: ${tx.hash}`);
  await tx.wait();
  console.log(`✅ Trusted remote configured!\n`);

  // Verify
  const newPath = await localOFT.trustedRemoteLookup(remoteLzChainId);
  console.log(`Verified path: ${newPath}`);
}

main()
  .then(() => process.exit(0))
  .catch((error) => {
    console.error(error);
    process.exit(1);
  });
