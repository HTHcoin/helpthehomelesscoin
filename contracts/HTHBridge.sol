// SPDX-License-Identifier: MIT
pragma solidity ^0.8.19;

/**
 * @title HTHBridge
 * @notice Bridge contract for cross-chain HTH transfers
 * @dev Deployed on HTH native chain - locks HTH for bridging to other chains
 *
 * Architecture:
 * - Native HTH Chain: This contract locks/unlocks native HTH
 * - Other Chains: Wrapped HTH (wHTH) is minted/burned by partner bridge contracts
 * - Relayers monitor events and trigger corresponding actions on destination chain
 */
contract HTHBridge {
    // ============ State Variables ============

    address public owner;
    address public relayer;
    bool public paused;

    uint256 public bridgeFee;           // Fee in basis points (100 = 1%)
    uint256 public minBridgeAmount;     // Minimum amount to bridge
    uint256 public maxBridgeAmount;     // Maximum amount per transaction
    uint256 public dailyLimit;          // Daily bridge limit
    uint256 public dailyBridged;        // Amount bridged today
    uint256 public lastResetTime;       // Last daily reset timestamp

    uint256 public nonce;               // Unique identifier for each bridge tx

    mapping(bytes32 => bool) public processedTransactions;  // Prevent replay attacks
    mapping(uint256 => bool) public supportedChains;        // Supported destination chains
    mapping(address => uint256) public userDailyBridged;    // Per-user daily limit tracking
    mapping(address => uint256) public userLastReset;       // Per-user reset timestamp

    uint256 public userDailyLimit;      // Per-user daily limit

    // ============ Events ============

    event BridgeInitiated(
        uint256 indexed nonce,
        address indexed sender,
        address indexed recipient,
        uint256 amount,
        uint256 fee,
        uint256 destinationChainId,
        uint256 timestamp
    );

    event BridgeCompleted(
        bytes32 indexed txHash,
        address indexed recipient,
        uint256 amount,
        uint256 sourceChainId
    );

    event RelayerUpdated(address indexed oldRelayer, address indexed newRelayer);
    event OwnerUpdated(address indexed oldOwner, address indexed newOwner);
    event FeeUpdated(uint256 oldFee, uint256 newFee);
    event LimitsUpdated(uint256 minAmount, uint256 maxAmount, uint256 dailyLimit);
    event ChainSupported(uint256 chainId, bool supported);
    event Paused(bool isPaused);
    event EmergencyWithdraw(address indexed to, uint256 amount);

    // ============ Modifiers ============

    modifier onlyOwner() {
        require(msg.sender == owner, "Bridge: not owner");
        _;
    }

    modifier onlyRelayer() {
        require(msg.sender == relayer, "Bridge: not relayer");
        _;
    }

    modifier onlyOwnerOrRelayer() {
        require(msg.sender == owner || msg.sender == relayer, "Bridge: not authorized");
        _;
    }

    modifier whenNotPaused() {
        require(!paused, "Bridge: paused");
        _;
    }

    modifier nonReentrant() {
        require(nonce == nonce, "Bridge: reentrant call");
        _;
    }

    // ============ Constructor ============

    constructor(address _relayer) {
        owner = msg.sender;
        relayer = _relayer;

        bridgeFee = 50;                          // 0.5% fee
        minBridgeAmount = 100 * 1e8;             // 100 HTH minimum
        maxBridgeAmount = 1_000_000 * 1e8;       // 1M HTH max per tx
        dailyLimit = 10_000_000 * 1e8;           // 10M HTH daily limit
        userDailyLimit = 500_000 * 1e8;          // 500K HTH per user daily
        lastResetTime = block.timestamp;

        // Support common chains by default
        supportedChains[1] = true;       // Ethereum Mainnet
        supportedChains[56] = true;      // BSC
        supportedChains[137] = true;     // Polygon
        supportedChains[42161] = true;   // Arbitrum
        supportedChains[10] = true;      // Optimism
        supportedChains[43114] = true;   // Avalanche
    }

    // ============ Bridge Functions ============

    /**
     * @notice Bridge HTH to another chain
     * @param recipient Address to receive tokens on destination chain
     * @param destinationChainId Target chain ID
     */
    function bridge(
        address recipient,
        uint256 destinationChainId
    ) external payable whenNotPaused nonReentrant {
        require(msg.value >= minBridgeAmount, "Bridge: amount too low");
        require(msg.value <= maxBridgeAmount, "Bridge: amount too high");
        require(supportedChains[destinationChainId], "Bridge: chain not supported");
        require(recipient != address(0), "Bridge: invalid recipient");

        // Reset daily limits if needed
        _resetDailyLimits();
        _resetUserDailyLimit(msg.sender);

        // Check limits
        require(dailyBridged + msg.value <= dailyLimit, "Bridge: daily limit exceeded");
        require(
            userDailyBridged[msg.sender] + msg.value <= userDailyLimit,
            "Bridge: user daily limit exceeded"
        );

        // Calculate fee
        uint256 fee = (msg.value * bridgeFee) / 10000;
        uint256 amountAfterFee = msg.value - fee;

        // Update state
        dailyBridged += msg.value;
        userDailyBridged[msg.sender] += msg.value;
        nonce++;

        emit BridgeInitiated(
            nonce,
            msg.sender,
            recipient,
            amountAfterFee,
            fee,
            destinationChainId,
            block.timestamp
        );
    }

    /**
     * @notice Complete a bridge from another chain (release locked HTH)
     * @param txHash Unique transaction hash from source chain
     * @param recipient Address to receive HTH
     * @param amount Amount to release
     * @param sourceChainId Chain ID where bridge was initiated
     */
    function completeBridge(
        bytes32 txHash,
        address payable recipient,
        uint256 amount,
        uint256 sourceChainId
    ) external onlyRelayer whenNotPaused nonReentrant {
        require(!processedTransactions[txHash], "Bridge: already processed");
        require(recipient != address(0), "Bridge: invalid recipient");
        require(amount > 0, "Bridge: invalid amount");
        require(address(this).balance >= amount, "Bridge: insufficient liquidity");

        processedTransactions[txHash] = true;

        (bool success, ) = recipient.call{value: amount}("");
        require(success, "Bridge: transfer failed");

        emit BridgeCompleted(txHash, recipient, amount, sourceChainId);
    }

    // ============ View Functions ============

    /**
     * @notice Calculate bridge fee for an amount
     * @param amount Amount to bridge
     * @return fee Fee amount
     * @return amountAfterFee Amount recipient will receive
     */
    function calculateFee(uint256 amount) external view returns (uint256 fee, uint256 amountAfterFee) {
        fee = (amount * bridgeFee) / 10000;
        amountAfterFee = amount - fee;
    }

    /**
     * @notice Get remaining daily bridge capacity
     */
    function remainingDailyLimit() external view returns (uint256) {
        if (block.timestamp >= lastResetTime + 1 days) {
            return dailyLimit;
        }
        return dailyLimit > dailyBridged ? dailyLimit - dailyBridged : 0;
    }

    /**
     * @notice Get user's remaining daily limit
     */
    function userRemainingLimit(address user) external view returns (uint256) {
        if (block.timestamp >= userLastReset[user] + 1 days) {
            return userDailyLimit;
        }
        return userDailyLimit > userDailyBridged[user] ? userDailyLimit - userDailyBridged[user] : 0;
    }

    /**
     * @notice Check if a chain is supported
     */
    function isChainSupported(uint256 chainId) external view returns (bool) {
        return supportedChains[chainId];
    }

    /**
     * @notice Get bridge liquidity (contract balance)
     */
    function liquidity() external view returns (uint256) {
        return address(this).balance;
    }

    // ============ Admin Functions ============

    function setRelayer(address _relayer) external onlyOwner {
        require(_relayer != address(0), "Bridge: invalid relayer");
        emit RelayerUpdated(relayer, _relayer);
        relayer = _relayer;
    }

    function transferOwnership(address newOwner) external onlyOwner {
        require(newOwner != address(0), "Bridge: invalid owner");
        emit OwnerUpdated(owner, newOwner);
        owner = newOwner;
    }

    function setFee(uint256 _fee) external onlyOwner {
        require(_fee <= 500, "Bridge: fee too high"); // Max 5%
        emit FeeUpdated(bridgeFee, _fee);
        bridgeFee = _fee;
    }

    function setLimits(
        uint256 _minAmount,
        uint256 _maxAmount,
        uint256 _dailyLimit,
        uint256 _userDailyLimit
    ) external onlyOwner {
        require(_minAmount < _maxAmount, "Bridge: invalid limits");
        minBridgeAmount = _minAmount;
        maxBridgeAmount = _maxAmount;
        dailyLimit = _dailyLimit;
        userDailyLimit = _userDailyLimit;
        emit LimitsUpdated(_minAmount, _maxAmount, _dailyLimit);
    }

    function setSupportedChain(uint256 chainId, bool supported) external onlyOwner {
        supportedChains[chainId] = supported;
        emit ChainSupported(chainId, supported);
    }

    function pause(bool _paused) external onlyOwnerOrRelayer {
        paused = _paused;
        emit Paused(_paused);
    }

    /**
     * @notice Emergency withdraw (only owner, use with caution)
     */
    function emergencyWithdraw(address payable to, uint256 amount) external onlyOwner {
        require(to != address(0), "Bridge: invalid address");
        require(amount <= address(this).balance, "Bridge: insufficient balance");

        (bool success, ) = to.call{value: amount}("");
        require(success, "Bridge: withdraw failed");

        emit EmergencyWithdraw(to, amount);
    }

    // ============ Internal Functions ============

    function _resetDailyLimits() internal {
        if (block.timestamp >= lastResetTime + 1 days) {
            dailyBridged = 0;
            lastResetTime = block.timestamp;
        }
    }

    function _resetUserDailyLimit(address user) internal {
        if (block.timestamp >= userLastReset[user] + 1 days) {
            userDailyBridged[user] = 0;
            userLastReset[user] = block.timestamp;
        }
    }

    // ============ Receive Function ============

    /**
     * @notice Add liquidity to the bridge
     */
    receive() external payable {
        // Accept HTH deposits for liquidity
    }
}
