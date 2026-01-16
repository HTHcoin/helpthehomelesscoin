/**
 * Test bridging HTH between chains
 *
 * Usage:
 *   npx hardhat run scripts/test-bridge.js --network ethereum
 *
 * This will bridge a small amount from the current network to another chain.
 *
 * Environment variables:
 *   PRIVATE_KEY - Sender private key
 *   DESTINATION_CHAIN - Target chain name (e.g., "bsc", "polygon")
 *   BRIDGE_AMOUNT - Amount to bridge (default: 1 HTH)
 */

const hre = require("hardhat");
const { CHAINS, DEPLOYMENTS, getChainConfig } = require("./config");

// ABI for bridge operations
const OFT_ABI = [
  "function bridge(uint16 _dstChainId, bytes calldata _toAddress, uint256 _amount, address payable _refundAddress, address _zroPaymentAddress, bytes calldata _adapterParams) external payable",
  "function estimateBridgeFee(uint16 _dstChainId, bytes calldata _toAddress, uint256 _amount, bool _useZro, bytes calldata _adapterParams) external view returns (uint256 nativeFee, uint256 zroFee)",
  "function balanceOf(address) external view returns (uint256)",
  "function approve(address spender, uint256 amount) external returns (bool)",
  "function allowance(address owner, address spender) external view returns (uint256)"
];

// ProxyOFT ABI (needs approval first)
const PROXY_OFT_ABI = [
  ...OFT_ABI,
  "function innerToken() external view returns (address)"
];

const ERC20_ABI = [
  "function approve(address spender, uint256 amount) external returns (bool)",
  "function allowance(address owner, address spender) external view returns (uint256)",
  "function balanceOf(address) external view returns (uint256)"
];

async function main() {
  const networkName = hre.network.name;
  const destinationChain = process.env.DESTINATION_CHAIN || "bsc";
  const bridgeAmount = process.env.BRIDGE_AMOUNT || "1"; // 1 HTH

  console.log(`\n========================================`);
  console.log(`Testing Bridge: ${networkName} -> ${destinationChain}`);
  console.log(`========================================\n`);

  const sourceConfig = getChainConfig(networkName);
  const destConfig = CHAINS[destinationChain];

  if (!destConfig) {
    console.error(`Error: Unknown destination chain: ${destinationChain}`);
    process.exit(1);
  }

  const [signer] = await hre.ethers.getSigners();
  console.log(`Sender: ${signer.address}`);

  // Get contract addresses
  let sourceAddress;
  if (sourceConfig.isNative) {
    sourceAddress = DEPLOYMENTS.hth?.proxyOFT || DEPLOYMENTS.hth?.hthOFT;
  } else {
    sourceAddress = DEPLOYMENTS[networkName]?.oft;
  }

  if (!sourceAddress) {
    console.error(`Error: No deployment found for ${networkName}`);
    process.exit(1);
  }

  console.log(`Source contract: ${sourceAddress}`);
  console.log(`Destination chain: ${destConfig.name} (LZ ID: ${destConfig.lzChainId})`);

  // Connect to contract
  const oft = new hre.ethers.Contract(
    sourceAddress,
    sourceConfig.isNative ? PROXY_OFT_ABI : OFT_ABI,
    signer
  );

  // Parse amount (8 decimals)
  const amount = hre.ethers.parseUnits(bridgeAmount, 8);
  console.log(`Amount: ${bridgeAmount} HTH (${amount} units)\n`);

  // If ProxyOFT, need to approve the token first
  if (sourceConfig.isNative && DEPLOYMENTS.hth?.proxyOFT) {
    console.log(`Checking token approval for ProxyOFT...`);
    const tokenAddress = await oft.innerToken();
    const token = new hre.ethers.Contract(tokenAddress, ERC20_ABI, signer);

    const currentAllowance = await token.allowance(signer.address, sourceAddress);
    if (currentAllowance < amount) {
      console.log(`Approving tokens...`);
      const approveTx = await token.approve(sourceAddress, hre.ethers.MaxUint256);
      await approveTx.wait();
      console.log(`Approved!\n`);
    } else {
      console.log(`Already approved.\n`);
    }
  }

  // Check balance
  const balance = await oft.balanceOf(signer.address);
  console.log(`Current balance: ${hre.ethers.formatUnits(balance, 8)} HTH`);

  if (balance < amount) {
    console.error(`Error: Insufficient balance`);
    process.exit(1);
  }

  // Encode recipient address
  const toAddress = hre.ethers.solidityPacked(["address"], [signer.address]);

  // Estimate fee
  console.log(`\nEstimating bridge fee...`);
  const [nativeFee, zroFee] = await oft.estimateBridgeFee(
    destConfig.lzChainId,
    toAddress,
    amount,
    false,  // Don't use ZRO
    "0x"    // Default adapter params
  );

  console.log(`Native fee: ${hre.ethers.formatEther(nativeFee)} ETH`);
  console.log(`ZRO fee: ${zroFee}`);

  // Check ETH balance for fee
  const ethBalance = await hre.ethers.provider.getBalance(signer.address);
  console.log(`\nETH balance: ${hre.ethers.formatEther(ethBalance)}`);

  if (ethBalance < nativeFee) {
    console.error(`Error: Insufficient ETH for bridge fee`);
    process.exit(1);
  }

  // Execute bridge
  console.log(`\n🚀 Executing bridge...`);
  console.log(`   From: ${networkName} -> ${destConfig.name}`);
  console.log(`   Amount: ${bridgeAmount} HTH`);
  console.log(`   Fee: ${hre.ethers.formatEther(nativeFee)} ETH`);

  const tx = await oft.bridge(
    destConfig.lzChainId,
    toAddress,
    amount,
    signer.address,  // Refund address
    hre.ethers.ZeroAddress,  // ZRO payment address
    "0x",  // Adapter params
    { value: nativeFee }
  );

  console.log(`\nTransaction submitted: ${tx.hash}`);
  console.log(`Waiting for confirmation...`);

  const receipt = await tx.wait();
  console.log(`\n✅ Bridge transaction confirmed!`);
  console.log(`   Block: ${receipt.blockNumber}`);
  console.log(`   Gas used: ${receipt.gasUsed}`);

  // Check new balance
  const newBalance = await oft.balanceOf(signer.address);
  console.log(`\nNew balance: ${hre.ethers.formatUnits(newBalance, 8)} HTH`);

  console.log(`\n========================================`);
  console.log(`Bridge Complete!`);
  console.log(`========================================`);
  console.log(`\nYour tokens are being transferred via LayerZero.`);
  console.log(`This typically takes 1-3 minutes depending on the chains.`);
  console.log(`\nTrack on LayerZero Scan: https://layerzeroscan.com/tx/${tx.hash}`);
}

main()
  .then(() => process.exit(0))
  .catch((error) => {
    console.error(error);
    process.exit(1);
  });
