// Copyright (c) 2024 The HTH Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef HTH_SIMPLEVM_H
#define HTH_SIMPLEVM_H

#include <cstdint>
#include <cstring>
#include <vector>
#include <string>
#include <map>

/**
 * SimpleVM - A minimal EVM bytecode executor for read-only contract calls
 *
 * This implements a subset of EVM opcodes sufficient for executing
 * view/pure function calls like name(), symbol(), balanceOf(), etc.
 *
 * For full EVM execution including state changes, EVMone integration
 * should be used instead.
 */

// EVM Opcodes we support
namespace SimpleVM {

// Stop and Arithmetic Operations
constexpr uint8_t OP_STOP = 0x00;
constexpr uint8_t OP_ADD = 0x01;
constexpr uint8_t OP_MUL = 0x02;
constexpr uint8_t OP_SUB = 0x03;
constexpr uint8_t OP_DIV = 0x04;
constexpr uint8_t OP_SDIV = 0x05;
constexpr uint8_t OP_MOD = 0x06;
constexpr uint8_t OP_SMOD = 0x07;
constexpr uint8_t OP_ADDMOD = 0x08;
constexpr uint8_t OP_MULMOD = 0x09;
constexpr uint8_t OP_EXP = 0x0a;
constexpr uint8_t OP_SIGNEXTEND = 0x0b;

// Comparison & Bitwise Logic Operations
constexpr uint8_t OP_LT = 0x10;
constexpr uint8_t OP_GT = 0x11;
constexpr uint8_t OP_SLT = 0x12;
constexpr uint8_t OP_SGT = 0x13;
constexpr uint8_t OP_EQ = 0x14;
constexpr uint8_t OP_ISZERO = 0x15;
constexpr uint8_t OP_AND = 0x16;
constexpr uint8_t OP_OR = 0x17;
constexpr uint8_t OP_XOR = 0x18;
constexpr uint8_t OP_NOT = 0x19;
constexpr uint8_t OP_BYTE = 0x1a;
constexpr uint8_t OP_SHL = 0x1b;
constexpr uint8_t OP_SHR = 0x1c;
constexpr uint8_t OP_SAR = 0x1d;

// SHA3
constexpr uint8_t OP_SHA3 = 0x20;

// Environmental Information
constexpr uint8_t OP_ADDRESS = 0x30;
constexpr uint8_t OP_BALANCE = 0x31;
constexpr uint8_t OP_ORIGIN = 0x32;
constexpr uint8_t OP_CALLER = 0x33;
constexpr uint8_t OP_CALLVALUE = 0x34;
constexpr uint8_t OP_CALLDATALOAD = 0x35;
constexpr uint8_t OP_CALLDATASIZE = 0x36;
constexpr uint8_t OP_CALLDATACOPY = 0x37;
constexpr uint8_t OP_CODESIZE = 0x38;
constexpr uint8_t OP_CODECOPY = 0x39;
constexpr uint8_t OP_GASPRICE = 0x3a;
constexpr uint8_t OP_EXTCODESIZE = 0x3b;
constexpr uint8_t OP_EXTCODECOPY = 0x3c;
constexpr uint8_t OP_RETURNDATASIZE = 0x3d;
constexpr uint8_t OP_RETURNDATACOPY = 0x3e;
constexpr uint8_t OP_EXTCODEHASH = 0x3f;

// Block Information
constexpr uint8_t OP_BLOCKHASH = 0x40;
constexpr uint8_t OP_COINBASE = 0x41;
constexpr uint8_t OP_TIMESTAMP = 0x42;
constexpr uint8_t OP_NUMBER = 0x43;
constexpr uint8_t OP_DIFFICULTY = 0x44;
constexpr uint8_t OP_GASLIMIT = 0x45;
constexpr uint8_t OP_CHAINID = 0x46;
constexpr uint8_t OP_SELFBALANCE = 0x47;

// Stack, Memory, Storage and Flow Operations
constexpr uint8_t OP_POP = 0x50;
constexpr uint8_t OP_MLOAD = 0x51;
constexpr uint8_t OP_MSTORE = 0x52;
constexpr uint8_t OP_MSTORE8 = 0x53;
constexpr uint8_t OP_SLOAD = 0x54;
constexpr uint8_t OP_SSTORE = 0x55;
constexpr uint8_t OP_JUMP = 0x56;
constexpr uint8_t OP_JUMPI = 0x57;
constexpr uint8_t OP_PC = 0x58;
constexpr uint8_t OP_MSIZE = 0x59;
constexpr uint8_t OP_GAS = 0x5a;
constexpr uint8_t OP_JUMPDEST = 0x5b;

// Push operations (PUSH1-PUSH32)
constexpr uint8_t OP_PUSH1 = 0x60;
constexpr uint8_t OP_PUSH32 = 0x7f;

// Duplication operations (DUP1-DUP16)
constexpr uint8_t OP_DUP1 = 0x80;
constexpr uint8_t OP_DUP16 = 0x8f;

// Exchange operations (SWAP1-SWAP16)
constexpr uint8_t OP_SWAP1 = 0x90;
constexpr uint8_t OP_SWAP16 = 0x9f;

// Logging operations
constexpr uint8_t OP_LOG0 = 0xa0;
constexpr uint8_t OP_LOG4 = 0xa4;

// System operations
constexpr uint8_t OP_CREATE = 0xf0;
constexpr uint8_t OP_CALL = 0xf1;
constexpr uint8_t OP_CALLCODE = 0xf2;
constexpr uint8_t OP_RETURN = 0xf3;
constexpr uint8_t OP_DELEGATECALL = 0xf4;
constexpr uint8_t OP_CREATE2 = 0xf5;
constexpr uint8_t OP_STATICCALL = 0xfa;
constexpr uint8_t OP_REVERT = 0xfd;
constexpr uint8_t OP_INVALID = 0xfe;
constexpr uint8_t OP_SELFDESTRUCT = 0xff;

// 256-bit word type (stored as 32 bytes, big-endian)
struct Word256 {
    uint8_t bytes[32];

    Word256() { memset(bytes, 0, 32); }
    Word256(uint64_t value);
    Word256(const std::vector<unsigned char>& data);

    bool isZero() const;
    uint64_t toUint64() const;
    std::vector<unsigned char> toBytes() const;

    static Word256 fromBigEndian(const uint8_t* data, size_t len);
};

// Execution result
struct ExecutionResult {
    bool success;
    std::vector<unsigned char> output;
    uint64_t gasUsed;
    std::string error;

    ExecutionResult() : success(false), gasUsed(0) {}
};

// Storage callback interface
class StorageInterface {
public:
    virtual ~StorageInterface() = default;
    virtual std::vector<unsigned char> getStorage(const std::vector<unsigned char>& key) = 0;
    virtual void setStorage(const std::vector<unsigned char>& key, const std::vector<unsigned char>& value) = 0;
};

// VM execution context
class VMContext {
public:
    std::vector<unsigned char> contractAddress;
    std::vector<unsigned char> callerAddress;
    std::vector<unsigned char> originAddress;
    std::vector<unsigned char> callData;
    std::vector<unsigned char> code;
    uint64_t gasLimit;
    uint64_t value;
    uint64_t blockNumber;
    uint64_t timestamp;
    uint64_t gasPrice;
    StorageInterface* storage;
    bool readOnly;  // If true, SSTORE will fail

    VMContext() : gasLimit(0), value(0), blockNumber(0), timestamp(0), gasPrice(0), storage(nullptr), readOnly(true) {}
};

// Execute bytecode
ExecutionResult Execute(const VMContext& ctx);

// Execute a specific function call (helper)
ExecutionResult CallContract(
    const std::vector<unsigned char>& contractAddress,
    const std::vector<unsigned char>& code,
    const std::vector<unsigned char>& callData,
    StorageInterface* storage,
    uint64_t gasLimit = 100000,
    bool readOnly = true);

} // namespace SimpleVM

#endif // HTH_SIMPLEVM_H
