#pragma once
#include "common.h" // Assuming common.h defines BYTE, WORD etc.

#include <string>
#include <vector>
#include <array>


namespace GB {

// Flag modification types
enum class FlagModificationType { NONE, RESET, SET, MODIFIED };
// Instruction groups
enum class InstructionGroup { X8_ALU, X16_ALU, X8_LSM, X16_LSM, X8_RSB, CONTROL_BR, CONTROL_MISC };
// CPU registers
enum class Register { NONE, A, B, C, D, E, H, L, AF, BC, DE, HL, SP, PC, MEM_BC, MEM_DE, MEM_HL, MEM_HLI, MEM_HLD, MEM_C, MEM_A8, MEM_A16 };
// Condition types
enum class ConditionType { NONE, Z, NZ, C, NC };

// Structure to hold opcode information
struct OpcodeInfo {
    std::string mnemonic;
    uint8_t length;
    std::vector<uint8_t> cycles;
    std::array<FlagModificationType, 4> flags; // Z, N, H, C
    uint16_t address;
    InstructionGroup group;
    Register operand1;
    Register operand2;
    ConditionType condition;
    uint16_t extraData;
    bool isPrefixed;
};

// Opcode tables (Singleton)
class OpcodeTables {
public:
    static OpcodeTables& getInstance();
    const OpcodeInfo& getInfo(uint8_t opcode, bool prefixed = false) const;

    // Delete copy/move operations for Singleton
    OpcodeTables(const OpcodeTables&) = delete;
    OpcodeTables& operator=(const OpcodeTables&) = delete;
    OpcodeTables(OpcodeTables&&) = delete;
    OpcodeTables& operator=(OpcodeTables&&) = delete;

private:
    OpcodeTables(); // Private constructor

    std::array<OpcodeInfo, 256> standardOpcodes;
    std::array<OpcodeInfo, 256> cbPrefixedOpcodes;
    const OpcodeInfo unknownOpcodeInfo; // Default info for invalid opcodes (const member)
};

} // namespace GB
