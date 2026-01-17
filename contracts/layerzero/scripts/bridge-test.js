/**
 * Bridge HTH from Sepolia to Arbitrum Sepolia
 */

const hre = require("hardhat");
const { CHAINS, DEPLOYMENTS } = require("./config");

const OFT_ABI = [
  "function bridge(uint16 _dstChainId, bytes calldata _toAddress, uint256 _amount, address payable _refundAddress, address _zroPaymentAddress, bytes calldata _adapterParams) external payable",
  "function estimateBridgeFee(uint16 _dstChainId, bytes calldata _toAddress, uint256 _amount, bool _useZro, bytes calldata _adapterParams) external view returns (uint256 nativeFee, uint256 zroFee)",
  "function balanceOf(address) external view returns (uint256)",
  "function totalSupply() external view returns (uint256)",
  "function lockedSupply() external view returns (uint256)"
];

async function main() {
  const [signer] = await hre.ethers.getSigners();

  console.log(`\n========================================`);
  console.log(`Bridging HTH: Sepolia -> Arbitrum Sepolia`);
  console.log(`========================================\n`);

  const sepoliaOFT = DEPLOYMENTS.sepolia.oft;
  const arbSepoliaLzId = CHAINS.arbitrumSepolia.lzChainId;

  console.log(`From: Sepolia (${sepoliaOFT})`);
  console.log(`To: Arbitrum Sepolia (LZ ID: ${arbSepoliaLzId})`);
  console.log(`Sender: ${signer.address}\n`);

  const oft = new hre.ethers.Contract(sepoliaOFT, OFT_ABI, signer);

  // Check balance before
  const balanceBefore = await oft.balanceOf(signer.address);
  console.log(`Balance before: ${hre.ethers.formatUnits(balanceBefore, 8)} HTH`);

  // Amount to bridge: 1 HTH
  const amount = hre.ethers.parseUnits("1", 8);
  console.log(`Amount to bridge: 1 HTH\n`);

  // Encode recipient
  const toAddress = hre.ethers.solidityPacked(["address"], [signer.address]);

  // Estimate fee
  console.log(`Estimating bridge fee...`);
  const [nativeFee] = await oft.estimateBridgeFee(
    arbSepoliaLzId,
    toAddress,
    amount,
    false,
    "0x"
  );
  console.log(`Bridge fee: ${hre.ethers.formatEther(nativeFee)} ETH\n`);

  // Execute bridge
  console.log(`🚀 Executing bridge...`);
  const tx = await oft.bridge(
    arbSepoliaLzId,
    toAddress,
    amount,
    signer.address,
    hre.ethers.ZeroAddress,
    "0x",
    { value: nativeFee }
  );

  console.log(`TX Hash: ${tx.hash}`);
  console.log(`Waiting for confirmation...`);

  const receipt = await tx.wait();
  console.log(`\n✅ Bridge transaction confirmed!`);
  console.log(`   Block: ${receipt.blockNumber}`);
  console.log(`   Gas used: ${receipt.gasUsed}`);

  // Check balance after
  const balanceAfter = await oft.balanceOf(signer.address);
  const lockedSupply = await oft.lockedSupply();

  console.log(`\nBalance after: ${hre.ethers.formatUnits(balanceAfter, 8)} HTH`);
  console.log(`Locked supply: ${hre.ethers.formatUnits(lockedSupply, 8)} HTH`);

  console.log(`\n========================================`);
  console.log(`Bridge Complete!`);
  console.log(`========================================`);
  console.log(`\nTokens are being sent via LayerZero (~1-3 min).`);
  console.log(`\nTrack: https://testnet.layerzeroscan.com/tx/${tx.hash}`);
  console.log(`\nCheck Arbitrum Sepolia balance after delivery:`);
  console.log(`  Contract: ${DEPLOYMENTS.arbitrumSepolia.oft}`);
}

main()
  .then(() => process.exit(0))
  .catch((error) => {
    console.error(error);
    process.exit(1);
  });
