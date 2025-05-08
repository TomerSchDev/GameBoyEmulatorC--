#include "cpu_instructions/load_instructions.h"
#include "../cpu.h"
#include "../common.h"      // For logUnhandledOpcode
#include "../cpu_constants.h" // For FULL_OPCODE_TABLE

// Helper to get a reference to an 8-bit register based on a 3-bit index
// Index: 0:B, 1:C, 2:D, 3:E, 4:H, 5:L, 7:A. Index 6 is (HL) and handled separately.
BYTE& LoadInstructions::getRegisterReference(BYTE reg_index) {
    switch (reg_index) {
        case 0: return cpu.getB();
        case 1: return cpu.getC();
        case 2: return cpu.getD();
        case 3: return cpu.getE();
        case 4: return cpu.getH();
        case 5: return cpu.getL();
        case 7: return cpu.getA();
        default:
            // This should not be called with reg_index == 6 or invalid index
            LOG_ERROR("Invalid register index for getRegisterReference: " + std::to_string(reg_index));
            static BYTE dummy = 0; // Should ideally throw or handle error more robustly
            return dummy;
    }
}

// Helper to get a value from an 8-bit register or (HL) based on a 3-bit index
// Index: 0:B, 1:C, 2:D, 3:E, 4:H, 5:L, 6:(HL), 7:A
BYTE LoadInstructions::getRegisterValue(BYTE reg_index, bool& is_hl_memory) {
    is_hl_memory = false;
    switch (reg_index) {
        case 0: return cpu.getB();
        case 1: return cpu.getC();
        case 2: return cpu.getD();
        case 3: return cpu.getE();
        case 4: return cpu.getH();
        case 5: return cpu.getL();
        case 6: 
            is_hl_memory = true;
            return cpu.readMemory(cpu.getHL_ref().reg); // Read from (HL)
        case 7: return cpu.getA();
        default:
            LOG_ERROR("Invalid register index for getRegisterValue: " + std::to_string(reg_index));
            return 0; // Should ideally throw or handle error more robustly
    }
}


int LoadInstructions::execute(BYTE opcode) {
    // LD r,r' ; LD r,(HL) ; LD (HL),r  (Opcodes 0x40 - 0x7F, excluding 0x76 HALT)
    if (opcode >= 0x40 && opcode <= 0x7F) {
        if (opcode == 0x76) { // HALT instruction
            // This should be handled by ControlInstructions. If it reaches here, it's an issue.
            LOG_WARNING("HALT (0x76) encountered in LoadInstructions unit.");
            return cpu.handleUnknownOpcode(opcode); // Delegate to CPU's unknown opcode handler
        }

        BYTE dest_idx = (opcode >> 3) & 0x07; // Destination: 0-7 (B,C,D,E,H,L,(HL),A)
        BYTE src_idx  = opcode & 0x07;        // Source: 0-7 (B,C,D,E,H,L,(HL),A)
        bool src_is_hl_mem;

        if (dest_idx == 6) { // Destination is (HL): LD (HL), r_src
            BYTE src_val = getRegisterValue(src_idx, src_is_hl_mem); // src_is_hl_mem will be false here as src_idx won't be 6 for LD (HL),r
            CPU_LD_HL_R(src_val);
        } else { // Destination is a register r_dest
            BYTE& dest_ref = getRegisterReference(dest_idx);
            if (src_idx == 6) { // Source is (HL): LD r_dest, (HL)
                CPU_LD_R_HL(dest_ref);
            } else { // Source is a register r_src: LD r_dest, r_src
                BYTE src_val = getRegisterValue(src_idx, src_is_hl_mem);
                CPU_LD_R_R(dest_ref, src_val);
            }
        }
    } else {
        // Specific Load Instructions
        switch (opcode) {
            // 8-bit loads
            case 0x06: CPU_LD_B_d8(); break;
            case 0x0E: CPU_LD_C_d8(); break;
            case 0x16: CPU_LD_D_d8(); break;
            case 0x1E: CPU_LD_E_d8(); break;
            case 0x26: CPU_LD_H_d8(); break;
            case 0x2E: CPU_LD_L_d8(); break;
            case 0x3E: CPU_LD_A_d8(); break;
            case 0x36: CPU_LD_HL_d8(); break; // LD (HL), d8

            case 0x02: CPU_LD_BC_A(); break;
            case 0x12: CPU_LD_DE_A(); break;
            case 0x22: CPU_LD_HLI_A(); break;
            case 0x32: CPU_LD_HLD_A(); break;

            case 0x0A: CPU_LD_A_BC(); break;
            case 0x1A: CPU_LD_A_DE(); break;
            case 0x2A: CPU_LD_A_HLI(); break;
            case 0x3A: CPU_LD_A_HLD(); break;

            // 16-bit loads
            case 0x01: CPU_LD_BC_d16(); break;
            case 0x11: CPU_LD_DE_d16(); break;
            case 0x21: CPU_LD_HL_d16(); break;
            case 0x31: CPU_LD_SP_d16(); break;

            // High RAM loads
            case 0xF0: CPU_LD_A_FF00_N(); break;
            case 0xE0: CPU_LD_FF00_N_A(); break;
            case 0xF2: CPU_LD_A_FF00_C(); break;
            case 0xE2: CPU_LD_FF00_C_A(); break;

            // LD A,(nn) and LD (nn),A
            case 0xFA: CPU_LD_A_NN(); break;
            case 0xEA: CPU_LD_NN_A(); break;

            // Misc 16-bit loads
            case 0xF9: CPU_LD_SP_HL(); break;
            case 0xF8: CPU_LDHL_SP_N(); break;
            
            // Opcodes like 0x08 LD (nn),SP are handled by specific cases if they exist
            // Example: LD (a16), SP (Opcode 0x08)
            case 0x08: {
                WORD address = cpu.readWord();
                cpu.writeMemory(address, cpu.getSP() & 0xFF); // Low byte of SP
                cpu.writeMemory(address + 1, (cpu.getSP() >> 8) & 0xFF); // High byte of SP
                break;
            }

            default:
                logUnhandledOpcode(opcode);
                return cpu.handleUnknownOpcode(opcode); // Delegate if truly unknown to this unit
        }
    }
    return CPUConstants::FULL_OPCODE_TABLE[opcode].duration_cycles;
}

// --- Helper Implementations (all void) ---

void LoadInstructions::CPU_LD_R_R(BYTE& dest_reg_ref, BYTE src_val) {
    dest_reg_ref = src_val;
}

void LoadInstructions::CPU_LD_R_HL(BYTE& dest_reg_ref) {
    dest_reg_ref = cpu.readMemory(cpu.getHL_ref().reg);
}

void LoadInstructions::CPU_LD_HL_R(BYTE src_val) {
    cpu.writeMemory(cpu.getHL_ref().reg, src_val);
}

// 8-bit immediate loads
void LoadInstructions::CPU_LD_B_d8() { cpu.getB() = cpu.readByte(); }
void LoadInstructions::CPU_LD_C_d8() { cpu.getC() = cpu.readByte(); }
void LoadInstructions::CPU_LD_D_d8() { cpu.getD() = cpu.readByte(); }
void LoadInstructions::CPU_LD_E_d8() { cpu.getE() = cpu.readByte(); }
void LoadInstructions::CPU_LD_H_d8() { cpu.getH() = cpu.readByte(); }
void LoadInstructions::CPU_LD_L_d8() { cpu.getL() = cpu.readByte(); }
void LoadInstructions::CPU_LD_A_d8() { cpu.getA() = cpu.readByte(); }
void LoadInstructions::CPU_LD_HL_d8() { cpu.writeMemory(cpu.getHL_ref().reg, cpu.readByte()); }

// LD (rr), A
void LoadInstructions::CPU_LD_BC_A() { cpu.writeMemory(cpu.getBC_ref().reg, cpu.getA()); }
void LoadInstructions::CPU_LD_DE_A() { cpu.writeMemory(cpu.getDE_ref().reg, cpu.getA()); }
void LoadInstructions::CPU_LD_HLI_A() { cpu.writeMemory(cpu.getHL_ref().reg++, cpu.getA()); }
void LoadInstructions::CPU_LD_HLD_A() { cpu.writeMemory(cpu.getHL_ref().reg--, cpu.getA()); }

// LD A, (rr)
void LoadInstructions::CPU_LD_A_BC() { cpu.getA() = cpu.readMemory(cpu.getBC_ref().reg); }
void LoadInstructions::CPU_LD_A_DE() { cpu.getA() = cpu.readMemory(cpu.getDE_ref().reg); }
void LoadInstructions::CPU_LD_A_HLI() { cpu.getA() = cpu.readMemory(cpu.getHL_ref().reg++); }
void LoadInstructions::CPU_LD_A_HLD() { cpu.getA() = cpu.readMemory(cpu.getHL_ref().reg--); }

// 16-bit immediate loads
void LoadInstructions::CPU_LD_BC_d16() { cpu.getBC_ref().reg = cpu.readWord(); }
void LoadInstructions::CPU_LD_DE_d16() { cpu.getDE_ref().reg = cpu.readWord(); }
void LoadInstructions::CPU_LD_HL_d16() { cpu.getHL_ref().reg = cpu.readWord(); }
void LoadInstructions::CPU_LD_SP_d16() { cpu.setSP(cpu.readWord()); } // Assuming cpu.setSP()

// High RAM loads
void LoadInstructions::CPU_LD_A_FF00_N() { cpu.getA() = cpu.readMemory(0xFF00 + cpu.readByte()); }
void LoadInstructions::CPU_LD_FF00_N_A() { cpu.writeMemory(0xFF00 + cpu.readByte(), cpu.getA()); }
void LoadInstructions::CPU_LD_A_FF00_C() { cpu.getA() = cpu.readMemory(0xFF00 + cpu.getC()); }
void LoadInstructions::CPU_LD_FF00_C_A() { cpu.writeMemory(0xFF00 + cpu.getC(), cpu.getA()); }

// LD A,(nn) and LD (nn),A
void LoadInstructions::CPU_LD_A_NN() { cpu.getA() = cpu.readMemory(cpu.readWord()); }
void LoadInstructions::CPU_LD_NN_A() { cpu.writeMemory(cpu.readWord(), cpu.getA()); }

// Misc 16-bit loads
void LoadInstructions::CPU_LD_SP_HL() { cpu.setSP(cpu.getHL_ref().reg); }
void LoadInstructions::CPU_LDHL_SP_N() {
    BYTE n_signed = cpu.readByte();
    WORD current_sp = cpu.getSP();
    WORD result;
    int temp_result = static_cast<int>(current_sp) + static_cast<signed char>(n_signed);
    result = static_cast<WORD>(temp_result);
    cpu.getHL_ref().reg = result;

    // Flags for LD HL, SP+n: Z=0, N=0, H and C are set based on carry/borrow from bit 3/7 of SP+n
    BYTE flags = 0;
    // Half Carry: Carry from bit 3 to bit 4
    if (((current_sp & 0x0F) + (n_signed & 0x0F)) > 0x0F) {
        flags |= CPU::FLAG_H_MASK;
    }
    // Carry: Carry from bit 7 to bit 8 (for the lower byte of SP)
    // More accurately, for the whole 16-bit addition, if there's a carry out of bit 7 of the lower byte sum
    // or carry out of bit 15 of the full sum.
    // For SP+n, C is set if carry from bit 7, H if carry from bit 3.
    // This applies to the addition of the lower byte of SP and n.
    // Let's use the common definition:
    // H: Set if carry from bit 3 of (SP & 0xFF) + n
    // C: Set if carry from bit 7 of (SP & 0xFF) + n
    // This seems to be for ADD SP,n. For LD HL,SP+n, flags are Z=0, N=0, H, C from SP+e.
    // The H and C flags are set depending on the carry from bit 3 and bit 7 of the addition SP + n (signed).
    // This is one of the few 16-bit operations that affect H and C.
    // Z and N are reset.
    // Check carry from bit 3 for H
    if (((current_sp & 0x0F) + (static_cast<signed char>(n_signed) & 0x0F)) & 0x10) flags |= CPU::FLAG_H_MASK;
    // Check carry from bit 7 for C
    if (((current_sp & 0xFF) + (static_cast<signed char>(n_signed) & 0xFF)) & 0x100) flags |= CPU::FLAG_C_MASK;
    
    cpu.setFlags(flags); // Z=0, N=0, H=?, C=?
}
