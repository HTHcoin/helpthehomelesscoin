/**
 * LayerZero Chain Configuration
 * Contains endpoint addresses and chain IDs for all supported networks
 */

const CHAINS = {
  // ============ MAINNETS ============

  hth: {
    name: "HTH Native",
    lzChainId: 999,  // TODO: Register with LayerZero for official chain ID
    endpoint: "0x0000000000000000000000000000000000000000",  // TODO: Deploy LZ endpoint on HTH
    networkName: "hth",
    isNative: true
  },

  ethereum: {
    name: "Ethereum",
    lzChainId: 101,
    endpoint: "0x66A71Dcef29A0fFBDBE3c6a460a3B5BC225Cd675",
    networkName: "ethereum",
    isNative: false
  },

  bsc: {
    name: "BNB Smart Chain",
    lzChainId: 102,
    endpoint: "0x3c2269811836af69497E5F486A85D7316753cf62",
    networkName: "bsc",
    isNative: false
  },

  avalanche: {
    name: "Avalanche",
    lzChainId: 106,
    endpoint: "0x3c2269811836af69497E5F486A85D7316753cf62",
    networkName: "avalanche",
    isNative: false
  },

  polygon: {
    name: "Polygon",
    lzChainId: 109,
    endpoint: "0x3c2269811836af69497E5F486A85D7316753cf62",
    networkName: "polygon",
    isNative: false
  },

  arbitrum: {
    name: "Arbitrum One",
    lzChainId: 110,
    endpoint: "0x3c2269811836af69497E5F486A85D7316753cf62",
    networkName: "arbitrum",
    isNative: false
  },

  optimism: {
    name: "Optimism",
    lzChainId: 111,
    endpoint: "0x3c2269811836af69497E5F486A85D7316753cf62",
    networkName: "optimism",
    isNative: false
  },

  fantom: {
    name: "Fantom",
    lzChainId: 112,
    endpoint: "0xb6319cC6c8c27A8F5dAF0dD3DF91EA35C4720dd7",
    networkName: "fantom",
    isNative: false
  },

  base: {
    name: "Base",
    lzChainId: 184,
    endpoint: "0xb6319cC6c8c27A8F5dAF0dD3DF91EA35C4720dd7",
    networkName: "base",
    isNative: false
  },

  // ============ TESTNETS ============

  sepolia: {
    name: "Sepolia",
    lzChainId: 10161,
    endpoint: "0xae92d5aD7583AD66E49A0c67BAd18F6ba52dDDc1",
    networkName: "sepolia",
    isNative: false
  },

  bscTestnet: {
    name: "BSC Testnet",
    lzChainId: 10102,
    endpoint: "0x6Fcb97553D41516Cb228ac03FdC8B9a0a9df04A1",
    networkName: "bscTestnet",
    isNative: false
  },

  mumbai: {
    name: "Mumbai",
    lzChainId: 10109,
    endpoint: "0xf69186dfBa60DdB133E91E9A4B5673624293d8F8",
    networkName: "mumbai",
    isNative: false
  },

  arbitrumSepolia: {
    name: "Arbitrum Sepolia",
    lzChainId: 10231,
    endpoint: "0x6098e96a28E02f27B1e6BD381f870F1C8Bd169d3",
    networkName: "arbitrumSepolia",
    isNative: false
  },

  optimismSepolia: {
    name: "Optimism Sepolia",
    lzChainId: 10232,
    endpoint: "0x55370E0fBB5f5b8dAeD978BA1c075a499eB107B8",
    networkName: "optimismSepolia",
    isNative: false
  },

  baseSepolia: {
    name: "Base Sepolia",
    lzChainId: 10245,
    endpoint: "0x55370E0fBB5f5b8dAeD978BA1c075a499eB107B8",
    networkName: "baseSepolia",
    isNative: false
  }
};

// Deployed contract addresses (update after deployment)
const DEPLOYMENTS = {
  hth: {
    proxyOFT: "",
    hthToken: ""  // Native HTH token address (if using ProxyOFT)
  },
  ethereum: { oft: "" },
  bsc: { oft: "" },
  avalanche: { oft: "" },
  polygon: { oft: "" },
  arbitrum: { oft: "" },
  optimism: { oft: "" },
  fantom: { oft: "" },
  base: { oft: "" },
  // Testnets
  sepolia: { oft: "" },
  bscTestnet: { oft: "" },
  mumbai: { oft: "" },
  arbitrumSepolia: { oft: "" },
  optimismSepolia: { oft: "" },
  baseSepolia: { oft: "" }
};

// Get chain config by network name
function getChainConfig(networkName) {
  const config = Object.values(CHAINS).find(c => c.networkName === networkName);
  if (!config) {
    throw new Error(`Unknown network: ${networkName}`);
  }
  return config;
}

// Get all mainnet chains
function getMainnetChains() {
  return Object.entries(CHAINS)
    .filter(([_, config]) => !config.networkName.includes("Testnet") &&
                            !config.networkName.includes("sepolia") &&
                            !config.networkName.includes("mumbai"))
    .map(([key, config]) => ({ key, ...config }));
}

// Get all testnet chains
function getTestnetChains() {
  return Object.entries(CHAINS)
    .filter(([_, config]) => config.networkName.includes("Testnet") ||
                            config.networkName.includes("sepolia") ||
                            config.networkName.includes("mumbai"))
    .map(([key, config]) => ({ key, ...config }));
}

module.exports = {
  CHAINS,
  DEPLOYMENTS,
  getChainConfig,
  getMainnetChains,
  getTestnetChains
};
