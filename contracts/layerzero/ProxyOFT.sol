// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

import "./OFTCore.sol";

/**
 * @title ProxyOFT
 * @notice Proxy OFT for the native HTH chain
 * @dev Use this contract on the native HTH chain to lock/unlock existing HTH tokens
 *      while HTH_OFT contracts on other chains mint/burn wrapped versions
 *
 * Deployment:
 * - Native HTH Chain: Deploy ProxyOFT (this contract) - locks/unlocks native HTH
 * - Other EVM Chains: Deploy HTH_OFT with isNativeChain=false - mints/burns wrapped HTH
 */
contract ProxyOFT is OFTCore {
    // ============ State ============

    // The native HTH token on this chain
    IERC20 public immutable innerToken;

    // Total amount locked in this contract
    uint256 public outboundAmount;

    // ============ Events ============

    event TokensLocked(address indexed from, uint256 amount, uint16 dstChainId);
    event TokensUnlocked(address indexed to, uint256 amount, uint16 srcChainId);

    // ============ Constructor ============

    /**
     * @param _lzEndpoint LayerZero endpoint address
     * @param _token Address of the native HTH token
     */
    constructor(
        address _lzEndpoint,
        address _token
    ) OFTCore(_lzEndpoint) {
        innerToken = IERC20(_token);
    }

    // ============ OFT Interface ============

    /**
     * @notice Returns the circulating supply (total supply minus locked amount)
     */
    function circulatingSupply() public view override returns (uint256) {
        return innerToken.totalSupply() - outboundAmount;
    }

    /**
     * @notice Returns the underlying token address
     */
    function token() external view override returns (address) {
        return address(innerToken);
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
     * @notice Debit tokens from sender (lock in this contract)
     */
    function _debitFrom(
        address _from,
        uint16 _dstChainId,
        bytes memory /*_toAddress*/,
        uint256 _amount
    ) internal override returns (uint256) {
        require(
            innerToken.allowance(_from, address(this)) >= _amount,
            "ProxyOFT: insufficient allowance"
        );

        // Transfer tokens from sender to this contract (lock)
        uint256 balanceBefore = innerToken.balanceOf(address(this));
        innerToken.transferFrom(_from, address(this), _amount);
        uint256 balanceAfter = innerToken.balanceOf(address(this));

        // Calculate actual amount received (handles fee-on-transfer tokens)
        uint256 actualAmount = balanceAfter - balanceBefore;

        outboundAmount += actualAmount;
        emit TokensLocked(_from, actualAmount, _dstChainId);

        return actualAmount;
    }

    /**
     * @notice Credit tokens to recipient (unlock from this contract)
     */
    function _creditTo(
        uint16 _srcChainId,
        address _toAddress,
        uint256 _amount
    ) internal override returns (uint256) {
        require(outboundAmount >= _amount, "ProxyOFT: insufficient locked");

        outboundAmount -= _amount;

        // Transfer tokens from this contract to recipient (unlock)
        innerToken.transfer(_toAddress, _amount);

        emit TokensUnlocked(_toAddress, _amount, _srcChainId);
        return _amount;
    }

    // ============ Receive ETH ============

    receive() external payable {}
}

/**
 * @title IERC20
 * @notice Minimal ERC20 interface for ProxyOFT
 */
interface IERC20 {
    function totalSupply() external view returns (uint256);
    function balanceOf(address account) external view returns (uint256);
    function transfer(address to, uint256 amount) external returns (bool);
    function allowance(address owner, address spender) external view returns (uint256);
    function approve(address spender, uint256 amount) external returns (bool);
    function transferFrom(address from, address to, uint256 amount) external returns (bool);
}
