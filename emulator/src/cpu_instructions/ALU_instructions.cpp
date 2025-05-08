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
        case ALU::ADD_A_N: {
            BYTE n = cpu.readByte();
            return CPU_8BIT_ADD(cpu.getAF().hi, n, false);
        } 
        // ADC A,r
        case ALU::ADC_A_B: return CPU_8BIT_ADD(cpu.getAF().hi, cpu.getBC().hi, true);
        case ALU::ADC_A_C: return CPU_8BIT_ADD(cpu.getAF().hi, cpu.getBC().lo, true);
        case ALU::ADC_A_D: return CPU_8BIT_ADD(cpu.getAF().hi, cpu.getDE().hi, true);
        case ALU::ADC_A_E: return CPU_8BIT_ADD(cpu.getAF().hi, cpu.getDE().lo, true);
        case ALU::ADC_A_H: return CPU_8BIT_ADD(cpu.getAF().hi, cpu.getHL().hi, true);
        case ALU::ADC_A_L: return CPU_8BIT_ADD(cpu.getAF().hi, cpu.getHL().lo, true);
        case ALU::ADC_A_A: return CPU_8BIT_ADD(cpu.getAF().hi, cpu.getAF().hi, true);
        case ALU::ADC_A_N: {
            BYTE n = cpu.readByte();
            return CPU_8BIT_ADD(cpu.getAF().hi, n, true);
        }

        // SUB r
        case ALU::SUB_B: return CPU_8BIT_SUB(cpu.getAF().hi, cpu.getBC().hi, false);
        case ALU::SUB_C: return CPU_8BIT_SUB(cpu.getAF().hi, cpu.getBC().lo, false);
        case ALU::SUB_D: return CPU_8BIT_SUB(cpu.getAF().hi, cpu.getDE().hi, false);
        case ALU::SUB_E: return CPU_8BIT_SUB(cpu.getAF().hi, cpu.getDE().lo, false);
        case ALU::SUB_H: return CPU_8BIT_SUB(cpu.getAF().hi, cpu.getHL().hi, false);
        case ALU::SUB_L: return CPU_8BIT_SUB(cpu.getAF().hi, cpu.getHL().lo, false);
        case ALU::SUB_A: return CPU_8BIT_SUB(cpu.getAF().hi, cpu.getAF().hi, false);
        case ALU::SUB_N: {
            BYTE n = cpu.readByte();
            return CPU_8BIT_SUB(cpu.getAF().hi, n, false);
        }

        // SBC A,r
        case ALU::SBC_A_B: return CPU_8BIT_SUB(cpu.getAF().hi, cpu.getBC().hi, true);
        case ALU::SBC_A_C: return CPU_8BIT_SUB(cpu.getAF().hi, cpu.getBC().lo, true);
        case ALU::SBC_A_D: return CPU_8BIT_SUB(cpu.getAF().hi, cpu.getDE().hi, true);
        case ALU::SBC_A_E: return CPU_8BIT_SUB(cpu.getAF().hi, cpu.getDE().lo, true);
        case ALU::SBC_A_H: return CPU_8BIT_SUB(cpu.getAF().hi, cpu.getHL().hi, true);
        case ALU::SBC_A_L: return CPU_8BIT_SUB(cpu.getAF().hi, cpu.getHL().lo, true);
        case ALU::SBC_A_A: return CPU_8BIT_SUB(cpu.getAF().hi, cpu.getAF().hi, true);
        case ALU::SBC_A_N: {
            BYTE n = cpu.readByte();
            return CPU_8BIT_SUB(cpu.getAF().hi, n, true);
        }

        // AND r
        case ALU::AND_B: return CPU_8BIT_AND(cpu.getAF().hi, cpu.getBC().hi);
        case ALU::AND_C: return CPU_8BIT_AND(cpu.getAF().hi, cpu.getBC().lo);
        case ALU::AND_D: return CPU_8BIT_AND(cpu.getAF().hi, cpu.getDE().hi);
        case ALU::AND_E: return CPU_8BIT_AND(cpu.getAF().hi, cpu.getDE().lo);
        case ALU::AND_H: return CPU_8BIT_AND(cpu.getAF().hi, cpu.getHL().hi);
        case ALU::AND_L: return CPU_8BIT_AND(cpu.getAF().hi, cpu.getHL().lo);
        case ALU::AND_A: return CPU_8BIT_AND(cpu.getAF().hi, cpu.getAF().hi);
        case ALU::AND_HL: return CPU_8BIT_AND(cpu.getAF().hi, cpu.readMemory(cpu.getHL().reg));
        case ALU::AND_N: {
            BYTE n = cpu.readByte();
            return CPU_8BIT_AND(cpu.getAF().hi, n);
        }

        // OR r
        case ALU::OR_B: return CPU_8BIT_OR(cpu.getAF().hi, cpu.getBC().hi);
        case ALU::OR_C: return CPU_8BIT_OR(cpu.getAF().hi, cpu.getBC().lo);
        case ALU::OR_D: return CPU_8BIT_OR(cpu.getAF().hi, cpu.getDE().hi);
        case ALU::OR_E: return CPU_8BIT_OR(cpu.getAF().hi, cpu.getDE().lo);
        case ALU::OR_H: return CPU_8BIT_OR(cpu.getAF().hi, cpu.getHL().hi);
        case ALU::OR_L: return CPU_8BIT_OR(cpu.getAF().hi, cpu.getHL().lo);
        case ALU::OR_A: return CPU_8BIT_OR(cpu.getAF().hi, cpu.getAF().hi);
        case ALU::OR_HL: return CPU_8BIT_OR(cpu.getAF().hi, cpu.readMemory(cpu.getHL().reg));
        case ALU::OR_N: {
            BYTE n = cpu.readByte();
            return CPU_8BIT_OR(cpu.getAF().hi, n);
        }

        // XOR r
        case ALU::XOR_B: return CPU_8BIT_XOR(cpu.getAF().hi, cpu.getBC().hi);
        case ALU::XOR_C: return CPU_8BIT_XOR(cpu.getAF().hi, cpu.getBC().lo);
        case ALU::XOR_D: return CPU_8BIT_XOR(cpu.getAF().hi, cpu.getDE().hi);
        case ALU::XOR_E: return CPU_8BIT_XOR(cpu.getAF().hi, cpu.getDE().lo);
        case ALU::XOR_H: return CPU_8BIT_XOR(cpu.getAF().hi, cpu.getHL().hi);
        case ALU::XOR_L: return CPU_8BIT_XOR(cpu.getAF().hi, cpu.getHL().lo);
        case ALU::XOR_A: return CPU_8BIT_XOR(cpu.getAF().hi, cpu.getAF().hi);
        case ALU::XOR_HL: return CPU_8BIT_XOR(cpu.getAF().hi, cpu.readMemory(cpu.getHL().reg));
        case ALU::XOR_N: {
            BYTE n = cpu.readByte();
            return CPU_8BIT_XOR(cpu.getAF().hi, n);
        }

        // CP r
        case ALU::CP_B: return CPU_8BIT_CP(cpu.getAF().hi, cpu.getBC().hi);
        case ALU::CP_C: return CPU_8BIT_CP(cpu.getAF().hi, cpu.getBC().lo);
        case ALU::CP_D: return CPU_8BIT_CP(cpu.getAF().hi, cpu.getDE().hi);
        case ALU::CP_E: return CPU_8BIT_CP(cpu.getAF().hi, cpu.getDE().lo);
        case ALU::CP_H: return CPU_8BIT_CP(cpu.getAF().hi, cpu.getHL().hi);
        case ALU::CP_L: return CPU_8BIT_CP(cpu.getAF().hi, cpu.getHL().lo);
        case ALU::CP_HL: return CPU_8BIT_CP(cpu.getAF().hi, cpu.readMemory(cpu.getHL().reg));
        case ALU::CP_A: return CPU_8BIT_CP(cpu.getAF().hi, cpu.getAF().hi);
        case ALU::CP_N: {
            BYTE value = cpu.readByte(); // Read the next byte
            CPU_CP(value);
            return 8; // 8 cycles for CP n
        }

        // INC r
        case ALU::INC_B: return CPU_8BIT_INC(cpu.getBC().hi);
        case ALU::INC_C: return CPU_8BIT_INC(cpu.getBC().lo);
        case ALU::INC_D: return CPU_8BIT_INC(cpu.getDE().hi);
        case ALU::INC_E: return CPU_8BIT_INC(cpu.getDE().lo);
        case ALU::INC_H: return CPU_8BIT_INC(cpu.getHL().hi);
        case ALU::INC_L: return CPU_8BIT_INC(cpu.getHL().lo);
        case ALU::INC_A: return CPU_8BIT_INC(cpu.getAF().hi);
        // INC (HL) is missing, but it's not a direct register operation

        // DEC r
        case ALU::DEC_B: return CPU_8BIT_DEC(cpu.getBC().hi);
        case ALU::DEC_C: return CPU_8BIT_DEC(cpu.getBC().lo);
        case ALU::DEC_D: return CPU_8BIT_DEC(cpu.getDE().hi);
        case ALU::DEC_E: return CPU_8BIT_DEC(cpu.getDE().lo);
        case ALU::DEC_H: return CPU_8BIT_DEC(cpu.getHL().hi);
        case ALU::DEC_L: return CPU_8BIT_DEC(cpu.getHL().lo);
        case ALU::DEC_A: return CPU_8BIT_DEC(cpu.getAF().hi);
        // DEC (HL) is missing, but it's not a direct register operation
        // Add to ALUInstructions::execute switch statement:

        case 0x27: // DAA - Decimal Adjust Accumulator
            return CPU_DAA();

        case 0x29: // ADD HL, HL - Add HL to HL
            return CPU_ADD_HL_HL();

        default: 
            // Log error for unhandled opcode within this unit
            // Or, if this should never happen, assert or throw
            logUnhandledOpcode(opcode);
            return cpu.handleUnknownOpcode(opcode); // Or return a fixed cycle count like 4
    }
}
/*

constexpr BYTE FLAG_MASK_Z = 0x80;  // Zero Flag
constexpr BYTE FLAG_MASK_N = 0x40;  // Subtract Flag
constexpr BYTE FLAG_MASK_H = 0x20;  // Half Carry Flag
constexpr BYTE FLAG_MASK_C = 0x10;  // Carry Flag
*/
// Implement the DAA function:
int ALUInstructions::CPU_DAA() {
    // This implements the Decimal Adjust Accumulator instruction
    // which is used to correct BCD (Binary Coded Decimal) arithmetic
    
    BYTE a = cpu.getAF().hi;
    BYTE flags = cpu.getAF().lo;
    
    // Check for Half Carry or out of BCD range (0-9) in lower digit
    if ((flags & FLAG_MASK_H) || (a & 0x0F) > 9) {
        a += 0x06;
    }
    
    // Check for Carry or out of BCD range (0-9) in upper digit
    if ((flags & FLAG_MASK_C) || a > 0x9F) {
        a += 0x60;
        flags |= FLAG_MASK_C;
    } else {
        flags &= ~FLAG_MASK_C;
    }
    
    // Clear the H flag
    flags &= ~FLAG_MASK_H;
    
    // Set the Z flag if result is zero
    if (a == 0) {
        flags |= FLAG_MASK_Z;
    } else {
        flags &= ~FLAG_MASK_Z;
    }
    
    // Update A and flags
    cpu.getAF().hi = a;
    cpu.getAF().lo = flags;
    
    return 4;
}

// Implement the ADD HL, HL function:
int ALUInstructions::CPU_ADD_HL_HL() {
    // This adds the value in HL to itself
    WORD hl_value = cpu.getHL().reg;
    WORD result = hl_value + hl_value;
    
    // Set flags
    BYTE flags = cpu.getAF().lo;
    
    // Clear N flag
    flags &= ~FLAG_MASK_N;
    
    // Set H flag if carry from bit 11
    if ((((hl_value & 0x0FFF) + (hl_value & 0x0FFF)) & 0x1000) != 0) {
        flags |= FLAG_MASK_H;
    } else {
        flags &= ~FLAG_MASK_H;
    }
    
    // Set C flag if carry from bit 15
    if (result < hl_value) {
        flags |= FLAG_MASK_C;
    } else {
        flags &= ~FLAG_MASK_C;
    }
    
    // Update flags
    cpu.getAF().lo = flags;
    
    // Set result
    cpu.getHL().reg = result;
    
    return 8;
}


void ALUInstructions::CPU_CP(BYTE value) {
    // Compare A with value
    BYTE a = cpu.getAF().hi;
    
    // Set flags:
    // Z - Set if result is zero (A == value)
    // N - Set to 1
    // H - Set if no borrow from bit 4
    // C - Set if no borrow (A < value)
    
    setZeroFlag(a == value);
    setSubtractFlag(true);
    setHalfCarryFlag((a & 0xF) < (value & 0xF));
    setCarryFlag(a < value);
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