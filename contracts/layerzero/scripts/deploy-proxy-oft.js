/**
 * Deploy ProxyOFT on the native HTH chain
 *
 * Usage:
 *   npx hardhat run scripts/deploy-proxy-oft.js --network hth
 *
 * Environment variables:
 *   PRIVATE_KEY - Deployer private key
 *   HTH_TOKEN_ADDRESS - Address of existing HTH token (optional)
 */

const hre = require("hardhat");
const { getChainConfig } = require("./config");

async function main() {
  const networkName = hre.network.name;
  console.log(`\n========================================`);
  console.log(`Deploying ProxyOFT on ${networkName}`);
  console.log(`========================================\n`);

  // Get chain config
  const chainConfig = getChainConfig(networkName);

  if (!chainConfig.isNative) {
    console.error("Error: ProxyOFT should only be deployed on the native HTH chain.");
    console.error("Use deploy-hth-oft.js for external chains.");
    process.exit(1);
  }

  const [deployer] = await hre.ethers.getSigners();
  console.log(`Deployer address: ${deployer.address}`);
  console.log(`Deployer balance: ${hre.ethers.formatEther(await hre.ethers.provider.getBalance(deployer.address))} ETH\n`);

  // Check for HTH token address
  const hthTokenAddress = process.env.HTH_TOKEN_ADDRESS;

  if (hthTokenAddress) {
    // Deploy ProxyOFT (wraps existing HTH token)
    console.log(`Deploying ProxyOFT to wrap existing HTH token at ${hthTokenAddress}...`);

    const ProxyOFT = await hre.ethers.getContractFactory("ProxyOFT");
    const proxyOFT = await ProxyOFT.deploy(
      chainConfig.endpoint,
      hthTokenAddress
    );

    await proxyOFT.waitForDeployment();
    const proxyAddress = await proxyOFT.getAddress();

    console.log(`\n✅ ProxyOFT deployed to: ${proxyAddress}`);
    console.log(`   LayerZero Endpoint: ${chainConfig.endpoint}`);
    console.log(`   HTH Token: ${hthTokenAddress}`);

    // Verify contract
    console.log(`\nTo verify on explorer:`);
    console.log(`npx hardhat verify --network ${networkName} ${proxyAddress} ${chainConfig.endpoint} ${hthTokenAddress}`);

    return {
      type: "ProxyOFT",
      address: proxyAddress,
      endpoint: chainConfig.endpoint,
      token: hthTokenAddress
    };
  } else {
    // Deploy HTH_OFT as the native token (mint fresh supply)
    console.log(`No HTH_TOKEN_ADDRESS provided.`);
    console.log(`Deploying HTH_OFT as the native token with initial supply...\n`);

    const INITIAL_SUPPLY = hre.ethers.parseUnits("7000000000", 8); // 7 billion with 8 decimals

    const HTH_OFT = await hre.ethers.getContractFactory("HTH_OFT");
    const hthOFT = await HTH_OFT.deploy(
      chainConfig.endpoint,
      true,  // isNativeChain = true
      INITIAL_SUPPLY
    );

    await hthOFT.waitForDeployment();
    const oftAddress = await hthOFT.getAddress();

    console.log(`\n✅ HTH_OFT (Native) deployed to: ${oftAddress}`);
    console.log(`   LayerZero Endpoint: ${chainConfig.endpoint}`);
    console.log(`   Initial Supply: 7,000,000,000 HTH`);
    console.log(`   Is Native Chain: true`);

    // Verify contract
    console.log(`\nTo verify on explorer:`);
    console.log(`npx hardhat verify --network ${networkName} ${oftAddress} ${chainConfig.endpoint} true ${INITIAL_SUPPLY}`);

    return {
      type: "HTH_OFT",
      address: oftAddress,
      endpoint: chainConfig.endpoint,
      isNative: true,
      initialSupply: "7000000000"
    };
  }
}

main()
  .then((result) => {
    console.log(`\n========================================`);
    console.log(`Deployment Complete!`);
    console.log(`========================================`);
    console.log(`\nUpdate DEPLOYMENTS in scripts/config.js:`);
    console.log(JSON.stringify(result, null, 2));
    process.exit(0);
  })
  .catch((error) => {
    console.error(error);
    process.exit(1);
  });
