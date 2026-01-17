// HTH Bridge Configuration

export interface ChainConfig {
  id: number;
  name: string;
  shortName: string;
  lzChainId: number;
  rpcUrl: string;
  explorer: string;
  oftAddress: string;
  isNative: boolean;
  icon: string;
}

export const CHAINS: Record<string, ChainConfig> = {
  sepolia: {
    id: 11155111,
    name: "Ethereum Sepolia",
    shortName: "Sepolia",
    lzChainId: 10161,
    rpcUrl: "https://ethereum-sepolia-rpc.publicnode.com",
    explorer: "https://sepolia.etherscan.io",
    oftAddress: "0x688b1c1b22aA1bDff449285A6F954aFdC9DE05d9",
    isNative: true,
    icon: "⟠"
  },
  arbitrumSepolia: {
    id: 421614,
    name: "Arbitrum Sepolia",
    shortName: "Arb Sepolia",
    lzChainId: 10231,
    rpcUrl: "https://sepolia-rollup.arbitrum.io/rpc",
    explorer: "https://sepolia.arbiscan.io",
    oftAddress: "0xcbAddD7bC31A41Fb9acC4f7de961d26d145d1FF6",
    isNative: false,
    icon: "🔵"
  },
  bscTestnet: {
    id: 97,
    name: "BSC Testnet",
    shortName: "BSC Test",
    lzChainId: 10102,
    rpcUrl: "https://data-seed-prebsc-1-s1.binance.org:8545/",
    explorer: "https://testnet.bscscan.com",
    oftAddress: "0xcbAddD7bC31A41Fb9acC4f7de961d26d145d1FF6",
    isNative: false,
    icon: "🟡"
  }
};

export const OFT_ABI = [
  "function name() view returns (string)",
  "function symbol() view returns (string)",
  "function decimals() view returns (uint8)",
  "function balanceOf(address) view returns (uint256)",
  "function totalSupply() view returns (uint256)",
  "function lockedSupply() view returns (uint256)",
  "function isNativeChain() view returns (bool)",
  "function bridge(uint16 _dstChainId, bytes calldata _toAddress, uint256 _amount, address payable _refundAddress, address _zroPaymentAddress, bytes calldata _adapterParams) external payable",
  "function estimateBridgeFee(uint16 _dstChainId, bytes calldata _toAddress, uint256 _amount, bool _useZro, bytes calldata _adapterParams) external view returns (uint256 nativeFee, uint256 zroFee)"
];

export const LAYERZERO_SCAN = "https://testnet.layerzeroscan.com/tx/";
