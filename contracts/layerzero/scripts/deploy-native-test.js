/**
 * Deploy HTH_OFT as native chain with test tokens
 * For testing cross-chain bridging
 */

const hre = require("hardhat");
const { getChainConfig } = require("./config");

async function main() {
  const networkName = hre.network.name;
  console.log(`\n========================================`);
  console.log(`Deploying HTH_OFT as NATIVE on ${networkName}`);
  console.log(`========================================\n`);

  const chainConfig = getChainConfig(networkName);
  const [deployer] = await hre.ethers.getSigners();

  console.log(`Deployer: ${deployer.address}`);
  const balance = await hre.ethers.provider.getBalance(deployer.address);
  console.log(`Balance: ${hre.ethers.formatEther(balance)} ETH\n`);

  // Initial supply: 1,000,000 HTH for testing (8 decimals)
  const INITIAL_SUPPLY = hre.ethers.parseUnits("1000000", 8);

  console.log(`Deploying with:`);
  console.log(`  Endpoint: ${chainConfig.endpoint}`);
  console.log(`  Is Native Chain: true`);
  console.log(`  Initial Supply: 1,000,000 HTH\n`);

  const HTH_OFT = await hre.ethers.getContractFactory("HTH_OFT");
  const hthOFT = await HTH_OFT.deploy(
    chainConfig.endpoint,
    true,  // isNativeChain = true
    INITIAL_SUPPLY
  );

  await hthOFT.waitForDeployment();
  const address = await hthOFT.getAddress();

  console.log(`\n✅ HTH_OFT (Native) deployed to: ${address}`);
  console.log(`   Chain: ${chainConfig.name}`);
  console.log(`   LayerZero Chain ID: ${chainConfig.lzChainId}`);
  console.log(`   Is Native Chain: true`);
  console.log(`   Initial Supply: 1,000,000 HTH`);
  console.log(`   Owner: ${deployer.address}`);

  // Verify balance
  const tokenBalance = await hthOFT.balanceOf(deployer.address);
  console.log(`   Owner Balance: ${hre.ethers.formatUnits(tokenBalance, 8)} HTH`);

  console.log(`\nUpdate config.js:`);
  console.log(`  ${networkName}: { oft: "${address}" }`);
}

main()
  .then(() => process.exit(0))
  .catch((error) => {
    console.error(error);
    process.exit(1);
  });
