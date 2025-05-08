#include "cpu_instructions/bit_instructions.h"
#include "../cpu.h"         // For CPU class reference, flag masks, register accessors
#include "../common.h"      // For BYTE, WORD, logUnhandledOpcode (assuming)
#include "../cpu_constants.h" // For CB_OPCODE_TABLE and CPU flag constants

// --- Bit Operation Implementations using cpu.setFlags ---

// Flags: Z 0 1 - (C is not affected)
void BitInstructions::CPU_BIT(BYTE value_to_test, BYTE bit_pos) {
    BYTE current_f_value = cpu.getFlags(); // Get current F register
    BYTE new_f_value = current_f_value & CPU::FLAG_C_MASK; // Preserve current C flag

    if (!((value_to_test) & (1 << bit_pos))) { // If the tested bit is 0
        new_f_value |= CPU::FLAG_Z_MASK; // Set Z flag
    }
    // N flag is reset (0)
    new_f_value |= CPU::FLAG_H_MASK; // H flag is set (1)
    cpu.setFlags(new_f_value);
}

// Flags: No flags affected
void BitInstructions::CPU_SET(BYTE& value_ref, BYTE bit_pos) {
    value_ref |= (1 << bit_pos);
    // No change to F register
}

// Flags: No flags affected
void BitInstructions::CPU_RES(BYTE& value_ref, BYTE bit_pos) {
    value_ref &= ~(1 << bit_pos);
    // No change to F register
}

// Flags: Z 0 0 C
void BitInstructions::CPU_RLC(BYTE& value_ref) {
    BYTE new_f_value = 0;
    bool new_carry = (value_ref & 0x80) != 0; // Old bit 7

    value_ref = (value_ref << 1) | (new_carry ? 1 : 0); // Rotate left, bit 0 becomes old bit 7

    if (value_ref == 0) new_f_value |= CPU::FLAG_Z_MASK;
    // N flag is 0
    // H flag is 0
    if (new_carry) new_f_value |= CPU::FLAG_C_MASK;
    cpu.setFlags(new_f_value);
}

// Flags: Z 0 0 C
void BitInstructions::CPU_RRC(BYTE& value_ref) {
    BYTE new_f_value = 0;
    bool new_carry = (value_ref & 0x01) != 0; // Old bit 0

    value_ref = (value_ref >> 1) | (new_carry ? 0x80 : 0); // Rotate right, bit 7 becomes old bit 0

    if (value_ref == 0) new_f_value |= CPU::FLAG_Z_MASK;
    // N flag is 0
    // H flag is 0
    if (new_carry) new_f_value |= CPU::FLAG_C_MASK;
    cpu.setFlags(new_f_value);
}

// Flags: Z 0 0 C
void BitInstructions::CPU_RL(BYTE& value_ref) {
    BYTE new_f_value = 0;
    bool old_cpu_carry = cpu.getFlagC();
    bool new_carry_val = (value_ref & 0x80) != 0; // Old bit 7

    value_ref = (value_ref << 1) | (old_cpu_carry ? 1 : 0);

    if (value_ref == 0) new_f_value |= CPU::FLAG_Z_MASK;
    // N flag is 0
    // H flag is 0
    if (new_carry_val) new_f_value |= CPU::FLAG_C_MASK;
    cpu.setFlags(new_f_value);
}

// Flags: Z 0 0 C
void BitInstructions::CPU_RR(BYTE& value_ref) {
    BYTE new_f_value = 0;
    bool old_cpu_carry = cpu.getFlagC();
    bool new_carry_val = (value_ref & 0x01) != 0; // Old bit 0

    value_ref = (value_ref >> 1) | (old_cpu_carry ? 0x80 : 0);

    if (value_ref == 0) new_f_value |= CPU::FLAG_Z_MASK;
    // N flag is 0
    // H flag is 0
    if (new_carry_val) new_f_value |= CPU::FLAG_C_MASK;
    cpu.setFlags(new_f_value);
}

// Flags: Z 0 0 C
void BitInstructions::CPU_SLA(BYTE& value_ref) {
    BYTE new_f_value = 0;
    bool new_carry = (value_ref & 0x80) != 0; // Old bit 7
    value_ref <<= 1; // Bit 0 becomes 0

    if (value_ref == 0) new_f_value |= CPU::FLAG_Z_MASK;
    // N flag is 0
    // H flag is 0
    if (new_carry) new_f_value |= CPU::FLAG_C_MASK;
    cpu.setFlags(new_f_value);
}

// Flags: Z 0 0 C
void BitInstructions::CPU_SRA(BYTE& value_ref) {
    BYTE new_f_value = 0;
    bool new_carry = (value_ref & 0x01) != 0; // Old bit 0
    BYTE msb = value_ref & 0x80; // Preserve bit 7
    value_ref = (value_ref >> 1) | msb;

    if (value_ref == 0) new_f_value |= CPU::FLAG_Z_MASK;
    // N flag is 0
    // H flag is 0
    if (new_carry) new_f_value |= CPU::FLAG_C_MASK;
    cpu.setFlags(new_f_value);
}

// Flags: Z 0 0 C
void BitInstructions::CPU_SRL(BYTE& value_ref) {
    BYTE new_f_value = 0;
    bool new_carry = (value_ref & 0x01) != 0; // Old bit 0
    value_ref >>= 1; // Bit 7 becomes 0

    if (value_ref == 0) new_f_value |= CPU::FLAG_Z_MASK;
    // N flag is 0
    // H flag is 0
    if (new_carry) new_f_value |= CPU::FLAG_C_MASK;
    cpu.setFlags(new_f_value);
}

// Flags: Z 0 0 0
void BitInstructions::CPU_SWAP(BYTE& value_ref) {
    BYTE new_f_value = 0;
    value_ref = ((value_ref & 0x0F) << 4) | ((value_ref & 0xF0) >> 4);

    if (value_ref == 0) new_f_value |= CPU::FLAG_Z_MASK;
    // N, H, C flags are 0
    cpu.setFlags(new_f_value);
}


int BitInstructions::execute(BYTE opcode) {
    BYTE target_value;
    BYTE bit = (opcode & 0x38) >> 3;  // Get bit number (0-7) from opcode bits 3,4,5
    BYTE reg_code = opcode & 0x07;    // Get register code (0-7) from opcode bits 0,1,2

    bool target_is_hl_memory = (reg_code == 0x06);

    // Read the target value
    // Assumes cpu has getA(), getB(), getC(), getD(), getE(), getH(), getL() returning BYTE&
    // and getHL_ref() returning Register& for cpu.getHL_ref().reg
    if (target_is_hl_memory) {
        target_value = cpu.readMemory(cpu.getHL_ref().reg);
    } else {
        switch(reg_code) {
            case 0: target_value = cpu.getB(); break;
            case 1: target_value = cpu.getC(); break;
            case 2: target_value = cpu.getD(); break;
            case 3: target_value = cpu.getE(); break;
            case 4: target_value = cpu.getH(); break;
            case 5: target_value = cpu.getL(); break;
            // case 6 is (HL), handled above
            case 7: target_value = cpu.getA(); break;
            default: 
                logUnhandledOpcode(0xCB00 | opcode); // Log with CB prefix for context
                return CPUConstants::CB_OPCODE_TABLE[opcode].duration_cycles; // Return table cycles
        }
    }

    // Determine operation type and execute
    // For BIT, target_value is passed by value. For others, by reference.
    if (opcode >= 0x40 && opcode <= 0x7F) { // BIT b, r/m
        CPU_BIT(target_value, bit);
        // target_value is not modified by CPU_BIT, so no write-back needed for BIT.
    } else if (opcode >= 0x80 && opcode <= 0xBF) { // RES b, r/m
        CPU_RES(target_value, bit);
    } else if (opcode >= 0xC0 && opcode <= 0xFF) { // SET b, r/m
        CPU_SET(target_value, bit);
    } else { // Rotations/shifts (0x00 - 0x3F)
        // The specific rotate/shift operation is determined by bits 3,4,5 (same as 'bit' variable for BIT/SET/RES)
        // but it's clearer to switch on the upper part of the opcode that defines the operation group.
        BYTE operation_sub_type = opcode & 0xF8; // Mask to get the group (RLC, RRC, RL, etc.)
        switch(operation_sub_type) {
            case 0x00: CPU_RLC(target_value); break; // RLC r
            case 0x08: CPU_RRC(target_value); break; // RRC r
            case 0x10: CPU_RL(target_value);  break; // RL r
            case 0x18: CPU_RR(target_value);  break; // RR r
            case 0x20: CPU_SLA(target_value); break; // SLA r
            case 0x28: CPU_SRA(target_value); break; // SRA r
            case 0x30: CPU_SWAP(target_value);break; // SWAP r
            case 0x38: CPU_SRL(target_value); break; // SRL r
            default:
                logUnhandledOpcode(0xCB00 | opcode); // Should not happen with valid CB opcodes
                // No write-back needed if unhandled
                return CPUConstants::CB_OPCODE_TABLE[opcode].duration_cycles;
        }
    }

    // If the value was modified (i.e., not a BIT operation) and target was a register, update the register.
    // For (HL) memory, write back if it's not a BIT operation.
    if (!(opcode >= 0x40 && opcode <= 0x7F)) { // If not a BIT operation
        if (target_is_hl_memory) {
            cpu.writeMemory(cpu.getHL_ref().reg, target_value);
        } else {
            switch(reg_code) {
                case 0: cpu.getB() = target_value; break;
                case 1: cpu.getC() = target_value; break;
                case 2: cpu.getD() = target_value; break;
                case 3: cpu.getE() = target_value; break;
                case 4: cpu.getH() = target_value; break;
                case 5: cpu.getL() = target_value; break;
                // case 6 is (HL)
                case 7: cpu.getA() = target_value; break;
            }
        }
    }

    return CPUConstants::CB_OPCODE_TABLE[opcode].duration_cycles;
}