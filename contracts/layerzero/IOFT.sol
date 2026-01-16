// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

/**
 * @title IOFT - Interface for Omnichain Fungible Token
 * @notice LayerZero OFT standard interface
 */
interface IOFT {
    /**
     * @notice Emitted when tokens are sent cross-chain
     */
    event SendToChain(
        uint16 indexed dstChainId,
        address indexed from,
        bytes indexed toAddress,
        uint256 amount
    );

    /**
     * @notice Emitted when tokens are received from another chain
     */
    event ReceiveFromChain(
        uint16 indexed srcChainId,
        bytes indexed srcAddress,
        address indexed toAddress,
        uint256 amount
    );

    /**
     * @notice Estimate fee for sending tokens cross-chain
     * @param dstChainId Destination chain ID (LayerZero chain ID)
     * @param toAddress Recipient address on destination chain
     * @param amount Amount of tokens to send
     * @param useZro Whether to pay fee in ZRO token
     * @param adapterParams LayerZero adapter parameters
     * @return nativeFee Estimated fee in native token
     * @return zroFee Estimated fee in ZRO token
     */
    function estimateSendFee(
        uint16 dstChainId,
        bytes calldata toAddress,
        uint256 amount,
        bool useZro,
        bytes calldata adapterParams
    ) external view returns (uint256 nativeFee, uint256 zroFee);

    /**
     * @notice Send tokens to another chain
     * @param from Address sending tokens
     * @param dstChainId Destination chain ID
     * @param toAddress Recipient address on destination chain
     * @param amount Amount of tokens to send
     * @param refundAddress Address to refund excess fee
     * @param zroPaymentAddress Address to pay ZRO fee (if applicable)
     * @param adapterParams LayerZero adapter parameters
     */
    function sendFrom(
        address from,
        uint16 dstChainId,
        bytes calldata toAddress,
        uint256 amount,
        address payable refundAddress,
        address zroPaymentAddress,
        bytes calldata adapterParams
    ) external payable;

    /**
     * @notice Get the circulating supply on this chain
     */
    function circulatingSupply() external view returns (uint256);

    /**
     * @notice Get the token interface type
     */
    function token() external view returns (address);
}
