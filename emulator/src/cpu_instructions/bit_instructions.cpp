#include "cpu_instructions/bit_instructions.h"
#include "../cpu.h"
#include "../common.h"

int BitInstructions::execute(BYTE opcode) {
    BYTE target_value;
    BYTE bit = (opcode & 0x38) >> 3;  // Get bit number from opcode
    BYTE reg_code = opcode & 0x07;    // Get register code

    bool target_is_hl_memory = (reg_code == 6);
    int cycles = target_is_hl_memory ? 16 : 8; // Base cycles, BIT (HL) is 12 by some docs, 16 by others. User table implies 16.

    // Read the target value
    switch(reg_code) {
        case 0: target_value = cpu.getBC().hi; break;  // B
        case 1: target_value = cpu.getBC().lo; break;  // C
        case 2: target_value = cpu.getDE().hi; break;  // D
        case 3: target_value = cpu.getDE().lo; break;  // E
        case 4: target_value = cpu.getHL().hi; break;  // H
        case 5: target_value = cpu.getHL().lo; break;  // L
        case 6: target_value = cpu.readMemory(cpu.getHL().reg); break;  // (HL)
        case 7: target_value = cpu.getAF().hi; break;  // A
        default: 
            logUnhandledOpcode(opcode); // Should not happen with valid CB opcodes
            return 8; // Default cycles
    }

    // Determine operation type and execute
    // Note: target_value is passed by value to BIT, by reference to others that modify it.
    if (opcode >= 0x40 && opcode <= 0x7F) { // BIT b, r
        CPU_BIT(target_value, bit); // target_value is not modified by CPU_BIT
        return cycles; 
    } else if (opcode >= 0x80 && opcode <= 0xBF) { // RES b, r
        CPU_RES(target_value, bit);
    } else if (opcode >= 0xC0 && opcode <= 0xFF) { // SET b, r
        CPU_SET(target_value, bit);
    } else { // Rotations/shifts (0x00 - 0x3F)
        switch(opcode & 0xF8) { // Check upper 5 bits to group rotate/shift types
            case 0x00: CPU_RLC(target_value); break; // RLC r
            case 0x08: CPU_RRC(target_value); break; // RRC r
            case 0x10: CPU_RL(target_value);  break; // RL r
            case 0x18: CPU_RR(target_value);  break; // RR r
            case 0x20: CPU_SLA(target_value); break; // SLA r
            case 0x28: CPU_SRA(target_value); break; // SRA r
            case 0x30: CPU_SWAP(target_value);break; // SWAP r
            case 0x38: CPU_SRL(target_value); break; // SRL r
            default:
                logUnhandledOpcode(opcode); // Should not happen
                return cycles; 
        }
    }

    // If the value was modified and target was a register, update the register
    if (!target_is_hl_memory) {
        switch(reg_code) {
            case 0: cpu.getBC().hi = target_value; break;
            case 1: cpu.getBC().lo = target_value; break;
            case 2: cpu.getDE().hi = target_value; break;
            case 3: cpu.getDE().lo = target_value; break;
            case 4: cpu.getHL().hi = target_value; break;
            case 5: cpu.getHL().lo = target_value; break;
            case 7: cpu.getAF().hi = target_value; break;
            // No case 6, as that's (HL) memory
        }
    } else {
        // If target was (HL) memory, write the modified value back
        cpu.writeMemory(cpu.getHL().reg, target_value);
    }

    return cycles;
}

// CPU_BIT does not modify reg, so it takes reg by value.
// Flags: Z 0 1 -
void BitInstructions::CPU_BIT(BYTE reg_val, BYTE bit) {
    setZeroFlag(!((reg_val) & (1 << bit)));
    setSubtractFlag(false);
    setHalfCarryFlag(true);
    // No return value needed as cycles are handled in execute
}

// SET, RES and Rotates/Shifts modify reg, so they take reg by reference.
// No flags affected by SET.
void BitInstructions::CPU_SET(BYTE& reg, BYTE bit) {
    reg |= (1 << bit);
}

// No flags affected by RES.
void BitInstructions::CPU_RES(BYTE& reg, BYTE bit) {
    reg &= ~(1 << bit);
}

// Flags: Z 0 0 C
void BitInstructions::CPU_RL(BYTE& reg) {
    bool oldCarry = checkFlag(FLAG_MASK_C);
    setCarryFlag(reg & 0x80); // New carry is old bit 7
    reg = (reg << 1) | (oldCarry ? 1 : 0);
    setZeroFlag(reg == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
}

// Flags: Z 0 0 C
void BitInstructions::CPU_RR(BYTE& reg) {
    bool oldCarry = checkFlag(FLAG_MASK_C);
    setCarryFlag(reg & 0x01); // New carry is old bit 0
    reg = (reg >> 1) | (oldCarry ? 0x80 : 0);
    setZeroFlag(reg == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
}

// Flags: Z 0 0 C
void BitInstructions::CPU_RLC(BYTE& reg) {
    setCarryFlag(reg & 0x80); // New carry is old bit 7
    reg = (reg << 1) | (reg >> 7);
    setZeroFlag(reg == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
}

// Flags: Z 0 0 C
void BitInstructions::CPU_RRC(BYTE& reg) {
    setCarryFlag(reg & 0x01); // New carry is old bit 0
    reg = (reg >> 1) | (reg << 7);
    setZeroFlag(reg == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
}

// Flags: Z 0 0 C
void BitInstructions::CPU_SLA(BYTE& reg) {
    setCarryFlag(reg & 0x80); // New carry is old bit 7
    reg <<= 1;
    setZeroFlag(reg == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
}

// Flags: Z 0 0 C (C from old bit 0, Z if result is 0, N=0, H=0)
void BitInstructions::CPU_SRA(BYTE& reg) {
    setCarryFlag(reg & 0x01); // New carry is old bit 0
    reg = (reg >> 1) | (reg & 0x80); // Bit 7 remains same
    setZeroFlag(reg == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
}

// Flags: Z 0 0 C (C from old bit 0, Z if result is 0, N=0, H=0)
void BitInstructions::CPU_SRL(BYTE& reg) {
    setCarryFlag(reg & 0x01); // New carry is old bit 0
    reg >>= 1; // Bit 7 becomes 0
    setZeroFlag(reg == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
}

// Flags: Z 0 0 0
void BitInstructions::CPU_SWAP(BYTE& reg) {
    reg = ((reg & 0x0F) << 4) | ((reg & 0xF0) >> 4);
    setZeroFlag(reg == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(false); // SWAP resets carry flag
}