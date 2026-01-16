// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

import "./WrappedHTH.sol";

/**
 * @title HTHBridgeExternal
 * @notice Bridge contract for external chains (ETH, BSC, Polygon, etc.)
 * @dev Mints/burns wHTH based on lock/unlock events from HTH native chain
 *
 * Flow:
 * HTH -> wHTH: User locks HTH on native chain -> Relayer calls mint() here
 * wHTH -> HTH: User calls bridge() here -> Burns wHTH -> Relayer unlocks on native chain
 */
contract HTHBridgeExternal {
    // ============ State Variables ============

    WrappedHTH public immutable wHTH;

    address public owner;
    address public relayer;
    bool public paused;

    uint256 public bridgeFee;           // Fee in basis points
    uint256 public minBridgeAmount;
    uint256 public maxBridgeAmount;

    uint256 public constant HTH_CHAIN_ID = 231;

    mapping(bytes32 => bool) public processedTransactions;
    uint256 public nonce;

    // ============ Events ============

    event BridgeToHTH(
        uint256 indexed nonce,
        address indexed sender,
        address indexed recipient,
        uint256 amount,
        uint256 fee,
        uint256 timestamp
    );

    event BridgeFromHTH(
        bytes32 indexed txHash,
        address indexed recipient,
        uint256 amount
    );

    event RelayerUpdated(address indexed oldRelayer, address indexed newRelayer);
    event FeeUpdated(uint256 oldFee, uint256 newFee);
    event Paused(bool isPaused);

    // ============ Modifiers ============

    modifier onlyOwner() {
        require(msg.sender == owner, "Bridge: not owner");
        _;
    }

    modifier onlyRelayer() {
        require(msg.sender == relayer, "Bridge: not relayer");
        _;
    }

    modifier whenNotPaused() {
        require(!paused, "Bridge: paused");
        _;
    }

    // ============ Constructor ============

    constructor(address _wHTH, address _relayer) {
        wHTH = WrappedHTH(_wHTH);
        owner = msg.sender;
        relayer = _relayer;

        bridgeFee = 50;                      // 0.5%
        minBridgeAmount = 100 * 1e8;         // 100 HTH
        maxBridgeAmount = 1_000_000 * 1e8;   // 1M HTH
    }

    // ============ Bridge Functions ============

    /**
     * @notice Bridge wHTH back to native HTH
     * @param amount Amount of wHTH to bridge
     * @param recipient Address to receive HTH on native chain
     */
    function bridge(uint256 amount, address recipient) external whenNotPaused {
        require(amount >= minBridgeAmount, "Bridge: amount too low");
        require(amount <= maxBridgeAmount, "Bridge: amount too high");
        require(recipient != address(0), "Bridge: invalid recipient");
        require(wHTH.balanceOf(msg.sender) >= amount, "Bridge: insufficient balance");

        // Calculate fee
        uint256 fee = (amount * bridgeFee) / 10000;
        uint256 amountAfterFee = amount - fee;

        // Burn the wHTH (user must have approved this contract)
        wHTH.burnFrom(msg.sender, amount, recipient);

        nonce++;

        emit BridgeToHTH(
            nonce,
            msg.sender,
            recipient,
            amountAfterFee,
            fee,
            block.timestamp
        );
    }

    /**
     * @notice Complete bridge from HTH native chain (mint wHTH)
     * @dev Only callable by relayer
     * @param txHash Transaction hash from HTH chain
     * @param recipient Address to receive wHTH
     * @param amount Amount to mint
     */
    function completeBridge(
        bytes32 txHash,
        address recipient,
        uint256 amount
    ) external onlyRelayer whenNotPaused {
        require(!processedTransactions[txHash], "Bridge: already processed");
        require(recipient != address(0), "Bridge: invalid recipient");
        require(amount > 0, "Bridge: invalid amount");

        processedTransactions[txHash] = true;

        // Mint wHTH to recipient
        wHTH.mint(txHash, recipient, amount);

        emit BridgeFromHTH(txHash, recipient, amount);
    }

    // ============ View Functions ============

    function calculateFee(uint256 amount) external view returns (uint256 fee, uint256 amountAfterFee) {
        fee = (amount * bridgeFee) / 10000;
        amountAfterFee = amount - fee;
    }

    // ============ Admin Functions ============

    function setRelayer(address _relayer) external onlyOwner {
        require(_relayer != address(0), "Bridge: invalid relayer");
        emit RelayerUpdated(relayer, _relayer);
        relayer = _relayer;
    }

    function setFee(uint256 _fee) external onlyOwner {
        require(_fee <= 500, "Bridge: fee too high");
        emit FeeUpdated(bridgeFee, _fee);
        bridgeFee = _fee;
    }

    function setLimits(uint256 _min, uint256 _max) external onlyOwner {
        require(_min < _max, "Bridge: invalid limits");
        minBridgeAmount = _min;
        maxBridgeAmount = _max;
    }

    function pause(bool _paused) external onlyOwner {
        paused = _paused;
        emit Paused(_paused);
    }

    function transferOwnership(address newOwner) external onlyOwner {
        require(newOwner != address(0), "Bridge: invalid owner");
        owner = newOwner;
    }
}
