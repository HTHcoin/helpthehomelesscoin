// SPDX-License-Identifier: MIT
pragma solidity ^0.8.19;

/**
 * @title WrappedHTH (wHTH)
 * @notice Wrapped HTH token for deployment on external chains (ETH, BSC, etc.)
 * @dev Minted when HTH is locked on native chain, burned when bridging back
 *
 * Deployment:
 * - Deploy on Ethereum, BSC, Polygon, etc.
 * - Bridge contract has mint/burn authority
 * - 1:1 backed by locked HTH on native chain
 */
contract WrappedHTH {
    // ============ ERC20 State ============

    string public constant name = "Wrapped HTH";
    string public constant symbol = "wHTH";
    uint8 public constant decimals = 8;  // Match native HTH decimals

    uint256 public totalSupply;
    mapping(address => uint256) public balanceOf;
    mapping(address => mapping(address => uint256)) public allowance;

    // ============ Bridge State ============

    address public owner;
    address public bridge;
    bool public paused;

    uint256 public constant HTH_NATIVE_CHAIN_ID = 231;  // HTH chain ID

    mapping(bytes32 => bool) public processedMints;     // Prevent replay
    uint256 public nonce;

    // ============ Events ============

    event Transfer(address indexed from, address indexed to, uint256 value);
    event Approval(address indexed owner, address indexed spender, uint256 value);

    event BridgeUpdated(address indexed oldBridge, address indexed newBridge);
    event Minted(bytes32 indexed txHash, address indexed to, uint256 amount);
    event Burned(address indexed from, address indexed recipient, uint256 amount, uint256 nonce);
    event Paused(bool isPaused);

    // ============ Modifiers ============

    modifier onlyOwner() {
        require(msg.sender == owner, "wHTH: not owner");
        _;
    }

    modifier onlyBridge() {
        require(msg.sender == bridge, "wHTH: not bridge");
        _;
    }

    modifier whenNotPaused() {
        require(!paused, "wHTH: paused");
        _;
    }

    // ============ Constructor ============

    constructor(address _bridge) {
        owner = msg.sender;
        bridge = _bridge;
    }

    // ============ ERC20 Functions ============

    function transfer(address to, uint256 value) external returns (bool) {
        require(to != address(0), "wHTH: transfer to zero");
        require(balanceOf[msg.sender] >= value, "wHTH: insufficient balance");

        balanceOf[msg.sender] -= value;
        balanceOf[to] += value;

        emit Transfer(msg.sender, to, value);
        return true;
    }

    function approve(address spender, uint256 value) external returns (bool) {
        allowance[msg.sender][spender] = value;
        emit Approval(msg.sender, spender, value);
        return true;
    }

    function transferFrom(address from, address to, uint256 value) external returns (bool) {
        require(to != address(0), "wHTH: transfer to zero");
        require(balanceOf[from] >= value, "wHTH: insufficient balance");
        require(allowance[from][msg.sender] >= value, "wHTH: insufficient allowance");

        balanceOf[from] -= value;
        balanceOf[to] += value;
        allowance[from][msg.sender] -= value;

        emit Transfer(from, to, value);
        return true;
    }

    // ============ Bridge Functions ============

    /**
     * @notice Mint wHTH when HTH is locked on native chain
     * @dev Only callable by bridge contract
     * @param txHash Unique transaction hash from HTH chain
     * @param to Recipient address
     * @param amount Amount to mint
     */
    function mint(
        bytes32 txHash,
        address to,
        uint256 amount
    ) external onlyBridge whenNotPaused {
        require(!processedMints[txHash], "wHTH: already minted");
        require(to != address(0), "wHTH: mint to zero");
        require(amount > 0, "wHTH: zero amount");

        processedMints[txHash] = true;
        totalSupply += amount;
        balanceOf[to] += amount;

        emit Transfer(address(0), to, amount);
        emit Minted(txHash, to, amount);
    }

    /**
     * @notice Burn wHTH to bridge back to native HTH chain
     * @param amount Amount to burn
     * @param recipient Address to receive HTH on native chain
     */
    function burn(uint256 amount, address recipient) external whenNotPaused {
        require(balanceOf[msg.sender] >= amount, "wHTH: insufficient balance");
        require(recipient != address(0), "wHTH: invalid recipient");
        require(amount > 0, "wHTH: zero amount");

        balanceOf[msg.sender] -= amount;
        totalSupply -= amount;
        nonce++;

        emit Transfer(msg.sender, address(0), amount);
        emit Burned(msg.sender, recipient, amount, nonce);
    }

    /**
     * @notice Burn wHTH from allowance (for bridge contract)
     */
    function burnFrom(address from, uint256 amount, address recipient) external onlyBridge whenNotPaused {
        require(balanceOf[from] >= amount, "wHTH: insufficient balance");
        require(allowance[from][msg.sender] >= amount, "wHTH: insufficient allowance");
        require(recipient != address(0), "wHTH: invalid recipient");

        balanceOf[from] -= amount;
        allowance[from][msg.sender] -= amount;
        totalSupply -= amount;
        nonce++;

        emit Transfer(from, address(0), amount);
        emit Burned(from, recipient, amount, nonce);
    }

    // ============ Admin Functions ============

    function setBridge(address _bridge) external onlyOwner {
        require(_bridge != address(0), "wHTH: invalid bridge");
        emit BridgeUpdated(bridge, _bridge);
        bridge = _bridge;
    }

    function transferOwnership(address newOwner) external onlyOwner {
        require(newOwner != address(0), "wHTH: invalid owner");
        owner = newOwner;
    }

    function pause(bool _paused) external onlyOwner {
        paused = _paused;
        emit Paused(_paused);
    }
}
