#include "cpu_instructions/ALU_instructions.h"
using namespace CPUConstants;

int ALUInstructions::execute(BYTE opcode) {
    switch(opcode) {
        // ADD A,r
        case ALU::ADD_A_B: return CPU_8BIT_ADD(cpu.getAF().hi, cpu.getBC().hi, false);
        case ALU::ADD_A_C: return CPU_8BIT_ADD(cpu.getAF().hi, cpu.getBC().lo, false);
        case ALU::ADD_A_D: return CPU_8BIT_ADD(cpu.getAF().hi, cpu.getDE().hi, false);
        case ALU::ADD_A_E: return CPU_8BIT_ADD(cpu.getAF().hi, cpu.getDE().lo, false);
        case ALU::ADD_A_H: return CPU_8BIT_ADD(cpu.getAF().hi, cpu.getHL().hi, false);
        case ALU::ADD_A_L: return CPU_8BIT_ADD(cpu.getAF().hi, cpu.getHL().lo, false);
        case ALU::ADD_A_A: return CPU_8BIT_ADD(cpu.getAF().hi, cpu.getAF().hi, false);

        // SUB r
        case ALU::SUB_B: return CPU_8BIT_SUB(cpu.getAF().hi, cpu.getBC().hi, false);
        case ALU::SUB_C: return CPU_8BIT_SUB(cpu.getAF().hi, cpu.getBC().lo, false);
        case ALU::SUB_D: return CPU_8BIT_SUB(cpu.getAF().hi, cpu.getDE().hi, false);
        case ALU::SUB_E: return CPU_8BIT_SUB(cpu.getAF().hi, cpu.getDE().lo, false);
        case ALU::SUB_H: return CPU_8BIT_SUB(cpu.getAF().hi, cpu.getHL().hi, false);
        case ALU::SUB_L: return CPU_8BIT_SUB(cpu.getAF().hi, cpu.getHL().lo, false);
        case ALU::SUB_A: return CPU_8BIT_SUB(cpu.getAF().hi, cpu.getAF().hi, false);

        default: 
        // Log error for unhandled opcode within this unit
        // Or, if this should never happen, assert or throw
        logUnhandledOpcode(opcode);
        return cpu.handleUnknownOpcode(opcode); // Or return a fixed cycle count like 4

    }
}

int ALUInstructions::CPU_8BIT_ADD(BYTE& reg1, BYTE reg2, bool useCarry) {
    BYTE carry = (useCarry && checkFlag(FLAG_MASK_C)) ? 1 : 0;
    WORD result = reg1 + reg2 + carry;

    setZeroFlag((result & BYTE_MASK) == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(((reg1 & LOWER_NIBBLE_MASK) + (reg2 & LOWER_NIBBLE_MASK) + carry) > LOWER_NIBBLE_MASK);
    setCarryFlag(result > BYTE_MASK);

    reg1 = result & BYTE_MASK;
    return ALU_REGULAR_CYCLES;
}

int ALUInstructions::CPU_8BIT_SUB(BYTE& reg1, BYTE reg2, bool useCarry) {
    BYTE carry = (useCarry && checkFlag(FLAG_MASK_C)) ? 1 : 0;
    WORD result = reg1 - reg2 - carry;

    setZeroFlag((result & BYTE_MASK) == 0);
    setSubtractFlag(true);
    setHalfCarryFlag(((reg1 & LOWER_NIBBLE_MASK) - (reg2 & LOWER_NIBBLE_MASK) - carry) < 0);
    setCarryFlag(result > BYTE_MASK);

    reg1 = result & BYTE_MASK;
    return 4;
}

int ALUInstructions::CPU_8BIT_AND(BYTE& reg1, BYTE reg2) {
    reg1 &= reg2;
    
    setZeroFlag(reg1 == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(true);
    setCarryFlag(false);
    
    return 4;
}

int ALUInstructions::CPU_8BIT_OR(BYTE& reg1, BYTE reg2) {
    reg1 |= reg2;
    
    setZeroFlag(reg1 == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(false);
    
    return 4;
}

int ALUInstructions::CPU_8BIT_XOR(BYTE& reg1, BYTE reg2) {
    reg1 ^= reg2;
    
    setZeroFlag(reg1 == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(false);
    
    return 4;
}

int ALUInstructions::CPU_8BIT_CP(BYTE reg1, BYTE reg2) {
    WORD result = reg1 - reg2;

    setZeroFlag((result & BYTE_MASK) == 0);
    setSubtractFlag(true);
    setHalfCarryFlag(((reg1 & LOWER_NIBBLE_MASK) - (reg2 & LOWER_NIBBLE_MASK)) < 0);
    setCarryFlag(result > BYTE_MASK);

    return 4;
}

int ALUInstructions::CPU_8BIT_INC(BYTE& reg) {
    // Calculate half carry before incrementing reg
    setHalfCarryFlag(((reg & LOWER_NIBBLE_MASK) + 1) > LOWER_NIBBLE_MASK);
    
    reg++;
    
    setZeroFlag(reg == 0);
    setSubtractFlag(false);
    
    // Carry flag is not affected
    return 4;
}

int ALUInstructions::CPU_8BIT_DEC(BYTE& reg) {
    // Calculate half carry before decrementing reg
    setHalfCarryFlag((reg & LOWER_NIBBLE_MASK) == 0); // Half carry if lower nibble was 0 before decrement

    reg--;
    
    setZeroFlag(reg == 0);
    setSubtractFlag(true);
    
    // Carry flag is not affected
    return 4;
}