#include "cpu_instructions/ALU_instructions.h"
#include "../cpu.h"           // For CPU class, flag masks, register access
#include "../common.h"        // For logUnhandledOpcode
#include "../cpu_constants.h" // For FULL_OPCODE_TABLE

// Note: Removed 'using namespace CPUConstants;'

int ALUInstructions::execute(BYTE opcode) {
    BYTE value; // For operand

    // 8-bit ALU operations (opcodes 0x80 - 0xBF and 0xC6, 0xD6, 0xE6, 0xF6, 0xFE)
    if ((opcode >= 0x80 && opcode <= 0xBF) || 
        opcode == 0xC6 || opcode == 0xD6 || opcode == 0xE6 || opcode == 0xF6 || opcode == 0xFE) {
        
        BYTE operation = (opcode >> 3) & 0x07; // ADD, ADC, SUB, SBC, AND, XOR, OR, CP
        BYTE operand_src = opcode & 0x07;    // B, C, D, E, H, L, (HL), A

        // Determine the value to operate with
        if (opcode == 0xC6 || opcode == 0xD6 || opcode == 0xE6 || opcode == 0xF6 || opcode == 0xFE) { // ADD A,n; ADC A,n etc.
            value = cpu.readByte();
            // operation is implicitly defined by the opcode itself for these cases
            if (opcode == 0xC6) operation = 0; // ADD A, n
            else if (opcode == 0xD6) operation = 2; // SUB n
            else if (opcode == 0xE6) operation = 4; // AND n
            else if (opcode == 0xF6) operation = 6; // OR n
            else if (opcode == 0xCE) operation = 1; // ADC A, n (Corrected opcode from comment)
            else if (opcode == 0xDE) operation = 3; // SBC A, n (Corrected opcode from comment)
            else if (opcode == 0xEE) operation = 5; // XOR n
            else if (opcode == 0xFE) operation = 7; // CP n
        } else if (operand_src == 0x06) { // (HL)
            value = cpu.readMemory(cpu.getHL_ref().reg);
        } else { // Register
            switch (operand_src) {
                case 0: value = cpu.getB(); break;
                case 1: value = cpu.getC(); break;
                case 2: value = cpu.getD(); break;
                case 3: value = cpu.getE(); break;
                case 4: value = cpu.getH(); break;
                case 5: value = cpu.getL(); break;
                case 7: value = cpu.getA(); break;
                default: logUnhandledOpcode(opcode); return CPUConstants::FULL_OPCODE_TABLE[opcode].duration_cycles; // Should not happen
            }
        }

        switch (operation) {
            case 0: CPU_ADD_A(value, false); break; // ADD
            case 1: CPU_ADD_A(value, true);  break; // ADC
            case 2: CPU_SUB_A(value, false); break; // SUB
            case 3: CPU_SUB_A(value, true);  break; // SBC
            case 4: CPU_AND_A(value);        break; // AND
            case 5: CPU_XOR_A(value);        break; // XOR
            case 6: CPU_OR_A(value);         break; // OR
            case 7: CPU_CP_A(value);         break; // CP
        }
    } 
    // INC/DEC r, (HL)
    else if ((opcode & 0xC7) == 0x04 || (opcode & 0xC7) == 0x05 ) { // INC r (0x04,0x0C..0x3C), DEC r (0x05,0x0D..0x3D)
        // Check if it's a register or (HL)
        switch(opcode) {
            case 0x04: CPU_INC_REG(cpu.getB()); break; case 0x05: CPU_DEC_REG(cpu.getB()); break;
            case 0x0C: CPU_INC_REG(cpu.getC()); break; case 0x0D: CPU_DEC_REG(cpu.getC()); break;
            case 0x14: CPU_INC_REG(cpu.getD()); break; case 0x15: CPU_DEC_REG(cpu.getD()); break;
            case 0x1C: CPU_INC_REG(cpu.getE()); break; case 0x1D: CPU_DEC_REG(cpu.getE()); break;
            case 0x24: CPU_INC_REG(cpu.getH()); break; case 0x25: CPU_DEC_REG(cpu.getH()); break;
            case 0x2C: CPU_INC_REG(cpu.getL()); break; case 0x2D: CPU_DEC_REG(cpu.getL()); break;
            case 0x3C: CPU_INC_REG(cpu.getA()); break; case 0x3D: CPU_DEC_REG(cpu.getA()); break;
            case 0x34: CPU_INC_HL_MEM(); break;        case 0x35: CPU_DEC_HL_MEM(); break;
            default: logUnhandledOpcode(opcode); return CPUConstants::FULL_OPCODE_TABLE[opcode].duration_cycles;
        }
    }
    // 16-bit ADD HL,rr and ADD SP,n
    else if (opcode == 0x09 || opcode == 0x19 || opcode == 0x29 || opcode == 0x39 || opcode == 0xE8) {
        switch (opcode) {
            case 0x09: CPU_ADD_HL_RR(cpu.getBC_ref().reg); break; // ADD HL, BC
            case 0x19: CPU_ADD_HL_RR(cpu.getDE_ref().reg); break; // ADD HL, DE
            case 0x29: CPU_ADD_HL_RR(cpu.getHL_ref().reg); break; // ADD HL, HL
            case 0x39: CPU_ADD_HL_RR(cpu.getSP()); break;         // ADD HL, SP
            case 0xE8: CPU_ADD_SP_N(); break;                     // ADD SP, n
            default: logUnhandledOpcode(opcode); return CPUConstants::FULL_OPCODE_TABLE[opcode].duration_cycles;
        }
    }
    // Special ALU
    else if (opcode == 0x27) { // DAA
        CPU_DAA();
    }
    // Opcodes like CPL (0x2F), SCF (0x37), CCF (0x3F) are assumed to be in ControlInstructions
    else {
        logUnhandledOpcode(opcode);
        return cpu.handleUnknownOpcode(opcode); // Delegate if truly unknown
    }

    return CPUConstants::FULL_OPCODE_TABLE[opcode].duration_cycles;
}


// --- Helper Implementations ---

void ALUInstructions::CPU_ADD_A(BYTE value, bool use_carry) {
    BYTE a = cpu.getA();
    BYTE carry_val = (use_carry && cpu.getFlagC()) ? 1 : 0;
    
    WORD result_word = static_cast<WORD>(a) + value + carry_val;
    BYTE result_byte = static_cast<BYTE>(result_word);

    BYTE new_f = 0;
    if (result_byte == 0) new_f |= CPU::FLAG_Z_MASK;
    // N is 0 (false)
    if (((a & 0x0F) + (value & 0x0F) + carry_val) > 0x0F) new_f |= CPU::FLAG_H_MASK;
    if (result_word > 0xFF) new_f |= CPU::FLAG_C_MASK;
    
    cpu.getA() = result_byte;
    cpu.setFlags(new_f);
}

void ALUInstructions::CPU_SUB_A(BYTE value, bool use_carry) {
    BYTE a = cpu.getA();
    BYTE carry_val = (use_carry && cpu.getFlagC()) ? 1 : 0;

    BYTE result_byte = a - value - carry_val;
    // For flags, use intermediate int to avoid underflow issues with BYTEs directly
    int result_int = static_cast<int>(a) - value - carry_val;


    BYTE new_f = CPU::FLAG_N_MASK; // N is 1
    if (result_byte == 0) new_f |= CPU::FLAG_Z_MASK;
    if (((static_cast<int>(a & 0x0F) - (value & 0x0F) - carry_val) < 0)) new_f |= CPU::FLAG_H_MASK;
    if (result_int < 0) new_f |= CPU::FLAG_C_MASK;
    
    cpu.getA() = result_byte;
    cpu.setFlags(new_f);
}

void ALUInstructions::CPU_AND_A(BYTE value) {
    BYTE& a_ref = cpu.getA();
    a_ref &= value;

    BYTE new_f = CPU::FLAG_H_MASK; // H is 1
    if (a_ref == 0) new_f |= CPU::FLAG_Z_MASK;
    // N is 0, C is 0
    cpu.setFlags(new_f);
}

void ALUInstructions::CPU_OR_A(BYTE value) {
    BYTE& a_ref = cpu.getA();
    a_ref |= value;

    BYTE new_f = 0; // N, H, C are 0
    if (a_ref == 0) new_f |= CPU::FLAG_Z_MASK;
    cpu.setFlags(new_f);
}

void ALUInstructions::CPU_XOR_A(BYTE value) {
    BYTE& a_ref = cpu.getA();
    a_ref ^= value;

    BYTE new_f = 0; // N, H, C are 0
    if (a_ref == 0) new_f |= CPU::FLAG_Z_MASK;
    cpu.setFlags(new_f);
}

void ALUInstructions::CPU_CP_A(BYTE value) {
    BYTE a = cpu.getA();
    
    BYTE new_f = CPU::FLAG_N_MASK; // N is 1
    if (a == value) new_f |= CPU::FLAG_Z_MASK;
    // H: Set if no borrow from bit 4 (a_low < val_low)
    if ((a & 0x0F) < (value & 0x0F)) new_f |= CPU::FLAG_H_MASK;
    // C: Set if no borrow (a < val)
    if (a < value) new_f |= CPU::FLAG_C_MASK;
    
    cpu.setFlags(new_f);
}

void ALUInstructions::CPU_INC_REG(BYTE& reg_ref) {
    BYTE old_val = reg_ref;
    reg_ref++;
    
    BYTE new_f = cpu.getFlags() & CPU::FLAG_C_MASK; // Preserve C
    if (reg_ref == 0) new_f |= CPU::FLAG_Z_MASK;
    // N is 0
    if ((old_val & 0x0F) == 0x0F) new_f |= CPU::FLAG_H_MASK; // H if carry from bit 3
    
    cpu.setFlags(new_f);
}

void ALUInstructions::CPU_DEC_REG(BYTE& reg_ref) {
    BYTE old_val = reg_ref;
    reg_ref--;
    
    BYTE new_f = (cpu.getFlags() & CPU::FLAG_C_MASK) | CPU::FLAG_N_MASK; // Preserve C, Set N
    if (reg_ref == 0) new_f |= CPU::FLAG_Z_MASK;
    if ((old_val & 0x0F) == 0x00) new_f |= CPU::FLAG_H_MASK; // H if borrow from bit 4
    
    cpu.setFlags(new_f);
}

void ALUInstructions::CPU_INC_HL_MEM() {
    WORD address = cpu.getHL_ref().reg;
    BYTE val = cpu.readMemory(address);
    BYTE old_val = val;
    val++;
    cpu.writeMemory(address, val);
    
    BYTE new_f = cpu.getFlags() & CPU::FLAG_C_MASK; // Preserve C
    if (val == 0) new_f |= CPU::FLAG_Z_MASK;
    // N is 0
    if ((old_val & 0x0F) == 0x0F) new_f |= CPU::FLAG_H_MASK;
    
    cpu.setFlags(new_f);
}

void ALUInstructions::CPU_DEC_HL_MEM() {
    WORD address = cpu.getHL_ref().reg;
    BYTE val = cpu.readMemory(address);
    BYTE old_val = val;
    val--;
    cpu.writeMemory(address, val);
    
    BYTE new_f = (cpu.getFlags() & CPU::FLAG_C_MASK) | CPU::FLAG_N_MASK; // Preserve C, Set N
    if (val == 0) new_f |= CPU::FLAG_Z_MASK;
    if ((old_val & 0x0F) == 0x00) new_f |= CPU::FLAG_H_MASK;
    
    cpu.setFlags(new_f);
}

void ALUInstructions::CPU_ADD_HL_RR(WORD rr_value) {
    WORD hl_val = cpu.getHL_ref().reg;
    WORD result_32 = static_cast<WORD>(hl_val) + rr_value;

    BYTE current_f = cpu.getFlags();
    // Z flag is preserved
    BYTE new_f = current_f & CPU::FLAG_Z_MASK; 
    // N is reset (0)
    
    // H: Carry from bit 11 to bit 12
    if (((hl_val & 0x0FFF) + (rr_value & 0x0FFF)) > 0x0FFF) {
        new_f |= CPU::FLAG_H_MASK;
    }
    // C: Carry from bit 15
    if (result_32 > 0xFFFF) {
        new_f |= CPU::FLAG_C_MASK;
    }
    
    cpu.getHL_ref().reg = static_cast<WORD>(result_32);
    cpu.setFlags(new_f);
}

void ALUInstructions::CPU_ADD_SP_N() {
    signed char n_signed = static_cast<signed char>(cpu.readByte());
    WORD current_sp = cpu.getSP();
    WORD result_sp = current_sp + n_signed;

    BYTE new_f = 0; // Z, N are 0
    // H: Carry from bit 3 of (SP_low + n)
    if (((current_sp & 0x0F) + (n_signed & 0x0F)) > 0x0F) new_f |= CPU::FLAG_H_MASK;
    // C: Carry from bit 7 of (SP_low + n)
    if (((current_sp & 0xFF) + (n_signed & 0xFF)) > 0xFF) new_f |= CPU::FLAG_C_MASK;
    
    cpu.setSP(result_sp);
    cpu.setFlags(new_f);
}


// DAA using the logic from the user's provided code, adapted for the new flag system
void ALUInstructions::CPU_DAA() {
    BYTE temp_a = cpu.getA();
    BYTE current_flags = cpu.getFlags();
    
    // Store N flag to be preserved, clear Z, H, C for recalculation based on DAA logic
    BYTE final_flags = current_flags & CPU::FLAG_N_MASK; 

    bool n_flag_is_set = (current_flags & CPU::FLAG_N_MASK) != 0;

    if (!n_flag_is_set) { // After addition (or if N is not considered for adjustment path by user's logic)
        if ((current_flags & CPU::FLAG_H_MASK) || (temp_a & 0x0F) > 9) {
            temp_a += 0x06;
        }
        if ((current_flags & CPU::FLAG_C_MASK) || temp_a > 0x99) { // Check 'a' after potential first adjustment
            temp_a += 0x60;
            final_flags |= CPU::FLAG_C_MASK; // Set C if this adjustment occurs
        }
        // If C was not set by the above, it remains cleared in final_flags for the C bit.
    } else { // After subtraction (Game Boy DAA specific logic)
        // User's DAA in the provided snippet did not show a distinct path for N=1.
        // Standard Gameboy DAA for N=1:
        if (current_flags & CPU::FLAG_C_MASK) { // If C was set (borrow occurred)
            temp_a -= 0x60;
            final_flags |= CPU::FLAG_C_MASK; // C remains set
        }
        if (current_flags & CPU::FLAG_H_MASK) { // If H was set (borrow occurred)
            temp_a -= 0x06;
        }
    }
    
    // H is always reset by DAA
    // final_flags &= ~CPU::FLAG_H_MASK; // H is already 0 in final_flags unless N is H. Explicitly clear.
                                      // H is reset by DAA instruction.
    
    if (temp_a == 0) {
        final_flags |= CPU::FLAG_Z_MASK;
    }
    // N is preserved (already in final_flags)
    // C is set if a carry occurred during the DAA adjustment for additions, or preserved if set during subtraction adjustment.

    cpu.getA() = temp_a;
    cpu.setFlags(final_flags);
}
void ALUInstructions::CPU_DAA_UserLogic() {
    BYTE a_val = cpu.getA();
    BYTE f_val = cpu.getFlags();
    BYTE new_f = f_val & CPU::FLAG_N_MASK; // Preserve N, clear Z,H,C

    if ((f_val & CPU::FLAG_H_MASK) || (a_val & 0x0F) > 9) {
        a_val += 0x06;
    }
    // Note: a_val might have been modified by the H-flag logic above
    if ((f_val & CPU::FLAG_C_MASK) || a_val > 0x9F) { 
        a_val += 0x60;
        new_f |= CPU::FLAG_C_MASK; // Set C
    }
    // else C remains cleared in new_f

    // H is reset (already cleared in new_f unless N is H)
    if (a_val == 0) {
        new_f |= CPU::FLAG_Z_MASK;
    }
    cpu.getA() = a_val;
    cpu.setFlags(new_f);
}


