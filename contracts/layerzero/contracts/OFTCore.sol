// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

import "./LzApp.sol";
import "./IOFT.sol";

/**
 * @title OFTCore
 * @notice Core functionality for Omnichain Fungible Tokens
 * @dev Implements cross-chain token transfer logic via LayerZero
 */
abstract contract OFTCore is LzApp, IOFT {
    // ============ Constants ============

    uint16 public constant PT_SEND = 0;           // Packet type for send
    uint16 public constant PT_SEND_AND_CALL = 1;  // Packet type for send with call

    bool public useCustomAdapterParams;

    // ============ Events ============

    event SetUseCustomAdapterParams(bool useCustomAdapterParams);

    // ============ Constructor ============

    constructor(address _lzEndpoint) LzApp(_lzEndpoint) {}

    // ============ External Functions ============

    /**
     * @notice Estimate fee for cross-chain transfer
     */
    function estimateSendFee(
        uint16 _dstChainId,
        bytes calldata _toAddress,
        uint256 _amount,
        bool _useZro,
        bytes calldata _adapterParams
    ) public view virtual override returns (uint256 nativeFee, uint256 zroFee) {
        bytes memory payload = _encodeSendPayload(_toAddress, _amount);
        return lzEndpoint.estimateFees(_dstChainId, address(this), payload, _useZro, _adapterParams);
    }

    /**
     * @notice Send tokens cross-chain
     */
    function sendFrom(
        address _from,
        uint16 _dstChainId,
        bytes calldata _toAddress,
        uint256 _amount,
        address payable _refundAddress,
        address _zroPaymentAddress,
        bytes calldata _adapterParams
    ) public payable virtual override {
        _send(
            _from,
            _dstChainId,
            _toAddress,
            _amount,
            _refundAddress,
            _zroPaymentAddress,
            _adapterParams
        );
    }

    // ============ Internal Functions ============

    /**
     * @notice Internal send implementation
     */
    function _send(
        address _from,
        uint16 _dstChainId,
        bytes memory _toAddress,
        uint256 _amount,
        address payable _refundAddress,
        address _zroPaymentAddress,
        bytes memory _adapterParams
    ) internal virtual returns (uint256 amount) {
        _checkAdapterParams(_dstChainId, PT_SEND, _adapterParams, 0);

        amount = _debitFrom(_from, _dstChainId, _toAddress, _amount);

        bytes memory lzPayload = _encodeSendPayload(_toAddress, amount);
        _lzSend(_dstChainId, lzPayload, _refundAddress, _zroPaymentAddress, _adapterParams, msg.value);

        emit SendToChain(_dstChainId, _from, _toAddress, amount);
    }

    /**
     * @notice Handle received cross-chain message
     */
    function _blockingLzReceive(
        uint16 _srcChainId,
        bytes memory _srcAddress,
        uint64 _nonce,
        bytes memory _payload
    ) internal virtual override {
        uint16 packetType;
        assembly {
            packetType := mload(add(_payload, 32))
        }

        if (packetType == PT_SEND) {
            _sendAck(_srcChainId, _srcAddress, _nonce, _payload);
        } else {
            revert("OFTCore: unknown packet type");
        }
    }

    /**
     * @notice Process send acknowledgment (receive tokens)
     */
    function _sendAck(
        uint16 _srcChainId,
        bytes memory _srcAddress,
        uint64 /*_nonce*/,
        bytes memory _payload
    ) internal virtual {
        (bytes memory toAddressBytes, uint256 amount) = _decodeSendPayload(_payload);

        address to = _bytesToAddress(toAddressBytes);

        amount = _creditTo(_srcChainId, to, amount);

        emit ReceiveFromChain(_srcChainId, _srcAddress, to, amount);
    }

    /**
     * @notice Check adapter params
     */
    function _checkAdapterParams(
        uint16 _dstChainId,
        uint16 _pkType,
        bytes memory _adapterParams,
        uint256 _extraGas
    ) internal virtual {
        if (useCustomAdapterParams) {
            _checkGasLimit(_dstChainId, _pkType, _adapterParams, _extraGas);
        } else {
            require(_adapterParams.length == 0, "OFTCore: adapterParams must be empty");
        }
    }

    /**
     * @notice Encode send payload
     */
    function _encodeSendPayload(
        bytes memory _toAddress,
        uint256 _amount
    ) internal pure virtual returns (bytes memory) {
        return abi.encode(PT_SEND, _toAddress, _amount);
    }

    /**
     * @notice Decode send payload
     */
    function _decodeSendPayload(
        bytes memory _payload
    ) internal pure virtual returns (bytes memory toAddress, uint256 amount) {
        (, toAddress, amount) = abi.decode(_payload, (uint16, bytes, uint256));
    }

    /**
     * @notice Convert bytes to address
     */
    function _bytesToAddress(bytes memory _b) internal pure returns (address addr) {
        assembly {
            addr := mload(add(_b, 20))
        }
    }

    /**
     * @notice Debit tokens from sender (burn or lock)
     * @dev Override in derived contracts
     */
    function _debitFrom(
        address _from,
        uint16 _dstChainId,
        bytes memory _toAddress,
        uint256 _amount
    ) internal virtual returns (uint256);

    /**
     * @notice Credit tokens to recipient (mint or unlock)
     * @dev Override in derived contracts
     */
    function _creditTo(
        uint16 _srcChainId,
        address _toAddress,
        uint256 _amount
    ) internal virtual returns (uint256);

    // ============ Admin Functions ============

    /**
     * @notice Enable/disable custom adapter params
     */
    function setUseCustomAdapterParams(bool _useCustomAdapterParams) external onlyOwner {
        useCustomAdapterParams = _useCustomAdapterParams;
        emit SetUseCustomAdapterParams(_useCustomAdapterParams);
    }
}
