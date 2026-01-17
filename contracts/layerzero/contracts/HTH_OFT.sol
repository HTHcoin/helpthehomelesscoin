// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

import "./OFTCore.sol";

/**
 * @title HTH_OFT
 * @notice Omnichain Fungible Token for HelpTheHomeless (HTH)
 * @dev This contract enables HTH to be bridged across all EVM chains via LayerZero
 *
 * Deployment Strategy:
 * - HTH Native Chain: Deploy as ProxyOFT (locks/unlocks native HTH)
 * - Other EVM Chains: Deploy as OFT (mints/burns wrapped HTH)
 *
 * Supported Chains (LayerZero Chain IDs):
 * - Ethereum: 101
 * - BSC: 102
 * - Avalanche: 106
 * - Polygon: 109
 * - Arbitrum: 110
 * - Optimism: 111
 * - Fantom: 112
 * - Base: 184
 * - And many more...
 */
contract HTH_OFT is OFTCore {
    // ============ ERC20 State ============

    string public constant name = "HelpTheHomeless";
    string public constant symbol = "HTH";
    uint8 public constant decimals = 8;

    uint256 public totalSupply;
    mapping(address => uint256) public balanceOf;
    mapping(address => mapping(address => uint256)) public allowance;

    // ============ OFT State ============

    // True if this is the native chain (lock/unlock), false for other chains (mint/burn)
    bool public immutable isNativeChain;

    // Total amount locked (only used on native chain)
    uint256 public lockedSupply;

    // ============ Events ============

    event Transfer(address indexed from, address indexed to, uint256 value);
    event Approval(address indexed owner, address indexed spender, uint256 value);
    event TokensLocked(address indexed from, uint256 amount, uint16 dstChainId);
    event TokensUnlocked(address indexed to, uint256 amount, uint16 srcChainId);
    event TokensMinted(address indexed to, uint256 amount, uint16 srcChainId);
    event TokensBurned(address indexed from, uint256 amount, uint16 dstChainId);

    // ============ Constructor ============

    /**
     * @param _lzEndpoint LayerZero endpoint address
     * @param _isNativeChain True if deploying on HTH native chain
     * @param _initialSupply Initial supply (only for native chain, 0 for others)
     */
    constructor(
        address _lzEndpoint,
        bool _isNativeChain,
        uint256 _initialSupply
    ) OFTCore(_lzEndpoint) {
        isNativeChain = _isNativeChain;

        if (_isNativeChain && _initialSupply > 0) {
            // On native chain, mint initial supply to deployer
            totalSupply = _initialSupply;
            balanceOf[msg.sender] = _initialSupply;
            emit Transfer(address(0), msg.sender, _initialSupply);
        }
    }

    // ============ ERC20 Functions ============

    function transfer(address to, uint256 value) external returns (bool) {
        return _transfer(msg.sender, to, value);
    }

    function approve(address spender, uint256 value) external returns (bool) {
        allowance[msg.sender][spender] = value;
        emit Approval(msg.sender, spender, value);
        return true;
    }

    function transferFrom(address from, address to, uint256 value) external returns (bool) {
        if (allowance[from][msg.sender] != type(uint256).max) {
            require(allowance[from][msg.sender] >= value, "HTH: insufficient allowance");
            allowance[from][msg.sender] -= value;
        }
        return _transfer(from, to, value);
    }

    function _transfer(address from, address to, uint256 value) internal returns (bool) {
        require(to != address(0), "HTH: transfer to zero");
        require(balanceOf[from] >= value, "HTH: insufficient balance");

        balanceOf[from] -= value;
        balanceOf[to] += value;

        emit Transfer(from, to, value);
        return true;
    }

    // ============ OFT Interface ============

    /**
     * @notice Returns the circulating supply on this chain
     */
    function circulatingSupply() public view override returns (uint256) {
        if (isNativeChain) {
            // On native chain: total supply minus locked amount
            return totalSupply - lockedSupply;
        } else {
            // On other chains: total minted supply
            return totalSupply;
        }
    }

    /**
     * @notice Returns this contract as the token
     */
    function token() external view override returns (address) {
        return address(this);
    }

    // ============ Cross-Chain Functions ============

    /**
     * @notice Bridge HTH to another chain
     * @param _dstChainId Destination LayerZero chain ID
     * @param _toAddress Recipient address on destination chain
     * @param _amount Amount of HTH to bridge
     * @param _refundAddress Address to refund excess gas fee
     * @param _zroPaymentAddress ZRO payment address (use address(0) for native)
     * @param _adapterParams LayerZero adapter params
     */
    function bridge(
        uint16 _dstChainId,
        bytes calldata _toAddress,
        uint256 _amount,
        address payable _refundAddress,
        address _zroPaymentAddress,
        bytes calldata _adapterParams
    ) external payable {
        sendFrom(
            msg.sender,
            _dstChainId,
            _toAddress,
            _amount,
            _refundAddress,
            _zroPaymentAddress,
            _adapterParams
        );
    }

    /**
     * @notice Estimate bridge fee
     */
    function estimateBridgeFee(
        uint16 _dstChainId,
        bytes calldata _toAddress,
        uint256 _amount,
        bool _useZro,
        bytes calldata _adapterParams
    ) external view returns (uint256 nativeFee, uint256 zroFee) {
        return estimateSendFee(_dstChainId, _toAddress, _amount, _useZro, _adapterParams);
    }

    // ============ Internal OFT Functions ============

    /**
     * @notice Debit tokens from sender
     * @dev On native chain: locks tokens
     * @dev On other chains: burns tokens
     */
    function _debitFrom(
        address _from,
        uint16 _dstChainId,
        bytes memory /*_toAddress*/,
        uint256 _amount
    ) internal override returns (uint256) {
        require(balanceOf[_from] >= _amount, "HTH: insufficient balance");

        if (msg.sender != _from) {
            require(allowance[_from][msg.sender] >= _amount, "HTH: insufficient allowance");
            allowance[_from][msg.sender] -= _amount;
        }

        if (isNativeChain) {
            // Lock tokens on native chain
            balanceOf[_from] -= _amount;
            lockedSupply += _amount;
            emit TokensLocked(_from, _amount, _dstChainId);
        } else {
            // Burn tokens on other chains
            balanceOf[_from] -= _amount;
            totalSupply -= _amount;
            emit TokensBurned(_from, _amount, _dstChainId);
            emit Transfer(_from, address(0), _amount);
        }

        return _amount;
    }

    /**
     * @notice Credit tokens to recipient
     * @dev On native chain: unlocks tokens
     * @dev On other chains: mints tokens
     */
    function _creditTo(
        uint16 _srcChainId,
        address _toAddress,
        uint256 _amount
    ) internal override returns (uint256) {
        if (isNativeChain) {
            // Unlock tokens on native chain
            require(lockedSupply >= _amount, "HTH: insufficient locked");
            lockedSupply -= _amount;
            balanceOf[_toAddress] += _amount;
            emit TokensUnlocked(_toAddress, _amount, _srcChainId);
        } else {
            // Mint tokens on other chains
            totalSupply += _amount;
            balanceOf[_toAddress] += _amount;
            emit TokensMinted(_toAddress, _amount, _srcChainId);
            emit Transfer(address(0), _toAddress, _amount);
        }

        return _amount;
    }

    // ============ Receive ETH ============

    receive() external payable {}
}
