#include "cpu_instructions/load_instructions.h"
#include "../cpu.h"
#include "../common.h"

namespace {
    // Load opcodes
    constexpr BYTE LD_B_B = 0x40;
    constexpr BYTE LD_B_C = 0x41;
    constexpr BYTE LD_B_D = 0x42;
    constexpr BYTE LD_B_E = 0x43;
    constexpr BYTE LD_B_H = 0x44;
    constexpr BYTE LD_B_L = 0x45;
    constexpr BYTE LD_B_HL = 0x46;
    constexpr BYTE LD_B_A = 0x47;
    // ...similar patterns for C, D, E, H, L, and A
}

int LoadInstructions::execute(BYTE opcode) {
    // Regular register-to-register loads and LD r,(HL) / LD (HL),r
    if (opcode >= 0x40 && opcode <= 0x7F) {
        // HALT instruction is 0x76, which is in this range but is not a load.
        if (opcode == 0x76) { // HALT instruction
            // This should be handled by ControlInstructions, but as a safeguard:
            logUnhandledOpcode(opcode); // Or specific log
            return cpu.handleUnknownOpcode(opcode);
        }

        BYTE destReg = (opcode >> 3) & 0x07;
        BYTE srcReg = opcode & 0x07;
        
        BYTE* dest_ptr = nullptr;
        BYTE src_val;

        // Handle LD (HL),r (destination is (HL))
        if (destReg == 6) { // Destination is (HL)
            switch(srcReg) {
                case 0: src_val = cpu.getBC().hi; break; // B
                case 1: src_val = cpu.getBC().lo; break; // C
                case 2: src_val = cpu.getDE().hi; break; // D
                case 3: src_val = cpu.getDE().lo; break; // E
                case 4: src_val = cpu.getHL().hi; break; // H
                case 5: src_val = cpu.getHL().lo; break; // L
                // case 6: // (HL) to (HL) is HALT (0x76), handled above or by Control unit.
                // This case should not be reached if 0x76 is HALT.
                // If it were LD (HL),(HL), it would be an invalid/unlikely instruction.
                // For safety, we can log if srcReg is 6 here.
                // if (srcReg == 6) { LOG_ERROR("LD (HL),(HL) attempted via generic path"); return cpu.handleUnknownOpcode(opcode); }
                case 7: src_val = cpu.getAF().hi; break; // A
                default: // Should not happen for valid opcodes 0x70-0x75, 0x77
                    logUnhandledOpcode(opcode);
                    return cpu.handleUnknownOpcode(opcode);
            }
            return CPU_LD_HL_R(src_val);
        }

        // Handle LD r,r' and LD r,(HL) (destination is a register)
        switch(destReg) {
            case 0: dest_ptr = &cpu.getBC().hi; break; // B
            case 1: dest_ptr = &cpu.getBC().lo; break; // C
            case 2: dest_ptr = &cpu.getDE().hi; break; // D
            case 3: dest_ptr = &cpu.getDE().lo; break; // E
            case 4: dest_ptr = &cpu.getHL().hi; break; // H
            case 5: dest_ptr = &cpu.getHL().lo; break; // L
            // case 6: dest is (HL), handled above
            case 7: dest_ptr = &cpu.getAF().hi; break; // A
            default: // Should not happen
                logUnhandledOpcode(opcode);
                return cpu.handleUnknownOpcode(opcode);
        }

        // Get source value if destination is a register
        switch(srcReg) {
            case 0: src_val = cpu.getBC().hi; break; // B
            case 1: src_val = cpu.getBC().lo; break; // C
            case 2: src_val = cpu.getDE().hi; break; // D
            case 3: src_val = cpu.getDE().lo; break; // E
            case 4: src_val = cpu.getHL().hi; break; // H
            case 5: src_val = cpu.getHL().lo; break; // L
            case 6: return CPU_LD_R_HL(*dest_ptr); // LD r,(HL)
            case 7: src_val = cpu.getAF().hi; break; // A
            default: // Should not happen
                logUnhandledOpcode(opcode);
                return cpu.handleUnknownOpcode(opcode);
        }
        
        // If we reached here, it's LD r,r'
        return CPU_LD_R_R(*dest_ptr, src_val);
    }

    // Special load instructions
    switch(opcode) {
        case 0x02: return CPU_LD_BC_A();   // LD (BC),A
        case 0x12: return CPU_LD_DE_A();   // LD (DE),A
        case 0x22: return CPU_LD_HLI_A();  // LD (HL+),A
        case 0x32: return CPU_LD_HLD_A();  // LD (HL-),A
        case 0x0A: return CPU_LD_A_BC();   // LD A,(BC)
        case 0x1A: return CPU_LD_A_DE();   // LD A,(DE)
        case 0x2A: return CPU_LD_A_HLI();  // LD A,(HL+)
        case 0x3A: return CPU_LD_A_HLD();  // LD A,(HL-)
        case 0xF0: return CPU_LD_A_FF00_N(); // LD A,($FF00+n)
        case 0xE0: return CPU_LD_FF00_N_A(); // LD ($FF00+n),A
        // 8-bit immediate loads
        case 0x06: // LD B,d8
            cpu.getBC().hi = cpu.readByte();
            return 8;
            
        case 0x0E: // LD C,d8
            cpu.getBC().lo = cpu.readByte();
            return 8;
            
        case 0x3E: // LD A,d8
            cpu.getAF().hi = cpu.readByte();
            return 8;
        
        // 16-bit immediate loads
        case 0x01: // LD BC,d16
            cpu.getBC().reg = cpu.readWord();
            return 12;
            
        case 0x21: // LD HL,d16
            cpu.getHL().reg = cpu.readWord();
            return 12;
        // ADD MISSING LOAD INSTRUCTIONS HERE
        case 0x36: // LD (HL), n - Store immediate value n into the memory location pointed to by HL
            return CPU_LD_HL_n();
        
        case 0xEA: // LD (nn), A - Store value of A into memory at immediate address nn
            return CPU_LD_nn_A();
        case 0x31: // LD SP, nn - Load immediate 16-bit value into Stack Pointer
            return CPU_LD_SP_nn();
        case 0xE2: // LD (C), A - Store A to address $FF00+C
            return CPU_LD_FF00_C_A();
        
        default: 
            logUnhandledOpcode(opcode);
            return cpu.handleUnknownOpcode(opcode); 
    }
}
int LoadInstructions::CPU_LD_SP_nn() {
    // Read the 16-bit immediate value
    WORD value = cpu.readWord();
    
    // Set the stack pointer to this value
    cpu.getSP().reg = value; // Assuming getSP() returns a reference to the SP register
    
    // This instruction takes 12 cycles (3 M-cycles)
    return 12;
}
// And implement the function:
int LoadInstructions::CPU_LD_FF00_C_A() {
    // Calculate the address by adding C to 0xFF00
    WORD address = 0xFF00 + cpu.getBC().lo;
    
    // Store A at the calculated address
    cpu.writeMemory(address, cpu.getAF().hi);
    
    // This instruction takes 8 cycles
    return 8;
}
int LoadInstructions::CPU_LD_HL_n() {
    // Read the immediate byte value from PC
    BYTE value = cpu.readByte();
    
    // Get the address from HL register
    WORD address = cpu.getHL().reg; // or cpu.m_RegisterHL.reg; // Assuming m_RegisterHL is a member of CPU class
    
    // Write the value to memory at address HL
    cpu.writeMemory(address, value);
    
    // This instruction takes 12 cycles (3 M-cycles)
    return 12;
}

// LD (nn), A - Store value of A into memory at immediate address
int LoadInstructions::CPU_LD_nn_A() {
    // Read the 16-bit immediate address (low byte first, then high byte)
    BYTE low = cpu.readByte();
    BYTE high = cpu.readByte();
    WORD address = (high << 8) | low;
    
    // Get the value from register A
    BYTE value = cpu.getAF().hi; // Updated to use getAF() method
    
    // Write the value to memory at the specified address
    cpu.writeMemory(address, value);
    
    // This instruction takes 16 cycles (4 M-cycles)
    return 16;
}
int LoadInstructions::CPU_LD_R_R(BYTE& dest, BYTE src) {
    dest = src;
    return 4;
}
int LoadInstructions::CPU_LD_HL_R(BYTE src_val) {
    cpu.writeMemory(cpu.getHL().reg, src_val);
    return 8; // These instructions take 8 cycles
}

int LoadInstructions::CPU_LD_R_HL(BYTE& reg) {
    reg = cpu.readMemory(cpu.getHL().reg);
    return 8;
}

int LoadInstructions::CPU_LD_A_BC() {
    cpu.getAF().hi = cpu.readMemory(cpu.getBC().reg);
    return 8;
}

int LoadInstructions::CPU_LD_A_DE() {
    cpu.getAF().hi = cpu.readMemory(cpu.getDE().reg);
    return 8;
}

int LoadInstructions::CPU_LD_BC_A() {
    cpu.writeMemory(cpu.getBC().reg, cpu.getAF().hi);
    return 8;
}

int LoadInstructions::CPU_LD_DE_A() {
    cpu.writeMemory(cpu.getDE().reg, cpu.getAF().hi);
    return 8;
}

int LoadInstructions::CPU_LD_A_FF00_N() {
    BYTE offset = cpu.readByte();
    cpu.getAF().hi = cpu.readMemory(0xFF00 + offset);
    return 12;
}

int LoadInstructions::CPU_LD_FF00_N_A() {
    BYTE offset = cpu.readByte();
    cpu.writeMemory(0xFF00 + offset, cpu.getAF().hi);
    return 12;
}

int LoadInstructions::CPU_LD_HLI_A() {
    cpu.writeMemory(cpu.getHL().reg++, cpu.getAF().hi);
    return 8;
}

int LoadInstructions::CPU_LD_HLD_A() {
    cpu.writeMemory(cpu.getHL().reg--, cpu.getAF().hi);
    return 8;
}

int LoadInstructions::CPU_LD_A_HLI() {
    cpu.getAF().hi = cpu.readMemory(cpu.getHL().reg++);
    return 8;
}

int LoadInstructions::CPU_LD_A_HLD() {
    cpu.getAF().hi = cpu.readMemory(cpu.getHL().reg--);
    return 8;
}