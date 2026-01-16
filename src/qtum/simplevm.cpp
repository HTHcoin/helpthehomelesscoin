// Copyright (c) 2024 The HTH Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "qtum/simplevm.h"
#include "crypto/sha256.h"

extern "C" {
#include "crypto/sph_keccak.h"
}

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <stack>

namespace SimpleVM {

// Word256 implementation
Word256::Word256(uint64_t value) {
    memset(bytes, 0, 32);
    // Store as big-endian
    for (int i = 31; i >= 24 && value > 0; i--) {
        bytes[i] = value & 0xFF;
        value >>= 8;
    }
}

Word256::Word256(const std::vector<unsigned char>& data) {
    memset(bytes, 0, 32);
    size_t offset = 32 - std::min(data.size(), (size_t)32);
    memcpy(bytes + offset, data.data(), std::min(data.size(), (size_t)32));
}

bool Word256::isZero() const {
    for (int i = 0; i < 32; i++) {
        if (bytes[i] != 0) return false;
    }
    return true;
}

uint64_t Word256::toUint64() const {
    uint64_t result = 0;
    for (int i = 24; i < 32; i++) {
        result = (result << 8) | bytes[i];
    }
    return result;
}

std::vector<unsigned char> Word256::toBytes() const {
    return std::vector<unsigned char>(bytes, bytes + 32);
}

Word256 Word256::fromBigEndian(const uint8_t* data, size_t len) {
    Word256 w;
    size_t offset = 32 - std::min(len, (size_t)32);
    memcpy(w.bytes + offset, data, std::min(len, (size_t)32));
    return w;
}

// VM implementation
class VM {
private:
    std::vector<Word256> stack;
    std::vector<uint8_t> memory;
    const VMContext& ctx;
    size_t pc;
    uint64_t gasUsed;
    std::vector<unsigned char> returnData;
    bool stopped;
    std::string error;

    static const size_t MAX_STACK_SIZE = 1024;
    static const size_t MAX_MEMORY_SIZE = 1024 * 1024; // 1MB

public:
    VM(const VMContext& context) : ctx(context), pc(0), gasUsed(0), stopped(false) {}

    void push(const Word256& w) {
        if (stack.size() >= MAX_STACK_SIZE) {
            throw std::runtime_error("Stack overflow");
        }
        stack.push_back(w);
    }

    Word256 pop() {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow");
        }
        Word256 w = stack.back();
        stack.pop_back();
        return w;
    }

    Word256& peek(size_t depth = 0) {
        if (depth >= stack.size()) {
            throw std::runtime_error("Stack underflow");
        }
        return stack[stack.size() - 1 - depth];
    }

    void useGas(uint64_t amount) {
        gasUsed += amount;
        if (gasUsed > ctx.gasLimit) {
            throw std::runtime_error("Out of gas");
        }
    }

    void expandMemory(size_t size) {
        if (size > memory.size()) {
            if (size > MAX_MEMORY_SIZE) {
                throw std::runtime_error("Memory limit exceeded");
            }
            // Gas cost for memory expansion
            size_t words = (size + 31) / 32;
            size_t oldWords = (memory.size() + 31) / 32;
            if (words > oldWords) {
                uint64_t cost = 3 * (words - oldWords) + (words * words / 512) - (oldWords * oldWords / 512);
                useGas(cost);
            }
            memory.resize(size, 0);
        }
    }

    void mstore(size_t offset, const Word256& value) {
        expandMemory(offset + 32);
        memcpy(memory.data() + offset, value.bytes, 32);
    }

    Word256 mload(size_t offset) {
        expandMemory(offset + 32);
        Word256 w;
        memcpy(w.bytes, memory.data() + offset, 32);
        return w;
    }

    void mstore8(size_t offset, uint8_t value) {
        expandMemory(offset + 1);
        memory[offset] = value;
    }

    std::vector<unsigned char> getMemorySlice(size_t offset, size_t size) {
        if (size == 0) return {};
        expandMemory(offset + size);
        return std::vector<unsigned char>(memory.begin() + offset, memory.begin() + offset + size);
    }

    void setMemorySlice(size_t offset, const std::vector<unsigned char>& data) {
        if (data.empty()) return;
        expandMemory(offset + data.size());
        memcpy(memory.data() + offset, data.data(), data.size());
    }

    // Keccak256 hash (used by EVM SHA3 opcode)
    std::vector<unsigned char> keccak256(const std::vector<unsigned char>& data) {
        sph_keccak256_context ctx;
        std::vector<unsigned char> hash(32);
        sph_keccak256_init(&ctx);
        sph_keccak256(&ctx, data.data(), data.size());
        sph_keccak256_close(&ctx, hash.data());
        return hash;
    }

    ExecutionResult run() {
        ExecutionResult result;
        result.success = false;

        try {
            while (!stopped && pc < ctx.code.size()) {
                uint8_t opcode = ctx.code[pc];
                pc++;

                // Base gas cost
                useGas(3);

                switch (opcode) {
                    case OP_STOP:
                        stopped = true;
                        result.success = true;
                        break;

                    case OP_ADD: {
                        Word256 a = pop();
                        Word256 b = pop();
                        Word256 c;
                        uint64_t carry = 0;
                        for (int i = 31; i >= 0; i--) {
                            uint64_t sum = (uint64_t)a.bytes[i] + b.bytes[i] + carry;
                            c.bytes[i] = sum & 0xFF;
                            carry = sum >> 8;
                        }
                        push(c);
                        break;
                    }

                    case OP_MUL: {
                        Word256 a = pop();
                        Word256 b = pop();
                        // Simplified: only handle small values
                        uint64_t va = a.toUint64();
                        uint64_t vb = b.toUint64();
                        push(Word256(va * vb));
                        break;
                    }

                    case OP_SUB: {
                        Word256 a = pop();
                        Word256 b = pop();
                        Word256 c;
                        int64_t borrow = 0;
                        for (int i = 31; i >= 0; i--) {
                            int64_t diff = (int64_t)a.bytes[i] - b.bytes[i] - borrow;
                            if (diff < 0) {
                                diff += 256;
                                borrow = 1;
                            } else {
                                borrow = 0;
                            }
                            c.bytes[i] = (uint8_t)diff;
                        }
                        push(c);
                        break;
                    }

                    case OP_DIV: {
                        Word256 a = pop();
                        Word256 b = pop();
                        if (b.isZero()) {
                            push(Word256(0));
                        } else {
                            push(Word256(a.toUint64() / b.toUint64()));
                        }
                        break;
                    }

                    case OP_MOD: {
                        Word256 a = pop();
                        Word256 b = pop();
                        if (b.isZero()) {
                            push(Word256(0));
                        } else {
                            push(Word256(a.toUint64() % b.toUint64()));
                        }
                        break;
                    }

                    case OP_SDIV: {
                        // Signed division
                        Word256 a = pop();
                        Word256 b = pop();
                        if (b.isZero()) {
                            push(Word256(0));
                        } else {
                            // Check signs (MSB of first byte)
                            bool aSign = (a.bytes[0] & 0x80) != 0;
                            bool bSign = (b.bytes[0] & 0x80) != 0;

                            // Convert to positive if negative (two's complement)
                            Word256 aAbs = a, bAbs = b;
                            if (aSign) {
                                // Negate: flip bits and add 1
                                for (int i = 0; i < 32; i++) aAbs.bytes[i] = ~aAbs.bytes[i];
                                uint64_t carry = 1;
                                for (int i = 31; i >= 0 && carry; i--) {
                                    uint64_t sum = aAbs.bytes[i] + carry;
                                    aAbs.bytes[i] = sum & 0xFF;
                                    carry = sum >> 8;
                                }
                            }
                            if (bSign) {
                                for (int i = 0; i < 32; i++) bAbs.bytes[i] = ~bAbs.bytes[i];
                                uint64_t carry = 1;
                                for (int i = 31; i >= 0 && carry; i--) {
                                    uint64_t sum = bAbs.bytes[i] + carry;
                                    bAbs.bytes[i] = sum & 0xFF;
                                    carry = sum >> 8;
                                }
                            }

                            uint64_t result = aAbs.toUint64() / bAbs.toUint64();
                            Word256 r(result);

                            // Result is negative if signs differ
                            if (aSign != bSign && result != 0) {
                                for (int i = 0; i < 32; i++) r.bytes[i] = ~r.bytes[i];
                                uint64_t carry = 1;
                                for (int i = 31; i >= 0 && carry; i--) {
                                    uint64_t sum = r.bytes[i] + carry;
                                    r.bytes[i] = sum & 0xFF;
                                    carry = sum >> 8;
                                }
                            }
                            push(r);
                        }
                        break;
                    }

                    case OP_SMOD: {
                        // Signed modulo
                        Word256 a = pop();
                        Word256 b = pop();
                        if (b.isZero()) {
                            push(Word256(0));
                        } else {
                            bool aSign = (a.bytes[0] & 0x80) != 0;
                            bool bSign = (b.bytes[0] & 0x80) != 0;

                            Word256 aAbs = a, bAbs = b;
                            if (aSign) {
                                for (int i = 0; i < 32; i++) aAbs.bytes[i] = ~aAbs.bytes[i];
                                uint64_t carry = 1;
                                for (int i = 31; i >= 0 && carry; i--) {
                                    uint64_t sum = aAbs.bytes[i] + carry;
                                    aAbs.bytes[i] = sum & 0xFF;
                                    carry = sum >> 8;
                                }
                            }
                            if (bSign) {
                                for (int i = 0; i < 32; i++) bAbs.bytes[i] = ~bAbs.bytes[i];
                                uint64_t carry = 1;
                                for (int i = 31; i >= 0 && carry; i--) {
                                    uint64_t sum = bAbs.bytes[i] + carry;
                                    bAbs.bytes[i] = sum & 0xFF;
                                    carry = sum >> 8;
                                }
                            }

                            uint64_t result = aAbs.toUint64() % bAbs.toUint64();
                            Word256 r(result);

                            // Result has same sign as dividend
                            if (aSign && result != 0) {
                                for (int i = 0; i < 32; i++) r.bytes[i] = ~r.bytes[i];
                                uint64_t carry = 1;
                                for (int i = 31; i >= 0 && carry; i--) {
                                    uint64_t sum = r.bytes[i] + carry;
                                    r.bytes[i] = sum & 0xFF;
                                    carry = sum >> 8;
                                }
                            }
                            push(r);
                        }
                        break;
                    }

                    case OP_ADDMOD: {
                        // (a + b) % N
                        Word256 a = pop();
                        Word256 b = pop();
                        Word256 n = pop();
                        if (n.isZero()) {
                            push(Word256(0));
                        } else {
                            // Simplified for small values
                            uint64_t va = a.toUint64();
                            uint64_t vb = b.toUint64();
                            uint64_t vn = n.toUint64();
                            push(Word256((va + vb) % vn));
                        }
                        break;
                    }

                    case OP_MULMOD: {
                        // (a * b) % N
                        Word256 a = pop();
                        Word256 b = pop();
                        Word256 n = pop();
                        if (n.isZero()) {
                            push(Word256(0));
                        } else {
                            uint64_t va = a.toUint64();
                            uint64_t vb = b.toUint64();
                            uint64_t vn = n.toUint64();
                            push(Word256((va * vb) % vn));
                        }
                        break;
                    }

                    case OP_EXP: {
                        useGas(7); // EXP base cost
                        Word256 base = pop();
                        Word256 exp = pop();
                        uint64_t b = base.toUint64();
                        uint64_t e = exp.toUint64();

                        // Count bytes in exponent for gas
                        uint64_t expBytes = 0;
                        for (int i = 0; i < 32; i++) {
                            if (exp.bytes[i] != 0) {
                                expBytes = 32 - i;
                                break;
                            }
                        }
                        useGas(50 * expBytes);

                        // Compute power (simplified for 64-bit)
                        uint64_t result = 1;
                        while (e > 0) {
                            if (e & 1) result *= b;
                            b *= b;
                            e >>= 1;
                        }
                        push(Word256(result));
                        break;
                    }

                    case OP_SIGNEXTEND: {
                        // Sign extend value from (k+1) bytes to 32 bytes
                        Word256 k = pop();
                        Word256 value = pop();
                        uint64_t kVal = k.toUint64();

                        if (kVal < 31) {
                            // The sign bit is at byte position (31 - kVal)
                            size_t signByteIdx = 31 - kVal;
                            bool signBit = (value.bytes[signByteIdx] & 0x80) != 0;

                            // Extend the sign to all higher bytes
                            uint8_t extendByte = signBit ? 0xFF : 0x00;
                            for (size_t i = 0; i < signByteIdx; i++) {
                                value.bytes[i] = extendByte;
                            }
                        }
                        push(value);
                        break;
                    }

                    case OP_LT: {
                        Word256 a = pop();
                        Word256 b = pop();
                        bool lt = false;
                        for (int i = 0; i < 32; i++) {
                            if (a.bytes[i] < b.bytes[i]) { lt = true; break; }
                            if (a.bytes[i] > b.bytes[i]) { lt = false; break; }
                        }
                        push(Word256(lt ? 1 : 0));
                        break;
                    }

                    case OP_GT: {
                        Word256 a = pop();
                        Word256 b = pop();
                        bool gt = false;
                        for (int i = 0; i < 32; i++) {
                            if (a.bytes[i] > b.bytes[i]) { gt = true; break; }
                            if (a.bytes[i] < b.bytes[i]) { gt = false; break; }
                        }
                        push(Word256(gt ? 1 : 0));
                        break;
                    }

                    case OP_SLT: {
                        // Signed less than
                        Word256 a = pop();
                        Word256 b = pop();
                        bool aSign = (a.bytes[0] & 0x80) != 0;
                        bool bSign = (b.bytes[0] & 0x80) != 0;

                        bool slt;
                        if (aSign != bSign) {
                            // If signs differ, negative number is smaller
                            slt = aSign;
                        } else {
                            // Same sign: compare as unsigned
                            slt = false;
                            for (int i = 0; i < 32; i++) {
                                if (a.bytes[i] < b.bytes[i]) { slt = true; break; }
                                if (a.bytes[i] > b.bytes[i]) { slt = false; break; }
                            }
                        }
                        push(Word256(slt ? 1 : 0));
                        break;
                    }

                    case OP_SGT: {
                        // Signed greater than
                        Word256 a = pop();
                        Word256 b = pop();
                        bool aSign = (a.bytes[0] & 0x80) != 0;
                        bool bSign = (b.bytes[0] & 0x80) != 0;

                        bool sgt;
                        if (aSign != bSign) {
                            // If signs differ, positive number is greater
                            sgt = bSign;
                        } else {
                            // Same sign: compare as unsigned
                            sgt = false;
                            for (int i = 0; i < 32; i++) {
                                if (a.bytes[i] > b.bytes[i]) { sgt = true; break; }
                                if (a.bytes[i] < b.bytes[i]) { sgt = false; break; }
                            }
                        }
                        push(Word256(sgt ? 1 : 0));
                        break;
                    }

                    case OP_EQ: {
                        Word256 a = pop();
                        Word256 b = pop();
                        bool eq = (memcmp(a.bytes, b.bytes, 32) == 0);
                        push(Word256(eq ? 1 : 0));
                        break;
                    }

                    case OP_ISZERO: {
                        Word256 a = pop();
                        push(Word256(a.isZero() ? 1 : 0));
                        break;
                    }

                    case OP_AND: {
                        Word256 a = pop();
                        Word256 b = pop();
                        Word256 c;
                        for (int i = 0; i < 32; i++) {
                            c.bytes[i] = a.bytes[i] & b.bytes[i];
                        }
                        push(c);
                        break;
                    }

                    case OP_OR: {
                        Word256 a = pop();
                        Word256 b = pop();
                        Word256 c;
                        for (int i = 0; i < 32; i++) {
                            c.bytes[i] = a.bytes[i] | b.bytes[i];
                        }
                        push(c);
                        break;
                    }

                    case OP_XOR: {
                        Word256 a = pop();
                        Word256 b = pop();
                        Word256 c;
                        for (int i = 0; i < 32; i++) {
                            c.bytes[i] = a.bytes[i] ^ b.bytes[i];
                        }
                        push(c);
                        break;
                    }

                    case OP_NOT: {
                        Word256 a = pop();
                        Word256 c;
                        for (int i = 0; i < 32; i++) {
                            c.bytes[i] = ~a.bytes[i];
                        }
                        push(c);
                        break;
                    }

                    case OP_BYTE: {
                        Word256 i = pop();
                        Word256 x = pop();
                        uint64_t idx = i.toUint64();
                        if (idx < 32) {
                            push(Word256(x.bytes[idx]));
                        } else {
                            push(Word256(0));
                        }
                        break;
                    }

                    case OP_SHL: {
                        // Shift left - works on full 256 bits
                        Word256 shift = pop();
                        Word256 value = pop();
                        uint64_t s = shift.toUint64();
                        if (s >= 256) {
                            push(Word256(0));
                        } else if (s == 0) {
                            push(value);
                        } else {
                            Word256 result;
                            size_t byteShift = s / 8;
                            size_t bitShift = s % 8;

                            // First do byte-level shift
                            for (size_t i = 0; i + byteShift < 32; i++) {
                                result.bytes[i] = value.bytes[i + byteShift];
                            }

                            // Then do bit-level shift within bytes
                            if (bitShift > 0) {
                                for (size_t i = 0; i < 32; i++) {
                                    uint8_t current = result.bytes[i];
                                    uint8_t next = (i < 31) ? result.bytes[i + 1] : 0;
                                    result.bytes[i] = (current << bitShift) | (next >> (8 - bitShift));
                                }
                            }
                            push(result);
                        }
                        break;
                    }

                    case OP_SHR: {
                        // Shift right - works on full 256 bits
                        Word256 shift = pop();
                        Word256 value = pop();
                        uint64_t s = shift.toUint64();
                        if (s >= 256) {
                            push(Word256(0));
                        } else if (s == 0) {
                            push(value);
                        } else {
                            Word256 result;
                            size_t byteShift = s / 8;
                            size_t bitShift = s % 8;

                            // First do byte-level shift (shift bytes to the right)
                            for (size_t i = byteShift; i < 32; i++) {
                                result.bytes[i] = value.bytes[i - byteShift];
                            }

                            // Then do bit-level shift within bytes
                            if (bitShift > 0) {
                                for (int i = 31; i >= 0; i--) {
                                    uint8_t current = result.bytes[i];
                                    uint8_t prev = (i > 0) ? result.bytes[i - 1] : 0;
                                    result.bytes[i] = (current >> bitShift) | (prev << (8 - bitShift));
                                }
                            }
                            push(result);
                        }
                        break;
                    }

                    case OP_SAR: {
                        // Signed arithmetic right shift
                        Word256 shift = pop();
                        Word256 value = pop();
                        uint64_t s = shift.toUint64();
                        bool signBit = (value.bytes[0] & 0x80) != 0;

                        if (s >= 256) {
                            // All bits shifted out - result is all 0s or all 1s based on sign
                            Word256 result;
                            memset(result.bytes, signBit ? 0xFF : 0x00, 32);
                            push(result);
                        } else if (s == 0) {
                            push(value);
                        } else {
                            // Perform byte-level and bit-level shifts
                            Word256 result;
                            size_t byteShift = s / 8;
                            size_t bitShift = s % 8;

                            // Fill with sign extension
                            memset(result.bytes, signBit ? 0xFF : 0x00, 32);

                            // Shift bytes
                            for (size_t i = 0; i + byteShift < 32; i++) {
                                result.bytes[i + byteShift] = value.bytes[i];
                            }

                            // Shift remaining bits
                            if (bitShift > 0) {
                                for (int i = 31; i >= 0; i--) {
                                    uint8_t current = result.bytes[i];
                                    uint8_t next = (i > 0) ? result.bytes[i - 1] : (signBit ? 0xFF : 0x00);
                                    result.bytes[i] = (current >> bitShift) | (next << (8 - bitShift));
                                }
                            }
                            push(result);
                        }
                        break;
                    }

                    case OP_SHA3: {
                        useGas(27); // SHA3 base cost
                        Word256 offset = pop();
                        Word256 size = pop();
                        std::vector<unsigned char> data = getMemorySlice(offset.toUint64(), size.toUint64());
                        useGas(6 * ((size.toUint64() + 31) / 32)); // Word cost
                        std::vector<unsigned char> hash = keccak256(data);
                        push(Word256(hash));
                        break;
                    }

                    case OP_ADDRESS:
                        push(Word256(ctx.contractAddress));
                        break;

                    case OP_CALLER:
                        push(Word256(ctx.callerAddress));
                        break;

                    case OP_ORIGIN:
                        push(Word256(ctx.originAddress));
                        break;

                    case OP_CALLVALUE:
                        push(Word256(ctx.value));
                        break;

                    case OP_CALLDATALOAD: {
                        Word256 i = pop();
                        uint64_t offset = i.toUint64();
                        Word256 data;
                        for (size_t j = 0; j < 32; j++) {
                            if (offset + j < ctx.callData.size()) {
                                data.bytes[j] = ctx.callData[offset + j];
                            }
                        }
                        push(data);
                        break;
                    }

                    case OP_CALLDATASIZE:
                        push(Word256(ctx.callData.size()));
                        break;

                    case OP_CALLDATACOPY: {
                        Word256 destOffset = pop();
                        Word256 offset = pop();
                        Word256 size = pop();
                        uint64_t dOff = destOffset.toUint64();
                        uint64_t sOff = offset.toUint64();
                        uint64_t sz = size.toUint64();

                        std::vector<unsigned char> data(sz, 0);
                        for (size_t i = 0; i < sz && sOff + i < ctx.callData.size(); i++) {
                            data[i] = ctx.callData[sOff + i];
                        }
                        setMemorySlice(dOff, data);
                        break;
                    }

                    case OP_CODESIZE:
                        push(Word256(ctx.code.size()));
                        break;

                    case OP_CODECOPY: {
                        Word256 destOffset = pop();
                        Word256 offset = pop();
                        Word256 size = pop();
                        uint64_t dOff = destOffset.toUint64();
                        uint64_t sOff = offset.toUint64();
                        uint64_t sz = size.toUint64();

                        std::vector<unsigned char> data(sz, 0);
                        for (size_t i = 0; i < sz && sOff + i < ctx.code.size(); i++) {
                            data[i] = ctx.code[sOff + i];
                        }
                        setMemorySlice(dOff, data);
                        break;
                    }

                    case OP_GASPRICE:
                        push(Word256(ctx.gasPrice));
                        break;

                    case OP_RETURNDATASIZE:
                        push(Word256(returnData.size()));
                        break;

                    case OP_RETURNDATACOPY: {
                        Word256 destOffset = pop();
                        Word256 offset = pop();
                        Word256 size = pop();
                        uint64_t dOff = destOffset.toUint64();
                        uint64_t sOff = offset.toUint64();
                        uint64_t sz = size.toUint64();

                        std::vector<unsigned char> data(sz, 0);
                        for (size_t i = 0; i < sz && sOff + i < returnData.size(); i++) {
                            data[i] = returnData[sOff + i];
                        }
                        setMemorySlice(dOff, data);
                        break;
                    }

                    case OP_BLOCKHASH: {
                        Word256 blockNum = pop();
                        // Return zero hash for now (would need block history access)
                        push(Word256(0));
                        break;
                    }

                    case OP_COINBASE:
                        // Return zero address (miner address)
                        push(Word256(0));
                        break;

                    case OP_TIMESTAMP:
                        push(Word256(ctx.timestamp));
                        break;

                    case OP_NUMBER:
                        push(Word256(ctx.blockNumber));
                        break;

                    case OP_DIFFICULTY:
                        // Return 1 for PoW difficulty placeholder
                        push(Word256(1));
                        break;

                    case OP_GASLIMIT:
                        push(Word256(ctx.gasLimit));
                        break;

                    case OP_CHAINID:
                        push(Word256(8800)); // HTH chain ID
                        break;

                    case OP_SELFBALANCE:
                        // Return contract balance (placeholder - would need UTXO access)
                        push(Word256(ctx.value));
                        break;

                    case OP_BALANCE: {
                        Word256 addr = pop();
                        // Return 0 for external balances (would need UTXO access)
                        push(Word256(0));
                        break;
                    }

                    case OP_EXTCODESIZE: {
                        Word256 addr = pop();
                        // Return 0 for external code size (would need contract DB access)
                        push(Word256(0));
                        break;
                    }

                    case OP_EXTCODEHASH: {
                        Word256 addr = pop();
                        // Return empty hash for external code (would need contract DB access)
                        push(Word256(0));
                        break;
                    }

                    case OP_EXTCODECOPY: {
                        Word256 addr = pop();
                        Word256 destOffset = pop();
                        Word256 offset = pop();
                        Word256 size = pop();
                        // Copy zeros (would need contract DB access for external code)
                        uint64_t dOff = destOffset.toUint64();
                        uint64_t sz = size.toUint64();
                        std::vector<unsigned char> zeros(sz, 0);
                        setMemorySlice(dOff, zeros);
                        break;
                    }

                    case OP_POP:
                        pop();
                        break;

                    case OP_MLOAD: {
                        Word256 offset = pop();
                        push(mload(offset.toUint64()));
                        break;
                    }

                    case OP_MSTORE: {
                        Word256 offset = pop();
                        Word256 value = pop();
                        mstore(offset.toUint64(), value);
                        break;
                    }

                    case OP_MSTORE8: {
                        Word256 offset = pop();
                        Word256 value = pop();
                        mstore8(offset.toUint64(), value.bytes[31]);
                        break;
                    }

                    case OP_SLOAD: {
                        useGas(797); // SLOAD cost (cold)
                        Word256 key = pop();
                        std::vector<unsigned char> keyBytes = key.toBytes();
                        std::vector<unsigned char> value;
                        if (ctx.storage) {
                            value = ctx.storage->getStorage(keyBytes);
                        }
                        if (value.size() < 32) {
                            value.resize(32, 0);
                        }
                        push(Word256(value));
                        break;
                    }

                    case OP_SSTORE: {
                        // Check if we're in read-only mode
                        if (ctx.readOnly) {
                            throw std::runtime_error("SSTORE not allowed in read-only call");
                        }
                        useGas(2897); // SSTORE cost (cold, non-zero to non-zero)
                        Word256 key = pop();
                        Word256 value = pop();
                        std::vector<unsigned char> keyBytes = key.toBytes();
                        std::vector<unsigned char> valueBytes = value.toBytes();
                        if (ctx.storage) {
                            ctx.storage->setStorage(keyBytes, valueBytes);
                        }
                        break;
                    }

                    case OP_JUMP: {
                        Word256 dest = pop();
                        uint64_t target = dest.toUint64();
                        if (target >= ctx.code.size()) {
                            throw std::runtime_error("Invalid jump destination: target " + std::to_string(target) +
                                " >= code size " + std::to_string(ctx.code.size()));
                        }
                        if (ctx.code[target] != OP_JUMPDEST) {
                            throw std::runtime_error("Invalid jump destination: target " + std::to_string(target) +
                                " has opcode 0x" + std::to_string(ctx.code[target] / 16) + std::to_string(ctx.code[target] % 16) +
                                " (expected 0x5b JUMPDEST)");
                        }
                        pc = target;
                        break;
                    }

                    case OP_JUMPI: {
                        // EVM Yellow Paper: μ′pc ≡ μs[0] if μs[1] ≠ 0
                        // stack[0] = destination (counter), stack[1] = condition (b)
                        Word256 dest = pop();  // stack[0] = destination
                        Word256 cond = pop();  // stack[1] = condition
                        if (!cond.isZero()) {
                            uint64_t target = dest.toUint64();
                            if (target >= ctx.code.size()) {
                                throw std::runtime_error("Invalid jump destination: target " + std::to_string(target) +
                                    " >= code size " + std::to_string(ctx.code.size()));
                            }
                            if (ctx.code[target] != OP_JUMPDEST) {
                                throw std::runtime_error("Invalid jump destination: target " + std::to_string(target) +
                                    " has opcode 0x" + std::to_string(ctx.code[target] / 16) + std::to_string(ctx.code[target] % 16) +
                                    " (expected 0x5b JUMPDEST)");
                            }
                            pc = target;
                        }
                        break;
                    }

                    case OP_PC:
                        push(Word256(pc - 1));
                        break;

                    case OP_MSIZE:
                        push(Word256(memory.size()));
                        break;

                    case OP_GAS:
                        push(Word256(ctx.gasLimit - gasUsed));
                        break;

                    case OP_JUMPDEST:
                        // No-op marker
                        break;

                    case OP_RETURN: {
                        Word256 offset = pop();
                        Word256 size = pop();
                        result.output = getMemorySlice(offset.toUint64(), size.toUint64());
                        result.success = true;
                        stopped = true;
                        break;
                    }

                    case OP_REVERT: {
                        Word256 offset = pop();
                        Word256 size = pop();
                        result.output = getMemorySlice(offset.toUint64(), size.toUint64());
                        result.success = false;
                        result.error = "Reverted";
                        stopped = true;
                        break;
                    }

                    case OP_INVALID:
                        throw std::runtime_error("Invalid opcode");

                    default:
                        // Handle PUSH1-PUSH32
                        if (opcode >= OP_PUSH1 && opcode <= OP_PUSH32) {
                            uint8_t n = opcode - OP_PUSH1 + 1;
                            Word256 w;
                            size_t offset = 32 - n;
                            for (uint8_t i = 0; i < n && pc < ctx.code.size(); i++) {
                                w.bytes[offset + i] = ctx.code[pc++];
                            }
                            push(w);
                        }
                        // Handle DUP1-DUP16
                        else if (opcode >= OP_DUP1 && opcode <= OP_DUP16) {
                            uint8_t n = opcode - OP_DUP1;
                            push(peek(n));
                        }
                        // Handle SWAP1-SWAP16
                        else if (opcode >= OP_SWAP1 && opcode <= OP_SWAP16) {
                            uint8_t n = opcode - OP_SWAP1 + 1;
                            if (n >= stack.size()) {
                                throw std::runtime_error("Stack underflow");
                            }
                            std::swap(peek(0), peek(n));
                        }
                        // Handle LOG0-LOG4 (no-op for read-only)
                        else if (opcode >= OP_LOG0 && opcode <= OP_LOG4) {
                            uint8_t topics = opcode - OP_LOG0;
                            pop(); // offset
                            pop(); // size
                            for (uint8_t i = 0; i < topics; i++) {
                                pop(); // topic
                            }
                            // Logs are not emitted in read-only calls
                        }
                        else {
                            throw std::runtime_error("Unsupported opcode: 0x" +
                                std::to_string(opcode / 16) + std::to_string(opcode % 16));
                        }
                        break;
                }
            }

            result.gasUsed = gasUsed;

        } catch (const std::exception& e) {
            result.success = false;
            result.error = e.what();
            result.gasUsed = gasUsed;
        }

        return result;
    }
};

ExecutionResult Execute(const VMContext& ctx) {
    VM vm(ctx);
    return vm.run();
}

ExecutionResult CallContract(
    const std::vector<unsigned char>& contractAddress,
    const std::vector<unsigned char>& code,
    const std::vector<unsigned char>& callData,
    StorageInterface* storage,
    uint64_t gasLimit,
    bool readOnly)
{
    VMContext ctx;
    ctx.contractAddress = contractAddress;
    ctx.callerAddress = std::vector<unsigned char>(20, 0); // Zero address for view calls
    ctx.originAddress = ctx.callerAddress;
    ctx.callData = callData;
    ctx.code = code;
    ctx.gasLimit = gasLimit;
    ctx.value = 0;
    ctx.blockNumber = 0;
    ctx.timestamp = 0;
    ctx.gasPrice = 0;
    ctx.storage = storage;
    ctx.readOnly = readOnly;

    return Execute(ctx);
}

} // namespace SimpleVM
