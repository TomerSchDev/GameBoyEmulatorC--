#include "cpu_instructions/jump_instructions.h"
#include "../cpu.h"
#include "../common.h"      // For logUnhandledOpcode
#include "../cpu_constants.h" // For FULL_OPCODE_TABLE and flag constants

// Anonymous namespace for local constants (opcodes)
namespace {
    // Jump opcodes (as defined in your original file)
    constexpr BYTE JP_NN = 0xC3;    // JP nn
    constexpr BYTE JP_NZ = 0xC2;    // JP NZ,nn
    constexpr BYTE JP_Z  = 0xCA;     // JP Z,nn
    constexpr BYTE JP_NC = 0xD2;    // JP NC,nn
    constexpr BYTE JP_C  = 0xDA;     // JP C,nn
    constexpr BYTE JP_HL = 0xE9;    // JP (HL)
    
    constexpr BYTE JR_N  = 0x18;     // JR n
    constexpr BYTE JR_NZ = 0x20;    // JR NZ,n
    constexpr BYTE JR_Z  = 0x28;     // JR Z,n
    constexpr BYTE JR_NC = 0x30;    // JR NC,n
    constexpr BYTE JR_C  = 0x38;     // JR C,n

    // CALL opcodes
    constexpr BYTE CALL_NN = 0xCD;   // CALL nn
    constexpr BYTE CALL_NZ = 0xC4;   // CALL NZ,nn
    constexpr BYTE CALL_Z  = 0xCC;   // CALL Z,nn
    constexpr BYTE CALL_NC = 0xD4;   // CALL NC,nn
    constexpr BYTE CALL_C  = 0xDC;   // CALL C,nn

    // RET opcodes
    constexpr BYTE RET_  = 0xC9;   // RET
    constexpr BYTE RET_NZ= 0xC0;   // RET NZ
    constexpr BYTE RET_Z = 0xC8;   // RET Z
    constexpr BYTE RET_NC= 0xD0;   // RET NC
    constexpr BYTE RET_C = 0xD8;   // RET C
    constexpr BYTE RETI  = 0xD9;   // RETI

    // RST opcodes
    constexpr BYTE RST_00 = 0xC7;
    constexpr BYTE RST_08 = 0xCF;
    constexpr BYTE RST_10 = 0xD7;
    constexpr BYTE RST_18 = 0xDF;
    constexpr BYTE RST_20 = 0xE7;
    constexpr BYTE RST_28 = 0xEF;
    constexpr BYTE RST_30 = 0xF7;
    constexpr BYTE RST_38 = 0xFF;
}

// Helper to check condition for conditional jumps/calls/rets
// This replaces the local checkFlag method
bool JumpInstructions::checkCondition(BYTE opcode_condition_type) {
    // opcode_condition_type: 00 (NZ), 01 (Z), 10 (NC), 11 (C)
    // These are typically bits 3 and 4 of the opcode for JP cc, CALL cc, RET cc
    switch (opcode_condition_type) {
        case 0x00: return !cpu.getFlagZ(); // NZ
        case 0x01: return cpu.getFlagZ();  // Z
        case 0x02: return !cpu.getFlagC(); // NC
        case 0x03: return cpu.getFlagC();  // C
        default: return false; // Should not happen
    }
}


int JumpInstructions::execute(BYTE opcode) {
    const auto& entry = CPUConstants::FULL_OPCODE_TABLE[opcode];
    bool condition_met = false; // For conditional instructions

    switch(opcode) {
        // JP nn, JP cc,nn, JP (HL)
        case JP_NN: 
            CPU_JP_nn(); 
            break;
        case JP_HL: 
            CPU_JP_HL(); 
            break;
        case JP_NZ: case JP_Z: case JP_NC: case JP_C:
            condition_met = CPU_JP_cc_nn((opcode >> 3) & 0x03); // Pass condition type
            break;

        // JR n, JR cc,n
        case JR_N: 
            CPU_JR_n(); 
            break;
        case JR_NZ: case JR_Z: case JR_NC: case JR_C:
            condition_met = CPU_JR_cc_n((opcode >> 3) & 0x03); // Pass condition type
            break;

        // CALL nn, CALL cc,nn
        case CALL_NN: 
            CPU_CALL_nn(); 
            break;
        case CALL_NZ: case CALL_Z: case CALL_NC: case CALL_C:
            condition_met = CPU_CALL_cc_nn((opcode >> 3) & 0x03); // Pass condition type
            break;

        // RET, RET cc, RETI
        case RET_: 
            CPU_RET(); 
            break;
        case RETI: 
            CPU_RETI(); 
            break;
        case RET_NZ: case RET_Z: case RET_NC: case RET_C:
            condition_met = CPU_RET_cc((opcode >> 3) & 0x03); // Pass condition type
            break;
        
        // RST n
        case RST_00: CPU_RST(0x00); break;
        case RST_08: CPU_RST(0x08); break;
        case RST_10: CPU_RST(0x10); break;
        case RST_18: CPU_RST(0x18); break;
        case RST_20: CPU_RST(0x20); break;
        case RST_28: CPU_RST(0x28); break;
        case RST_30: CPU_RST(0x30); break;
        case RST_38: CPU_RST(0x38); break;
        
        default: 
            logUnhandledOpcode(opcode);
            // For unhandled opcodes within this unit, still return cycles from table
            return entry.duration_cycles; 
    }

    // Determine cycles based on whether a condition was met (if applicable)
    if (entry.duration_cycles_conditional > 0) { // Check if it's a conditional instruction
        return condition_met ? entry.duration_cycles_conditional : entry.duration_cycles;
    } else {
        return entry.duration_cycles; // For non-conditional or if condition not applicable
    }
}

// --- Helper Implementations ---

// JP nn
void JumpInstructions::CPU_JP_nn() {
    WORD address = cpu.readWord(); // Consumes 2 bytes for the address
    cpu.getPC() = address;
}

// JP (HL)
void JumpInstructions::CPU_JP_HL() {
    cpu.getPC() = cpu.getHL_ref().reg;
}

// JP cc, nn
bool JumpInstructions::CPU_JP_cc_nn(BYTE condition_type) {
    WORD address = cpu.readWord(); // Consumes 2 bytes for the address
    if (checkCondition(condition_type)) {
        cpu.getPC() = address;
        return true; // Condition met
    }
    return false; // Condition not met
}

// JR n
void JumpInstructions::CPU_JR_n() {
    BYTE offset = cpu.readByte(); // Consumes 1 byte for the offset
    cpu.getPC() += static_cast<signed char>(offset);
}

// JR cc, n
bool JumpInstructions::CPU_JR_cc_n(BYTE condition_type) {
    BYTE offset = cpu.readByte(); // Consumes 1 byte for the offset
    if (checkCondition(condition_type)) {
        cpu.getPC() += static_cast<signed char>(offset);
        return true; // Condition met
    }
    return false; // Condition not met
}

// CALL nn
void JumpInstructions::CPU_CALL_nn() {
    WORD address = cpu.readWord(); // Consumes 2 bytes for the address
    cpu.pushToStack(cpu.getPC()); // PC is already past CALL nn, points to next instruction
    cpu.getPC() = address;
}

// CALL cc, nn
bool JumpInstructions::CPU_CALL_cc_nn(BYTE condition_type) {
    WORD address = cpu.readWord(); // Consumes 2 bytes for the address
    if (checkCondition(condition_type)) {
        cpu.pushToStack(cpu.getPC()); // PC is already past CALL cc,nn
        cpu.getPC() = address;
        return true; // Condition met
    }
    return false; // Condition not met
}

// RET
void JumpInstructions::CPU_RET() {
    cpu.getPC() = cpu.popFromStack();
}

// RETI
void JumpInstructions::CPU_RETI() {
    cpu.getPC() = cpu.popFromStack();
    cpu.setInterruptState(true); // Enable interrupts
}

// RET cc
bool JumpInstructions::CPU_RET_cc(BYTE condition_type) {
    if (checkCondition(condition_type)) {
        cpu.getPC() = cpu.popFromStack();
        return true; // Condition met
    }
    return false; // Condition not met
}

// RST n
void JumpInstructions::CPU_RST(BYTE vector) {
    cpu.pushToStack(cpu.getPC()); // PC is already past RST n
    cpu.getPC() = static_cast<WORD>(vector);
}
