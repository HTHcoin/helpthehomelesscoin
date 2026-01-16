# HTH LayerZero OFT (Omnichain Fungible Token)

This implementation enables HTH to be bridged across all EVM-compatible chains using LayerZero's messaging protocol.

## Architecture

### Native HTH Chain
Deploy `ProxyOFT.sol` - This contract locks native HTH tokens when bridging out and unlocks them when bridging back.

### Other EVM Chains (Ethereum, BSC, Polygon, Arbitrum, etc.)
Deploy `HTH_OFT.sol` with `isNativeChain=false` - This contract mints wrapped HTH when receiving from another chain and burns them when sending out.

## Contracts

| Contract | Description |
|----------|-------------|
| `HTH_OFT.sol` | Main OFT implementation with ERC20 functionality |
| `ProxyOFT.sol` | Proxy for native chain (locks/unlocks existing tokens) |
| `OFTCore.sol` | Core cross-chain transfer logic |
| `LzApp.sol` | Base LayerZero application with trusted remote management |
| `ILayerZeroEndpoint.sol` | LayerZero endpoint interface |
| `IOFT.sol` | OFT standard interface |

## LayerZero Chain IDs

| Chain | ID |
|-------|-----|
| Ethereum | 101 |
| BSC | 102 |
| Avalanche | 106 |
| Polygon | 109 |
| Arbitrum | 110 |
| Optimism | 111 |
| Fantom | 112 |
| Base | 184 |
| HTH Native | TBD |

## Deployment Steps

### 1. Deploy on Native HTH Chain

```solidity
// If HTH has an existing ERC20-like token contract:
ProxyOFT proxy = new ProxyOFT(
    LZ_ENDPOINT_ADDRESS,    // LayerZero endpoint on HTH
    HTH_TOKEN_ADDRESS       // Existing HTH token address
);

// If deploying fresh (HTH is the native token):
HTH_OFT oft = new HTH_OFT(
    LZ_ENDPOINT_ADDRESS,    // LayerZero endpoint on HTH
    true,                   // isNativeChain = true
    INITIAL_SUPPLY          // Initial supply (e.g., 7_000_000_000 * 1e8)
);
```

### 2. Deploy on Other Chains (Ethereum, BSC, etc.)

```solidity
HTH_OFT oft = new HTH_OFT(
    LZ_ENDPOINT_ADDRESS,    // LayerZero endpoint on this chain
    false,                  // isNativeChain = false
    0                       // No initial supply (minted on demand)
);
```

### 3. Configure Trusted Remotes

After deploying on all chains, link them together:

```solidity
// On HTH native chain, trust the Ethereum deployment:
proxyOFT.setTrustedRemoteAddress(
    101,                                    // Ethereum chain ID
    abi.encodePacked(ETHEREUM_OFT_ADDRESS)  // Ethereum OFT address
);

// On Ethereum, trust the HTH native deployment:
ethereumOFT.setTrustedRemoteAddress(
    HTH_CHAIN_ID,                           // HTH chain ID
    abi.encodePacked(HTH_PROXY_ADDRESS)     // HTH ProxyOFT address
);

// Repeat for all chain pairs
```

## Usage

### Bridge HTH to Another Chain

```solidity
// 1. Approve the ProxyOFT to spend your HTH (on native chain)
hthToken.approve(proxyOFTAddress, amount);

// 2. Estimate the bridge fee
(uint256 nativeFee, ) = proxyOFT.estimateBridgeFee(
    101,                        // Destination: Ethereum
    abi.encodePacked(recipient),// Recipient address
    amount,                     // Amount to bridge
    false,                      // Don't use ZRO for payment
    bytes("")                   // Default adapter params
);

// 3. Bridge the tokens
proxyOFT.bridge{value: nativeFee}(
    101,                        // Destination: Ethereum
    abi.encodePacked(recipient),// Recipient address
    amount,                     // Amount to bridge
    payable(msg.sender),        // Refund address
    address(0),                 // ZRO payment address (none)
    bytes("")                   // Adapter params
);
```

### Bridge HTH Back to Native Chain

```solidity
// From Ethereum back to HTH native:
(uint256 nativeFee, ) = ethereumOFT.estimateBridgeFee(
    HTH_CHAIN_ID,
    abi.encodePacked(recipient),
    amount,
    false,
    bytes("")
);

ethereumOFT.bridge{value: nativeFee}(
    HTH_CHAIN_ID,
    abi.encodePacked(recipient),
    amount,
    payable(msg.sender),
    address(0),
    bytes("")
);
```

## Security Considerations

1. **Trusted Remotes**: Only the owner can set trusted remote addresses. Verify addresses carefully before setting.

2. **Initial Configuration**: After deployment, configure:
   - Trusted remotes for all connected chains
   - Minimum destination gas limits (if using custom adapter params)
   - Payload size limits (if needed)

3. **Ownership**: Transfer ownership to a multisig or governance contract for production.

4. **LayerZero Fees**: Bridge operations require native gas tokens to pay LayerZero relayer fees.

## Token Details

- **Name**: HelpTheHomeless
- **Symbol**: HTH
- **Decimals**: 8 (matching native HTH)
- **Max Supply**: 7,000,000,000 HTH (only mintable on native chain)

## Integration with HTH Ecosystem

This OFT implementation integrates with the HTH omnichain architecture:

- **Native Chain**: X25X PoW mining, masternodes, governance
- **EVM Chains**: DeFi access, DEX liquidity, cross-chain applications
- **Staking Nodes**: 1M HTH collateral across any supported chain
- **Governance**: Cross-chain proposal and voting aggregation
