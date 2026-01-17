// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

/**
 * @title ILayerZeroEndpoint
 * @notice Interface for LayerZero messaging endpoint
 */
interface ILayerZeroEndpoint {
    /**
     * @notice Send a message to a destination chain
     * @param dstChainId Destination chain ID
     * @param destination Destination address (path)
     * @param payload Message payload
     * @param refundAddress Address to refund excess fee
     * @param zroPaymentAddress Address for ZRO payment
     * @param adapterParams Adapter parameters
     */
    function send(
        uint16 dstChainId,
        bytes calldata destination,
        bytes calldata payload,
        address payable refundAddress,
        address zroPaymentAddress,
        bytes calldata adapterParams
    ) external payable;

    /**
     * @notice Estimate fee for sending a message
     * @param dstChainId Destination chain ID
     * @param userApplication User application address
     * @param payload Message payload
     * @param payInZRO Whether to pay in ZRO
     * @param adapterParams Adapter parameters
     * @return nativeFee Native token fee
     * @return zroFee ZRO token fee
     */
    function estimateFees(
        uint16 dstChainId,
        address userApplication,
        bytes calldata payload,
        bool payInZRO,
        bytes calldata adapterParams
    ) external view returns (uint256 nativeFee, uint256 zroFee);

    /**
     * @notice Get the inbound nonce for a source chain/address
     */
    function getInboundNonce(uint16 srcChainId, bytes calldata srcAddress) external view returns (uint64);

    /**
     * @notice Get the outbound nonce for a destination chain/address
     */
    function getOutboundNonce(uint16 dstChainId, address srcAddress) external view returns (uint64);

    /**
     * @notice Retry a failed message
     */
    function retryPayload(
        uint16 srcChainId,
        bytes calldata srcAddress,
        bytes calldata payload
    ) external;

    /**
     * @notice Check if there's a stored payload
     */
    function hasStoredPayload(uint16 srcChainId, bytes calldata srcAddress) external view returns (bool);

    /**
     * @notice Get stored payload length
     */
    function getSendLibraryAddress(address userApplication) external view returns (address);

    /**
     * @notice Get receive library address
     */
    function getReceiveLibraryAddress(address userApplication) external view returns (address);

    /**
     * @notice Check if sending is supported to a chain
     */
    function isSendingPayload() external view returns (bool);

    /**
     * @notice Check if receiving is supported from a chain
     */
    function isReceivingPayload() external view returns (bool);

    /**
     * @notice Get the configuration for a chain
     */
    function getConfig(
        uint16 version,
        uint16 chainId,
        address userApplication,
        uint256 configType
    ) external view returns (bytes memory);

    /**
     * @notice Get send version
     */
    function getSendVersion(address userApplication) external view returns (uint16);

    /**
     * @notice Get receive version
     */
    function getReceiveVersion(address userApplication) external view returns (uint16);

    /**
     * @notice Set configuration for a chain
     */
    function setConfig(
        uint16 version,
        uint16 chainId,
        uint256 configType,
        bytes calldata config
    ) external;

    /**
     * @notice Set send version
     */
    function setSendVersion(uint16 version) external;

    /**
     * @notice Set receive version
     */
    function setReceiveVersion(uint16 version) external;

    /**
     * @notice Force resume receive
     */
    function forceResumeReceive(uint16 srcChainId, bytes calldata srcAddress) external;
}

/**
 * @title ILayerZeroReceiver
 * @notice Interface for receiving LayerZero messages
 */
interface ILayerZeroReceiver {
    /**
     * @notice Receive a message from LayerZero
     * @param srcChainId Source chain ID
     * @param srcAddress Source address
     * @param nonce Message nonce
     * @param payload Message payload
     */
    function lzReceive(
        uint16 srcChainId,
        bytes calldata srcAddress,
        uint64 nonce,
        bytes calldata payload
    ) external;
}

/**
 * @title ILayerZeroUserApplicationConfig
 * @notice Interface for LayerZero user application configuration
 */
interface ILayerZeroUserApplicationConfig {
    /**
     * @notice Set configuration for a chain
     */
    function setConfig(
        uint16 version,
        uint16 chainId,
        uint256 configType,
        bytes calldata config
    ) external;

    /**
     * @notice Set send version
     */
    function setSendVersion(uint16 version) external;

    /**
     * @notice Set receive version
     */
    function setReceiveVersion(uint16 version) external;

    /**
     * @notice Force resume receive
     */
    function forceResumeReceive(uint16 srcChainId, bytes calldata srcAddress) external;
}
