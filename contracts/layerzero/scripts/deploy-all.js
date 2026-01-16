/**
 * Deploy HTH_OFT to multiple chains in sequence
 *
 * Usage:
 *   npx hardhat run scripts/deploy-all.js
 *
 * This script will deploy to all configured external chains.
 * Make sure you have funds on each chain before running.
 *
 * Environment variables:
 *   PRIVATE_KEY - Deployer private key
 */

const hre = require("hardhat");
const { exec } = require("child_process");
const util = require("util");
const execPromise = util.promisify(exec);

// Chains to deploy to (in order)
const DEPLOY_CHAINS = [
  // Mainnets
  "ethereum",
  "bsc",
  "polygon",
  "arbitrum",
  "optimism",
  "avalanche",
  "fantom",
  "base"
];

// Testnet chains (comment out mainnets and uncomment these for testing)
const DEPLOY_CHAINS_TESTNET = [
  "sepolia",
  "bscTestnet",
  "mumbai",
  "arbitrumSepolia",
  "optimismSepolia",
  "baseSepolia"
];

async function deployToChain(chainName) {
  console.log(`\n========================================`);
  console.log(`Deploying to ${chainName}...`);
  console.log(`========================================\n`);

  try {
    const { stdout, stderr } = await execPromise(
      `npx hardhat run scripts/deploy-hth-oft.js --network ${chainName}`,
      { cwd: __dirname + "/.." }
    );
    console.log(stdout);
    if (stderr) console.error(stderr);
    return { chain: chainName, success: true };
  } catch (error) {
    console.error(`Failed to deploy to ${chainName}:`, error.message);
    return { chain: chainName, success: false, error: error.message };
  }
}

async function main() {
  console.log(`\n╔════════════════════════════════════════╗`);
  console.log(`║   HTH OFT Multi-Chain Deployment       ║`);
  console.log(`╚════════════════════════════════════════╝\n`);

  // Use testnet chains if --testnet flag is passed
  const useTestnet = process.argv.includes("--testnet");
  const chains = useTestnet ? DEPLOY_CHAINS_TESTNET : DEPLOY_CHAINS;

  console.log(`Mode: ${useTestnet ? "TESTNET" : "MAINNET"}`);
  console.log(`Chains: ${chains.join(", ")}\n`);

  const results = [];

  for (const chain of chains) {
    const result = await deployToChain(chain);
    results.push(result);

    // Small delay between deployments
    await new Promise(resolve => setTimeout(resolve, 2000));
  }

  // Summary
  console.log(`\n╔════════════════════════════════════════╗`);
  console.log(`║   Deployment Summary                   ║`);
  console.log(`╚════════════════════════════════════════╝\n`);

  const successful = results.filter(r => r.success);
  const failed = results.filter(r => !r.success);

  console.log(`✅ Successful: ${successful.length}`);
  successful.forEach(r => console.log(`   - ${r.chain}`));

  if (failed.length > 0) {
    console.log(`\n❌ Failed: ${failed.length}`);
    failed.forEach(r => console.log(`   - ${r.chain}: ${r.error}`));
  }

  console.log(`\n========================================`);
  console.log(`Next Steps:`);
  console.log(`========================================`);
  console.log(`1. Update DEPLOYMENTS in scripts/config.js with the addresses above`);
  console.log(`2. Run configure-trusted-remotes.js on each chain`);
  console.log(`3. Test bridging with small amounts first`);
}

main()
  .then(() => process.exit(0))
  .catch((error) => {
    console.error(error);
    process.exit(1);
  });
