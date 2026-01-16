/**
 * Deploy HTH_OFT on external EVM chains
 *
 * Usage:
 *   npx hardhat run scripts/deploy-hth-oft.js --network ethereum
 *   npx hardhat run scripts/deploy-hth-oft.js --network bsc
 *   npx hardhat run scripts/deploy-hth-oft.js --network polygon
 *   npx hardhat run scripts/deploy-hth-oft.js --network arbitrum
 *   npx hardhat run scripts/deploy-hth-oft.js --network optimism
 *   npx hardhat run scripts/deploy-hth-oft.js --network base
 *
 * Environment variables:
 *   PRIVATE_KEY - Deployer private key
 */

const hre = require("hardhat");
const { getChainConfig } = require("./config");

async function main() {
  const networkName = hre.network.name;
  console.log(`\n========================================`);
  console.log(`Deploying HTH_OFT on ${networkName}`);
  console.log(`========================================\n`);

  // Get chain config
  const chainConfig = getChainConfig(networkName);

  if (chainConfig.isNative) {
    console.error("Error: HTH_OFT (external) should not be deployed on the native HTH chain.");
    console.error("Use deploy-proxy-oft.js for the native chain.");
    process.exit(1);
  }

  const [deployer] = await hre.ethers.getSigners();
  console.log(`Deployer address: ${deployer.address}`);

  const balance = await hre.ethers.provider.getBalance(deployer.address);
  console.log(`Deployer balance: ${hre.ethers.formatEther(balance)} ETH\n`);

  if (balance === 0n) {
    console.error("Error: Deployer has no balance. Please fund the wallet first.");
    process.exit(1);
  }

  console.log(`Chain: ${chainConfig.name}`);
  console.log(`LayerZero Chain ID: ${chainConfig.lzChainId}`);
  console.log(`LayerZero Endpoint: ${chainConfig.endpoint}\n`);

  console.log(`Deploying HTH_OFT (wrapped)...`);

  const HTH_OFT = await hre.ethers.getContractFactory("HTH_OFT");
  const hthOFT = await HTH_OFT.deploy(
    chainConfig.endpoint,
    false,  // isNativeChain = false
    0       // No initial supply (minted on demand from bridge)
  );

  await hthOFT.waitForDeployment();
  const oftAddress = await hthOFT.getAddress();

  console.log(`\n✅ HTH_OFT deployed to: ${oftAddress}`);
  console.log(`   Chain: ${chainConfig.name}`);
  console.log(`   LayerZero Chain ID: ${chainConfig.lzChainId}`);
  console.log(`   LayerZero Endpoint: ${chainConfig.endpoint}`);
  console.log(`   Is Native Chain: false`);
  console.log(`   Initial Supply: 0 (minted when bridged from native chain)`);

  // Verify contract
  console.log(`\nTo verify on explorer:`);
  console.log(`npx hardhat verify --network ${networkName} ${oftAddress} ${chainConfig.endpoint} false 0`);

  console.log(`\n========================================`);
  console.log(`Deployment Complete!`);
  console.log(`========================================`);
  console.log(`\nUpdate DEPLOYMENTS in scripts/config.js:`);
  console.log(`  ${networkName}: { oft: "${oftAddress}" }`);
  console.log(`\nNext step: Run configure-trusted-remotes.js to link chains`);

  return {
    chain: chainConfig.name,
    lzChainId: chainConfig.lzChainId,
    address: oftAddress,
    endpoint: chainConfig.endpoint,
    isNative: false
  };
}

main()
  .then(() => process.exit(0))
  .catch((error) => {
    console.error(error);
    process.exit(1);
  });
