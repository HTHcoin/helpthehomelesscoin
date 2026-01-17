'use client';

import { useState, useEffect, useCallback } from 'react';
import { ethers } from 'ethers';
import { CHAINS, OFT_ABI, LAYERZERO_SCAN, ChainConfig } from '../lib/config';

type Status = 'idle' | 'pending' | 'success' | 'error';

export default function BridgePage() {
  // Wallet state
  const [address, setAddress] = useState<string>('');
  const [provider, setProvider] = useState<ethers.BrowserProvider | null>(null);
  const [chainId, setChainId] = useState<number>(0);

  // Bridge state
  const [fromChain, setFromChain] = useState<string>('sepolia');
  const [toChain, setToChain] = useState<string>('arbitrumSepolia');
  const [amount, setAmount] = useState<string>('');
  const [fee, setFee] = useState<string>('0');

  // Balances
  const [balances, setBalances] = useState<Record<string, string>>({});

  // UI state
  const [status, setStatus] = useState<Status>('idle');
  const [statusMessage, setStatusMessage] = useState<string>('');
  const [txHash, setTxHash] = useState<string>('');
  const [isLoading, setIsLoading] = useState<boolean>(false);

  // Check if MetaMask is installed
  const hasMetaMask = typeof window !== 'undefined' && typeof window.ethereum !== 'undefined';

  // Connect wallet
  const connectWallet = async () => {
    if (!hasMetaMask) {
      alert('Please install MetaMask to use this bridge');
      return;
    }

    try {
      const provider = new ethers.BrowserProvider(window.ethereum);
      const accounts = await provider.send('eth_requestAccounts', []);
      const network = await provider.getNetwork();

      setProvider(provider);
      setAddress(accounts[0]);
      setChainId(Number(network.chainId));
    } catch (error: any) {
      console.error('Failed to connect:', error);
      alert('Failed to connect wallet');
    }
  };

  // Disconnect wallet
  const disconnectWallet = () => {
    setProvider(null);
    setAddress('');
    setChainId(0);
    setBalances({});
  };

  // Switch network
  const switchNetwork = async (targetChainId: number) => {
    if (!window.ethereum) return;

    const chainHex = '0x' + targetChainId.toString(16);

    try {
      await window.ethereum.request({
        method: 'wallet_switchEthereumChain',
        params: [{ chainId: chainHex }],
      });
    } catch (error: any) {
      // Chain not added, try to add it
      if (error.code === 4902) {
        const chain = Object.values(CHAINS).find(c => c.id === targetChainId);
        if (chain) {
          await window.ethereum.request({
            method: 'wallet_addEthereumChain',
            params: [{
              chainId: chainHex,
              chainName: chain.name,
              rpcUrls: [chain.rpcUrl],
              blockExplorerUrls: [chain.explorer],
            }],
          });
        }
      }
    }
  };

  // Fetch balances
  const fetchBalances = useCallback(async () => {
    if (!address) return;

    const newBalances: Record<string, string> = {};

    for (const [key, chain] of Object.entries(CHAINS)) {
      try {
        const rpcProvider = new ethers.JsonRpcProvider(chain.rpcUrl);
        const contract = new ethers.Contract(chain.oftAddress, OFT_ABI, rpcProvider);
        const balance = await contract.balanceOf(address);
        newBalances[key] = ethers.formatUnits(balance, 8);
      } catch (error) {
        console.error(`Failed to fetch balance for ${key}:`, error);
        newBalances[key] = '0';
      }
    }

    setBalances(newBalances);
  }, [address]);

  // Estimate fee
  const estimateFee = useCallback(async () => {
    if (!amount || parseFloat(amount) <= 0) {
      setFee('0');
      return;
    }

    try {
      const fromChainConfig = CHAINS[fromChain];
      const toChainConfig = CHAINS[toChain];

      const rpcProvider = new ethers.JsonRpcProvider(fromChainConfig.rpcUrl);
      const contract = new ethers.Contract(fromChainConfig.oftAddress, OFT_ABI, rpcProvider);

      const amountWei = ethers.parseUnits(amount, 8);
      const toAddress = ethers.solidityPacked(['address'], [address || ethers.ZeroAddress]);

      const [nativeFee] = await contract.estimateBridgeFee(
        toChainConfig.lzChainId,
        toAddress,
        amountWei,
        false,
        '0x'
      );

      setFee(ethers.formatEther(nativeFee));
    } catch (error) {
      console.error('Failed to estimate fee:', error);
      setFee('0');
    }
  }, [amount, fromChain, toChain, address]);

  // Execute bridge
  const executeBridge = async () => {
    if (!provider || !address || !amount) return;

    const fromChainConfig = CHAINS[fromChain];
    const toChainConfig = CHAINS[toChain];

    // Check if on correct network
    if (chainId !== fromChainConfig.id) {
      await switchNetwork(fromChainConfig.id);
      return;
    }

    setIsLoading(true);
    setStatus('pending');
    setStatusMessage('Preparing transaction...');

    try {
      const signer = await provider.getSigner();
      const contract = new ethers.Contract(fromChainConfig.oftAddress, OFT_ABI, signer);

      const amountWei = ethers.parseUnits(amount, 8);
      const toAddress = ethers.solidityPacked(['address'], [address]);

      // Get fee
      const [nativeFee] = await contract.estimateBridgeFee(
        toChainConfig.lzChainId,
        toAddress,
        amountWei,
        false,
        '0x'
      );

      setStatusMessage('Please confirm in MetaMask...');

      // Execute bridge
      const tx = await contract.bridge(
        toChainConfig.lzChainId,
        toAddress,
        amountWei,
        address,
        ethers.ZeroAddress,
        '0x',
        { value: nativeFee }
      );

      setTxHash(tx.hash);
      setStatusMessage('Transaction submitted, waiting for confirmation...');

      await tx.wait();

      setStatus('success');
      setStatusMessage(`Bridged ${amount} HTH from ${fromChainConfig.shortName} to ${toChainConfig.shortName}`);

      // Refresh balances after a delay
      setTimeout(fetchBalances, 5000);

    } catch (error: any) {
      console.error('Bridge failed:', error);
      setStatus('error');
      setStatusMessage(error.reason || error.message || 'Bridge failed');
    } finally {
      setIsLoading(false);
    }
  };

  // Swap chains
  const swapChains = () => {
    setFromChain(toChain);
    setToChain(fromChain);
  };

  // Set max amount
  const setMaxAmount = () => {
    const balance = balances[fromChain] || '0';
    setAmount(balance);
  };

  // Effects
  useEffect(() => {
    if (address) {
      fetchBalances();
    }
  }, [address, fetchBalances]);

  useEffect(() => {
    const timer = setTimeout(estimateFee, 500);
    return () => clearTimeout(timer);
  }, [estimateFee]);

  // Listen for account/network changes
  useEffect(() => {
    if (!window.ethereum) return;

    const handleAccountsChanged = (accounts: string[]) => {
      if (accounts.length === 0) {
        disconnectWallet();
      } else {
        setAddress(accounts[0]);
      }
    };

    const handleChainChanged = (chainIdHex: string) => {
      setChainId(parseInt(chainIdHex, 16));
    };

    window.ethereum.on('accountsChanged', handleAccountsChanged);
    window.ethereum.on('chainChanged', handleChainChanged);

    return () => {
      window.ethereum.removeListener('accountsChanged', handleAccountsChanged);
      window.ethereum.removeListener('chainChanged', handleChainChanged);
    };
  }, []);

  const fromChainConfig = CHAINS[fromChain];
  const toChainConfig = CHAINS[toChain];
  const isWrongNetwork = address && chainId !== fromChainConfig.id;
  const canBridge = address && amount && parseFloat(amount) > 0 && parseFloat(amount) <= parseFloat(balances[fromChain] || '0');

  return (
    <div className="container">
      <div className="header">
        <h1>HTH Bridge</h1>
        <p>Bridge HTH tokens across chains via LayerZero</p>
      </div>

      <div className="card">
        {!address ? (
          <button className="connect-btn" onClick={connectWallet}>
            Connect Wallet
          </button>
        ) : (
          <>
            <div className="wallet-info">
              <span className="wallet-address">
                {address.slice(0, 6)}...{address.slice(-4)}
              </span>
              <button className="disconnect-btn" onClick={disconnectWallet}>
                Disconnect
              </button>
            </div>

            {/* Chain Selector */}
            <div className="chain-selector">
              <div
                className={`chain-box ${chainId === fromChainConfig.id ? 'selected' : ''}`}
                onClick={() => switchNetwork(fromChainConfig.id)}
              >
                <div className="chain-icon">{fromChainConfig.icon}</div>
                <div className="chain-name">{fromChainConfig.shortName}</div>
              </div>

              <button className="swap-btn" onClick={swapChains}>
                ⇄
              </button>

              <div className="chain-box">
                <div className="chain-icon">{toChainConfig.icon}</div>
                <div className="chain-name">{toChainConfig.shortName}</div>
              </div>
            </div>

            {/* Amount Input */}
            <div className="amount-section">
              <div className="amount-label">
                <span>Amount</span>
                <span>Balance: {parseFloat(balances[fromChain] || '0').toLocaleString()} HTH</span>
              </div>
              <div className="amount-input-wrapper">
                <input
                  type="number"
                  className="amount-input"
                  placeholder="0.0"
                  value={amount}
                  onChange={(e) => setAmount(e.target.value)}
                />
                <button className="max-btn" onClick={setMaxAmount}>MAX</button>
                <span className="token-badge">HTH</span>
              </div>
            </div>

            {/* Fee Info */}
            <div className="fee-info">
              <div className="fee-row">
                <span>Bridge Fee</span>
                <span>{parseFloat(fee).toFixed(6)} ETH</span>
              </div>
              <div className="fee-row">
                <span>You will receive</span>
                <span>{amount || '0'} HTH</span>
              </div>
            </div>

            {/* Bridge Button */}
            {isWrongNetwork ? (
              <button
                className="bridge-btn"
                onClick={() => switchNetwork(fromChainConfig.id)}
              >
                Switch to {fromChainConfig.shortName}
              </button>
            ) : (
              <button
                className="bridge-btn"
                onClick={executeBridge}
                disabled={!canBridge || isLoading}
              >
                {isLoading && <span className="spinner"></span>}
                {isLoading ? 'Bridging...' : 'Bridge HTH'}
              </button>
            )}

            {/* Status */}
            {status !== 'idle' && (
              <div className={`status ${status}`}>
                <p>{statusMessage}</p>
                {txHash && (
                  <p>
                    <a href={`${LAYERZERO_SCAN}${txHash}`} target="_blank" rel="noopener noreferrer">
                      View on LayerZero Scan →
                    </a>
                  </p>
                )}
              </div>
            )}

            {/* Balances */}
            <div className="balances">
              {Object.entries(CHAINS).map(([key, chain]) => (
                <div key={key} className="balance-item">
                  <div className="balance-chain">{chain.icon} {chain.shortName}</div>
                  <div className="balance-amount">
                    {parseFloat(balances[key] || '0').toLocaleString()} HTH
                  </div>
                </div>
              ))}
            </div>
          </>
        )}
      </div>

      <div className="footer">
        Powered by <a href="https://layerzero.network" target="_blank" rel="noopener noreferrer">LayerZero</a>
      </div>
    </div>
  );
}

// TypeScript declarations for window.ethereum
declare global {
  interface Window {
    ethereum?: any;
  }
}
