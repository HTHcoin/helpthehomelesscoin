// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

import "./ILayerZeroEndpoint.sol";

/**
 * @title LzApp
 * @notice Base contract for LayerZero applications
 * @dev Handles trusted remotes, endpoint configuration, and message receiving
 */
abstract contract LzApp is ILayerZeroReceiver, ILayerZeroUserApplicationConfig {
    // ============ State Variables ============

    ILayerZeroEndpoint public immutable lzEndpoint;
    address public owner;

    // chainId => trusted remote address (path = remote + local)
    mapping(uint16 => bytes) public trustedRemoteLookup;
    mapping(uint16 => mapping(uint16 => uint256)) public minDstGasLookup;
    mapping(uint16 => uint256) public payloadSizeLimitLookup;
    address public precrime;

    // ============ Events ============

    event SetTrustedRemote(uint16 remoteChainId, bytes path);
    event SetTrustedRemoteAddress(uint16 remoteChainId, bytes remoteAddress);
    event SetPrecrime(address precrime);
    event SetMinDstGas(uint16 dstChainId, uint16 packetType, uint256 minGas);

    // ============ Errors ============

    error NotOwner();
    error InvalidEndpointCaller();
    error InvalidSource();
    error InvalidPayloadSize();
    error InvalidMinGas();
    error InvalidAdapterParams();
    error DestinationGasNotSet();

    // ============ Modifiers ============

    modifier onlyOwner() {
        if (msg.sender != owner) revert NotOwner();
        _;
    }

    // ============ Constructor ============

    constructor(address _endpoint) {
        lzEndpoint = ILayerZeroEndpoint(_endpoint);
        owner = msg.sender;
    }

    // ============ External Functions ============

    /**
     * @notice Receive a message from LayerZero endpoint
     */
    function lzReceive(
        uint16 _srcChainId,
        bytes calldata _srcAddress,
        uint64 _nonce,
        bytes calldata _payload
    ) public virtual override {
        // Only endpoint can call
        if (msg.sender != address(lzEndpoint)) revert InvalidEndpointCaller();

        bytes memory trustedRemote = trustedRemoteLookup[_srcChainId];
        // Source must be trusted
        if (
            _srcAddress.length != trustedRemote.length ||
            trustedRemote.length == 0 ||
            keccak256(_srcAddress) != keccak256(trustedRemote)
        ) revert InvalidSource();

        _blockingLzReceive(_srcChainId, _srcAddress, _nonce, _payload);
    }

    // ============ Internal Functions ============

    /**
     * @notice Internal function to handle received messages
     * @dev Override this in derived contracts
     */
    function _blockingLzReceive(
        uint16 _srcChainId,
        bytes memory _srcAddress,
        uint64 _nonce,
        bytes memory _payload
    ) internal virtual;

    /**
     * @notice Send a message via LayerZero
     */
    function _lzSend(
        uint16 _dstChainId,
        bytes memory _payload,
        address payable _refundAddress,
        address _zroPaymentAddress,
        bytes memory _adapterParams,
        uint256 _nativeFee
    ) internal virtual {
        bytes memory trustedRemote = trustedRemoteLookup[_dstChainId];
        if (trustedRemote.length == 0) revert InvalidSource();

        _checkPayloadSize(_dstChainId, _payload.length);

        lzEndpoint.send{value: _nativeFee}(
            _dstChainId,
            trustedRemote,
            _payload,
            _refundAddress,
            _zroPaymentAddress,
            _adapterParams
        );
    }

    /**
     * @notice Check gas limit in adapter params
     */
    function _checkGasLimit(
        uint16 _dstChainId,
        uint16 _type,
        bytes memory _adapterParams,
        uint256 _extraGas
    ) internal view virtual {
        uint256 providedGasLimit = _getGasLimit(_adapterParams);
        uint256 minGasLimit = minDstGasLookup[_dstChainId][_type] + _extraGas;
        if (providedGasLimit < minGasLimit) revert DestinationGasNotSet();
    }

    /**
     * @notice Get gas limit from adapter params
     */
    function _getGasLimit(bytes memory _adapterParams) internal pure virtual returns (uint256 gasLimit) {
        if (_adapterParams.length < 34) revert InvalidAdapterParams();
        assembly {
            gasLimit := mload(add(_adapterParams, 34))
        }
    }

    /**
     * @notice Check payload size against limit
     */
    function _checkPayloadSize(uint16 _dstChainId, uint256 _payloadSize) internal view virtual {
        uint256 payloadSizeLimit = payloadSizeLimitLookup[_dstChainId];
        if (payloadSizeLimit == 0) {
            payloadSizeLimit = 10000; // default
        }
        if (_payloadSize > payloadSizeLimit) revert InvalidPayloadSize();
    }

    // ============ View Functions ============

    /**
     * @notice Get trusted remote for a chain
     */
    function getTrustedRemoteAddress(uint16 _remoteChainId) external view returns (bytes memory) {
        bytes memory path = trustedRemoteLookup[_remoteChainId];
        if (path.length == 0) return bytes("");
        // Extract just the remote address (first 20 bytes)
        return abi.encodePacked(path.slice(0, 20));
    }

    /**
     * @notice Check if a remote is trusted
     */
    function isTrustedRemote(uint16 _srcChainId, bytes calldata _srcAddress) external view returns (bool) {
        bytes memory trustedSource = trustedRemoteLookup[_srcChainId];
        return keccak256(trustedSource) == keccak256(_srcAddress);
    }

    // ============ Admin Functions ============

    /**
     * @notice Set trusted remote path (remote + local)
     */
    function setTrustedRemote(uint16 _remoteChainId, bytes calldata _path) external onlyOwner {
        trustedRemoteLookup[_remoteChainId] = _path;
        emit SetTrustedRemote(_remoteChainId, _path);
    }

    /**
     * @notice Set trusted remote address (convenience function)
     */
    function setTrustedRemoteAddress(uint16 _remoteChainId, bytes calldata _remoteAddress) external onlyOwner {
        trustedRemoteLookup[_remoteChainId] = abi.encodePacked(_remoteAddress, address(this));
        emit SetTrustedRemoteAddress(_remoteChainId, _remoteAddress);
    }

    /**
     * @notice Set minimum gas for destination chain
     */
    function setMinDstGas(uint16 _dstChainId, uint16 _packetType, uint256 _minGas) external onlyOwner {
        minDstGasLookup[_dstChainId][_packetType] = _minGas;
        emit SetMinDstGas(_dstChainId, _packetType, _minGas);
    }

    /**
     * @notice Set payload size limit for a chain
     */
    function setPayloadSizeLimit(uint16 _dstChainId, uint256 _size) external onlyOwner {
        payloadSizeLimitLookup[_dstChainId] = _size;
    }

    /**
     * @notice Set precrime contract
     */
    function setPrecrime(address _precrime) external onlyOwner {
        precrime = _precrime;
        emit SetPrecrime(_precrime);
    }

    /**
     * @notice Transfer ownership
     */
    function transferOwnership(address newOwner) external onlyOwner {
        require(newOwner != address(0), "LzApp: new owner is zero");
        owner = newOwner;
    }

    // ============ LayerZero Config ============

    function setConfig(
        uint16 _version,
        uint16 _chainId,
        uint256 _configType,
        bytes calldata _config
    ) external override onlyOwner {
        lzEndpoint.setConfig(_version, _chainId, _configType, _config);
    }

    function setSendVersion(uint16 _version) external override onlyOwner {
        lzEndpoint.setSendVersion(_version);
    }

    function setReceiveVersion(uint16 _version) external override onlyOwner {
        lzEndpoint.setReceiveVersion(_version);
    }

    function forceResumeReceive(uint16 _srcChainId, bytes calldata _srcAddress) external override onlyOwner {
        lzEndpoint.forceResumeReceive(_srcChainId, _srcAddress);
    }
}

// Helper library for bytes slicing
library BytesLib {
    function slice(bytes memory _bytes, uint256 _start, uint256 _length) internal pure returns (bytes memory) {
        require(_length + 31 >= _length, "slice_overflow");
        require(_bytes.length >= _start + _length, "slice_outOfBounds");
        bytes memory tempBytes;
        assembly {
            switch iszero(_length)
            case 0 {
                tempBytes := mload(0x40)
                let lengthmod := and(_length, 31)
                let mc := add(add(tempBytes, lengthmod), mul(0x20, iszero(lengthmod)))
                let end := add(mc, _length)
                for {
                    let cc := add(add(add(_bytes, lengthmod), mul(0x20, iszero(lengthmod))), _start)
                } lt(mc, end) {
                    mc := add(mc, 0x20)
                    cc := add(cc, 0x20)
                } {
                    mstore(mc, mload(cc))
                }
                mstore(tempBytes, _length)
                mstore(0x40, and(add(mc, 31), not(31)))
            }
            default {
                tempBytes := mload(0x40)
                mstore(tempBytes, 0)
                mstore(0x40, add(tempBytes, 0x20))
            }
        }
        return tempBytes;
    }
}

// Make slice available on bytes
using BytesLib for bytes;
