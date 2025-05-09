#include "instructions.h"
#include "cpu.h" // For CPU class definition and access to its members
#include "logger.h" // For logging
#include <stdexcept> // For std::runtime_error if needed for critical errors

namespace GB {

// Helper function to get a reference to an 8-bit register based on the enum
BYTE& get_reg_ref(CPU& cpu, GB::Register reg_enum) {
    switch (reg_enum) {
        case GB::Register::A: return cpu.getA();
        case GB::Register::B: return cpu.getB();
        case GB::Register::C: return cpu.getC();
        case GB::Register::D: return cpu.getD();
        case GB::Register::E: return cpu.getE();
        case GB::Register::H: return cpu.getH();
        case GB::Register::L: return cpu.getL();
        default:
            LOG_ERROR("get_reg_ref called with invalid register enum for 8-bit ref.");
            throw std::runtime_error("Invalid register for 8-bit ref"); // Should not happen
    }
}

// Helper function to get a 16-bit register pair's value
WORD get_reg_pair_val(CPU& cpu, GB::Register reg_enum) {
    switch (reg_enum) {
        case GB::Register::AF: return cpu.getAF();
        case GB::Register::BC: return cpu.getBC();
        case GB::Register::DE: return cpu.getDE();
        case GB::Register::HL: return cpu.getHL();
        case GB::Register::SP: return cpu.getSP();
        default:
            LOG_ERROR("get_reg_pair_val called with invalid register enum for 16-bit val.");
            throw std::runtime_error("Invalid register for 16-bit val");
    }
    
}

// Helper function to set a 16-bit register pair's value
void set_reg_pair_val(CPU& cpu, GB::Register reg_enum, WORD value) {
    switch (reg_enum) {
        case GB::Register::AF: cpu.setAF(value); break;
        case GB::Register::BC: cpu.setBC(value); break;
        case GB::Register::DE: cpu.setDE(value); break;
        case GB::Register::HL: cpu.setHL(value); break;
        case GB::Register::SP: cpu.setSP(value); break;
        default:
            LOG_ERROR("set_reg_pair_val called with invalid register enum for 16-bit val.");
            throw std::runtime_error("Invalid register for 16-bit val set");
    }
}


// --- Instruction Implementations ---

// Group: CONTROL_MISC
int NOP_impl(CPU& cpu, const OpcodeInfo& info) {
    // NOP: No operation.
    // Flags: Z N H C
    //        - - - -
    // No flags are changed.
    return info.cycles[0];
}

int HALT_impl(CPU& cpu, const OpcodeInfo& info) {
    // HALT: Power down CPU until an interrupt occurs.
    // If IME is 0 and IF&IE is non-zero, a HALT bug occurs.
    // For now, just set the halt state. Interrupt handling will wake it.
    // Flags: - - - -
    BYTE IE = cpu.readMemory(0xFFFF);
    BYTE IF = cpu.readMemory(0xFF0F);
    if (!cpu.isInterruptMasterEnabled() && (IE & IF & 0x1F) != 0) {
        // HALT bug: CPU fails to halt and reads next instruction twice.
        // This is complex to emulate perfectly here without more context on the main loop.
        // For a simple HALT, just set the state.
        // A more accurate emulation might need PC not to increment for the *next* fetch if bug triggered.
        LOG_WARNING("HALT bug condition met (IME=0, (IE&IF)!=0). Simple HALT for now.");
        // The bug itself means the instruction *after* HALT is executed.
        // Here, we'll just halt. The main loop might need to handle the bug's PC behavior.
        cpu.setHaltState(true);

    } else {
        cpu.setHaltState(true);
    }
    return info.cycles[0];
}

int STOP_impl(CPU& cpu, const OpcodeInfo& info) {
    // STOP: Halt CPU and LCD until a button is pressed.
    // Consumes a 0x00 byte after it (which is part of its 2-byte length in OpcodeInfo).
    // The OpcodeInfo length should be 2, and CPU::readBytePC would have consumed the 0x00.
    // Flags: - - - -
    cpu.setStopState(true);
    // Further hardware (like LCD controller, timer) might need to be informed or paused.
    LOG_INFO("CPU STOPPED. Waiting for button press (not emulated here).");
    return info.cycles[0];
}

int DI_impl(CPU& cpu, const OpcodeInfo& info) {
    // DI: Disable interrupts (clear IME flag).
    // Flags: - - - -
    cpu.disableInterrupts();
    return info.cycles[0];
}

int EI_impl(CPU& cpu, const OpcodeInfo& info) {
    // EI: Enable interrupts. Takes effect after the instruction *following* EI.
    // Flags: - - - -
    cpu.scheduleInterruptEnable();
    return info.cycles[0];
}

// --- 8-bit Load Instructions ---
int LD_reg_reg_impl(CPU& cpu, const OpcodeInfo& info) {
    // LD r, r' : Load value from register r' into register r.
    // Example: LD B, C (info.operand1 = Register::B, info.operand2 = Register::C)
    // Flags: - - - -
    BYTE& dest_reg = get_reg_ref(cpu, info.operand1);
    BYTE& source_reg = get_reg_ref(cpu, info.operand2);
    dest_reg = source_reg;
    return info.cycles[0];
}

int LD_reg_n8_impl(CPU& cpu, const OpcodeInfo& info) {
    // LD r, n8 : Load immediate 8-bit value n8 into register r.
    // Example: LD B, 0x05 (info.operand1 = Register::B, n8 is read from PC)
    // Flags: - - - -
    BYTE& dest_reg = get_reg_ref(cpu, info.operand1);
    dest_reg = cpu.readBytePC(); // n8 is the byte after the opcode
    return info.cycles[0];
}

int LD_reg_memHL_impl(CPU& cpu, const OpcodeInfo& info) {
    // LD r, (HL) : Load value from memory address pointed by HL into register r.
    // Example: LD B, (HL) (info.operand1 = Register::B)
    // Flags: - - - -
    BYTE& dest_reg = get_reg_ref(cpu, info.operand1);
    dest_reg = cpu.readMemory(cpu.getHL());
    return info.cycles[0];
}

int LD_memHL_reg_impl(CPU& cpu, const OpcodeInfo& info) {
    // LD (HL), r : Store value from register r into memory address pointed by HL.
    // Example: LD (HL), B (info.operand2 = Register::B)
    // Flags: - - - -
    BYTE& source_reg = get_reg_ref(cpu, info.operand2);
    cpu.writeMemory(cpu.getHL(), source_reg);
    return info.cycles[0];
}

int LD_memHL_n8_impl(CPU& cpu, const OpcodeInfo& info) {
    // LD (HL), n8 : Store immediate 8-bit value n8 into memory address pointed by HL.
    // Flags: - - - -
    BYTE immediate_val = cpu.readBytePC(); // n8 is the byte after the opcode
    cpu.writeMemory(cpu.getHL(), immediate_val);
    return info.cycles[0];
}

int LD_A_memBC_impl(CPU& cpu, const OpcodeInfo& info) {
    // LD A, (BC)
    // Flags: - - - -
    cpu.getA() = cpu.readMemory(cpu.getBC());
    return info.cycles[0];
}

int LD_A_memDE_impl(CPU& cpu, const OpcodeInfo& info) {
    // LD A, (DE)
    // Flags: - - - -
    cpu.getA() = cpu.readMemory(cpu.getDE());
    return info.cycles[0];
}

int LD_A_memA16_impl(CPU& cpu, const OpcodeInfo& info) {
    // LD A, (a16)
    // Flags: - - - -
    WORD address = cpu.readWordPC(); // a16 is the two bytes after the opcode
    cpu.getA() = cpu.readMemory(address);
    return info.cycles[0];
}

int LD_memBC_A_impl(CPU& cpu, const OpcodeInfo& info) {
    // LD (BC), A
    // Flags: - - - -
    cpu.writeMemory(cpu.getBC(), cpu.getA());
    return info.cycles[0];
}

int LD_memDE_A_impl(CPU& cpu, const OpcodeInfo& info) {
    // LD (DE), A
    // Flags: - - - -
    cpu.writeMemory(cpu.getDE(), cpu.getA());
    return info.cycles[0];
}

int LD_memA16_A_impl(CPU& cpu, const OpcodeInfo& info) {
    // LD (a16), A
    // Flags: - - - -
    WORD address = cpu.readWordPC(); // a16 is the two bytes after the opcode
    cpu.writeMemory(address, cpu.getA());
    return info.cycles[0];
}

int LDH_memA8_A_impl(CPU& cpu, const OpcodeInfo& info) {
    // LDH (a8), A  : LD (0xFF00 + a8), A
    // Flags: - - - -
    BYTE offset = cpu.readBytePC(); // a8 is the byte after the opcode
    cpu.writeMemory(0xFF00 + offset, cpu.getA());
    return info.cycles[0];
}

int LDH_A_memA8_impl(CPU& cpu, const OpcodeInfo& info) {
    // LDH A, (a8)  : LD A, (0xFF00 + a8)
    // Flags: - - - -
    BYTE offset = cpu.readBytePC(); // a8 is the byte after the opcode
    cpu.getA() = cpu.readMemory(0xFF00 + offset);
    return info.cycles[0];
}

int LDH_memC_A_impl(CPU& cpu, const OpcodeInfo& info) {
    // LDH (C), A   : LD (0xFF00 + C), A
    // Flags: - - - -
    cpu.writeMemory(0xFF00 + cpu.getC(), cpu.getA());
    return info.cycles[0];
}

int LDH_A_memC_impl(CPU& cpu, const OpcodeInfo& info) {
    // LDH A, (C)   : LD A, (0xFF00 + C)
    // Flags: - - - -
    cpu.getA() = cpu.readMemory(0xFF00 + cpu.getC());
    return info.cycles[0];
}

int LD_A_memHLI_impl(CPU& cpu, const OpcodeInfo& info) {
    // LD A, (HL+) : LD A, (HL) then HL = HL + 1
    // Flags: - - - -
    cpu.getA() = cpu.readMemory(cpu.getHL());
    cpu.setHL(cpu.getHL() + 1);
    return info.cycles[0];
}

int LD_A_memHLD_impl(CPU& cpu, const OpcodeInfo& info) {
    // LD A, (HL-) : LD A, (HL) then HL = HL - 1
    // Flags: - - - -
    cpu.getA() = cpu.readMemory(cpu.getHL());
    cpu.setHL(cpu.getHL() - 1);
    return info.cycles[0];
}

int LD_memHLI_A_impl(CPU& cpu, const OpcodeInfo& info) {
    // LD (HL+), A : LD (HL), A then HL = HL + 1
    // Flags: - - - -
    cpu.writeMemory(cpu.getHL(), cpu.getA());
    cpu.setHL(cpu.getHL() + 1);
    return info.cycles[0];
}

int LD_memHLD_A_impl(CPU& cpu, const OpcodeInfo& info) {
    // LD (HL-), A : LD (HL), A then HL = HL - 1
    // Flags: - - - -
    cpu.writeMemory(cpu.getHL(), cpu.getA());
    cpu.setHL(cpu.getHL() - 1);
    return info.cycles[0];
}


// --- 16-bit Load Instructions ---
int LD_rr_n16_impl(CPU& cpu, const OpcodeInfo& info) {
    // LD rr, n16 : Load immediate 16-bit value n16 into register pair rr.
    // rr can be BC, DE, HL, SP.
    // Flags: - - - -
    WORD immediate_val = cpu.readWordPC(); // n16 is the two bytes after the opcode
    set_reg_pair_val(cpu, info.operand1, immediate_val);
    return info.cycles[0];
}

int LD_SP_HL_impl(CPU& cpu, const OpcodeInfo& info) {
    // LD SP, HL
    // Flags: - - - -
    cpu.setSP(cpu.getHL());
    return info.cycles[0]; // Typically 8 cycles
}

int LD_memA16_SP_impl(CPU& cpu, const OpcodeInfo& info) {
    // LD (a16), SP : Store SP at address a16.
    // (a16) is LSB, (a16+1) is MSB of SP.
    // Flags: - - - -
    WORD address = cpu.readWordPC();
    WORD sp_val = cpu.getSP();
    cpu.writeMemory(address, sp_val & 0xFF);       // LSB
    cpu.writeMemory(address + 1, (sp_val >> 8) & 0xFF); // MSB
    return info.cycles[0]; // Typically 20 cycles
}

int LD_HL_SP_e8_impl(CPU& cpu, const OpcodeInfo& info) {
    // LD HL, SP+e8 : Load SP + signed immediate e8 into HL.
    // Flags: Z N H C
    //        0 0 H C (H and C are set according to 8-bit addition of SP_lo and e8)
    BYTE e8_unsigned = cpu.readBytePC(); // Read the byte
    signed char e8_signed = static_cast<signed char>(e8_unsigned); // Interpret as signed
    WORD sp_val = cpu.getSP();
    WORD result = sp_val + e8_signed; // Perform signed addition

    // Flags are based on the addition of the lower byte of SP and e8 for H and C
    BYTE sp_lo = sp_val & 0xFF;
    unsigned int temp_calc = sp_lo + e8_unsigned; // Use unsigned for flag calculation

    cpu.setFlagZ(false);
    cpu.setFlagN(false);
    // Half Carry: Carry from bit 3 to bit 4 in (sp_lo & 0xF) + (e8 & 0xF)
    cpu.setFlagH(((sp_lo & 0xF) + (e8_unsigned & 0xF)) > 0xF);
    // Carry: Carry from bit 7 in sp_lo + e8
    cpu.setFlagC(temp_calc > 0xFF);

    cpu.setHL(result);
    return info.cycles[0]; // Typically 12 cycles
}


int PUSH_rr_impl(CPU& cpu, const OpcodeInfo& info) {
    // PUSH rr : Push register pair rr onto the stack.
    // rr can be AF, BC, DE, HL.
    // Flags: - - - -
    WORD val_to_push = get_reg_pair_val(cpu, info.operand1);
    // No need to mask AF here, the setAF ensures F's lower bits are 0 when read via getAF
    cpu.pushStackWord(val_to_push);
    return info.cycles[0]; // Typically 16 cycles
}

int POP_rr_impl(CPU& cpu, const OpcodeInfo& info) {
    // POP rr : Pop value from stack into register pair rr.
    // rr can be AF, BC, DE, HL.
    // Flags: Z N H C (if rr is AF, flags are set from popped value, lower bits masked)
    //        - - - - (otherwise)
    WORD popped_val = cpu.popStackWord();
    if (info.operand1 == GB::Register::AF) {
        // Lower 4 bits of F register are not writable, should be masked.
        popped_val &= 0xFFF0;
    }
    set_reg_pair_val(cpu, info.operand1, popped_val);
    return info.cycles[0]; // Typically 12 cycles
}

// --- 8-bit ALU Instructions ---
// Helper for 8-bit ADD/ADC
void alu_add_base(CPU& cpu, BYTE value, bool with_carry) {
    BYTE current_a = cpu.getA();
    BYTE carry_val = (with_carry && cpu.getFlagC()) ? 1 : 0;
    WORD result = static_cast<WORD>(current_a) + value + carry_val;

    cpu.setFlagZ((result & 0xFF) == 0);
    cpu.setFlagN(false);
    // Half Carry: Carry from bit 3 to bit 4
    cpu.setFlagH(((current_a & 0xF) + (value & 0xF) + carry_val) > 0xF);
    // Carry: Carry from bit 7
    cpu.setFlagC(result > 0xFF);
    cpu.getA() = static_cast<BYTE>(result & 0xFF);
}

int ADD_A_reg_impl(CPU& cpu, const OpcodeInfo& info) {
    alu_add_base(cpu, get_reg_ref(cpu, info.operand2), false);
    return info.cycles[0];
}
int ADD_A_n8_impl(CPU& cpu, const OpcodeInfo& info) {
    alu_add_base(cpu, cpu.readBytePC(), false);
    return info.cycles[0];
}
int ADD_A_memHL_impl(CPU& cpu, const OpcodeInfo& info) {
    alu_add_base(cpu, cpu.readMemory(cpu.getHL()), false);
    return info.cycles[0];
}
int ADC_A_reg_impl(CPU& cpu, const OpcodeInfo& info) {
    alu_add_base(cpu, get_reg_ref(cpu, info.operand2), true);
    return info.cycles[0];
}
int ADC_A_n8_impl(CPU& cpu, const OpcodeInfo& info) {
    alu_add_base(cpu, cpu.readBytePC(), true);
    return info.cycles[0];
}
int ADC_A_memHL_impl(CPU& cpu, const OpcodeInfo& info) {
    alu_add_base(cpu, cpu.readMemory(cpu.getHL()), true);
    return info.cycles[0];
}

// Helper for 8-bit SUB/SBC/CP
void alu_sub_base(CPU& cpu, BYTE value, bool with_carry, bool is_cp) {
    BYTE current_a = cpu.getA();
    BYTE carry_val = (with_carry && cpu.getFlagC()) ? 1 : 0;
    // Use unsigned arithmetic for borrow checks
    unsigned int result_unsigned = current_a - value - carry_val;

    cpu.setFlagZ((result_unsigned & 0xFF) == 0);
    cpu.setFlagN(true);
    // Half Carry (Borrow from bit 4): Check if lower nibble calculation borrows
    cpu.setFlagH((static_cast<int>(current_a & 0xF) - static_cast<int>(value & 0xF) - static_cast<int>(carry_val)) < 0);
    // Carry (Borrow from bit 8): Check if full calculation borrows
    cpu.setFlagC((static_cast<int>(current_a) - static_cast<int>(value) - static_cast<int>(carry_val)) < 0);

    if (!is_cp) { // CP only sets flags, doesn't change A
        cpu.getA() = static_cast<BYTE>(result_unsigned & 0xFF);
    }
}

int SUB_A_reg_impl(CPU& cpu, const OpcodeInfo& info) {
    alu_sub_base(cpu, get_reg_ref(cpu, info.operand2), false, false);
    return info.cycles[0];
}
int SUB_A_n8_impl(CPU& cpu, const OpcodeInfo& info) {
    alu_sub_base(cpu, cpu.readBytePC(), false, false);
    return info.cycles[0];
}
int SUB_A_memHL_impl(CPU& cpu, const OpcodeInfo& info) {
    alu_sub_base(cpu, cpu.readMemory(cpu.getHL()), false, false);
    return info.cycles[0];
}
int SBC_A_reg_impl(CPU& cpu, const OpcodeInfo& info) {
    alu_sub_base(cpu, get_reg_ref(cpu, info.operand2), true, false);
    return info.cycles[0];
}
int SBC_A_n8_impl(CPU& cpu, const OpcodeInfo& info) {
    alu_sub_base(cpu, cpu.readBytePC(), true, false);
    return info.cycles[0];
}
int SBC_A_memHL_impl(CPU& cpu, const OpcodeInfo& info) {
    alu_sub_base(cpu, cpu.readMemory(cpu.getHL()), true, false);
    return info.cycles[0];
}

int AND_A_reg_impl(CPU& cpu, const OpcodeInfo& info) {
    cpu.getA() &= get_reg_ref(cpu, info.operand2);
    cpu.setFlagZ(cpu.getA() == 0);
    cpu.setFlagN(false);
    cpu.setFlagH(true);
    cpu.setFlagC(false);
    return info.cycles[0];
}
int AND_A_n8_impl(CPU& cpu, const OpcodeInfo& info) {
    cpu.getA() &= cpu.readBytePC();
    cpu.setFlagZ(cpu.getA() == 0);
    cpu.setFlagN(false);
    cpu.setFlagH(true);
    cpu.setFlagC(false);
    return info.cycles[0];
}
int AND_A_memHL_impl(CPU& cpu, const OpcodeInfo& info) {
    cpu.getA() &= cpu.readMemory(cpu.getHL());
    cpu.setFlagZ(cpu.getA() == 0);
    cpu.setFlagN(false);
    cpu.setFlagH(true);
    cpu.setFlagC(false);
    return info.cycles[0];
}

int XOR_A_reg_impl(CPU& cpu, const OpcodeInfo& info) {
    cpu.getA() ^= get_reg_ref(cpu, info.operand2);
    cpu.setFlagZ(cpu.getA() == 0);
    cpu.setFlagN(false);
    cpu.setFlagH(false);
    cpu.setFlagC(false);
    return info.cycles[0];
}
int XOR_A_n8_impl(CPU& cpu, const OpcodeInfo& info) {
    cpu.getA() ^= cpu.readBytePC();
    cpu.setFlagZ(cpu.getA() == 0);
    cpu.setFlagN(false);
    cpu.setFlagH(false);
    cpu.setFlagC(false);
    return info.cycles[0];
}
int XOR_A_memHL_impl(CPU& cpu, const OpcodeInfo& info) {
    cpu.getA() ^= cpu.readMemory(cpu.getHL());
    cpu.setFlagZ(cpu.getA() == 0);
    cpu.setFlagN(false);
    cpu.setFlagH(false);
    cpu.setFlagC(false);
    return info.cycles[0];
}

int OR_A_reg_impl(CPU& cpu, const OpcodeInfo& info) {
    cpu.getA() |= get_reg_ref(cpu, info.operand2);
    cpu.setFlagZ(cpu.getA() == 0);
    cpu.setFlagN(false);
    cpu.setFlagH(false);
    cpu.setFlagC(false);
    return info.cycles[0];
}
int OR_A_n8_impl(CPU& cpu, const OpcodeInfo& info) {
    cpu.getA() |= cpu.readBytePC();
    cpu.setFlagZ(cpu.getA() == 0);
    cpu.setFlagN(false);
    cpu.setFlagH(false);
    cpu.setFlagC(false);
    return info.cycles[0];
}
int OR_A_memHL_impl(CPU& cpu, const OpcodeInfo& info) {
    cpu.getA() |= cpu.readMemory(cpu.getHL());
    cpu.setFlagZ(cpu.getA() == 0);
    cpu.setFlagN(false);
    cpu.setFlagH(false);
    cpu.setFlagC(false);
    return info.cycles[0];
}

int CP_A_reg_impl(CPU& cpu, const OpcodeInfo& info) {
    alu_sub_base(cpu, get_reg_ref(cpu, info.operand2), false, true); // true for is_cp
    return info.cycles[0];
}
int CP_A_n8_impl(CPU& cpu, const OpcodeInfo& info) {
    alu_sub_base(cpu, cpu.readBytePC(), false, true);
    return info.cycles[0];
}
int CP_A_memHL_impl(CPU& cpu, const OpcodeInfo& info) {
    alu_sub_base(cpu, cpu.readMemory(cpu.getHL()), false, true);
    return info.cycles[0];
}

// Helper for 8-bit INC
void alu_inc8(CPU& cpu, BYTE& reg) {
    BYTE original_val = reg;
    reg++;
    cpu.setFlagZ(reg == 0);
    cpu.setFlagN(false);
    cpu.setFlagH((original_val & 0xF) == 0xF); // Half carry if LSN was 0xF
    // C flag is not affected
}
int INC_reg_impl(CPU& cpu, const OpcodeInfo& info) {
    alu_inc8(cpu, get_reg_ref(cpu, info.operand1));
    return info.cycles[0];
}
int INC_memHL_impl(CPU& cpu, const OpcodeInfo& info) {
    BYTE val = cpu.readMemory(cpu.getHL());
    BYTE original_val = val;
    val++;
    cpu.writeMemory(cpu.getHL(), val);
    cpu.setFlagZ(val == 0);
    cpu.setFlagN(false);
    cpu.setFlagH((original_val & 0xF) == 0xF);
    // C flag is not affected
    return info.cycles[0];
}

// Helper for 8-bit DEC
void alu_dec8(CPU& cpu, BYTE& reg) {
    BYTE original_val = reg;
    reg--;
    cpu.setFlagZ(reg == 0);
    cpu.setFlagN(true);
    cpu.setFlagH((original_val & 0xF) == 0x0); // Half borrow if LSN was 0x0
    // C flag is not affected
}
int DEC_reg_impl(CPU& cpu, const OpcodeInfo& info) {
    alu_dec8(cpu, get_reg_ref(cpu, info.operand1));
    return info.cycles[0];
}
int DEC_memHL_impl(CPU& cpu, const OpcodeInfo& info) {
    BYTE val = cpu.readMemory(cpu.getHL());
    BYTE original_val = val;
    val--;
    cpu.writeMemory(cpu.getHL(), val);
    cpu.setFlagZ(val == 0);
    cpu.setFlagN(true);
    cpu.setFlagH((original_val & 0xF) == 0x0);
    // C flag is not affected
    return info.cycles[0];
}


// --- 16-bit ALU Instructions ---
int ADD_HL_rr_impl(CPU& cpu, const OpcodeInfo& info) {
    // ADD HL, rr (rr = BC, DE, HL, SP)
    // Flags: Z N H C
    //        - 0 H C
    WORD hl_val = cpu.getHL();
    WORD rr_val = get_reg_pair_val(cpu, info.operand2);
    WORD result = static_cast<WORD>(hl_val) + rr_val;

    cpu.setFlagN(false);
    // Half Carry: Carry from bit 11 to bit 12
    cpu.setFlagH(((hl_val & 0xFFF) + (rr_val & 0xFFF)) > 0xFFF);
    // Carry: Carry from bit 15
    cpu.setFlagC(result > 0xFFFF);
    cpu.setHL(static_cast<WORD>(result & 0xFFFF));
    // Z flag is not affected by 16-bit ADD
    return info.cycles[0];
}

int ADD_SP_e8_impl(CPU& cpu, const OpcodeInfo& info) {
    // ADD SP, e8 (e8 is signed immediate)
    // Flags: Z N H C
    //        0 0 H C (H and C are from LSB of SP + e8)
    BYTE e8_unsigned = cpu.readBytePC();
    signed char e8_signed = static_cast<signed char>(e8_unsigned);
    WORD sp_val = cpu.getSP();
    WORD result = sp_val + e8_signed;

    // Calculate flags based on LSB of SP and e8 for H and C
    BYTE sp_lo = sp_val & 0xFF;
    unsigned int temp_calc = sp_lo + e8_unsigned;

    cpu.setFlagZ(false);
    cpu.setFlagN(false);
    cpu.setFlagH(((sp_lo & 0xF) + (e8_unsigned & 0xF)) > 0xF);
    cpu.setFlagC(temp_calc > 0xFF);

    cpu.setSP(result);
    return info.cycles[0];
}


int INC_rr_impl(CPU& cpu, const OpcodeInfo& info) {
    // INC rr (rr = BC, DE, HL, SP)
    // Flags: - - - - (No flags affected for 16-bit INC/DEC)
    WORD val = get_reg_pair_val(cpu, info.operand1);
    val++;
    set_reg_pair_val(cpu, info.operand1, val);
    // Add 4 extra cycles for 16-bit INC/DEC
    return info.cycles[0]; // 8 cycles
}

int DEC_rr_impl(CPU& cpu, const OpcodeInfo& info) {
    // DEC rr (rr = BC, DE, HL, SP)
    // Flags: - - - -
    WORD val = get_reg_pair_val(cpu, info.operand1);
    val--;
    set_reg_pair_val(cpu, info.operand1, val);
    // Add 4 extra cycles for 16-bit INC/DEC
    return info.cycles[0]; // 8 cycles
}

// --- Rotate and Shift Instructions (Non-CB) ---
int RLCA_impl(CPU& cpu, const OpcodeInfo& info) {
    // RLCA: Rotate A left. Bit 7 to Carry and to Bit 0.
    // Flags: Z N H C
    //        0 0 0 C
    BYTE val_a = cpu.getA();
    bool carry = (val_a & 0x80) != 0; // Bit 7
    val_a <<= 1;
    if (carry) val_a |= 0x01;

    cpu.getA() = val_a;
    cpu.setFlagZ(false); // Z is always 0
    cpu.setFlagN(false);
    cpu.setFlagH(false);
    cpu.setFlagC(carry);
    return info.cycles[0];
}

int RLA_impl(CPU& cpu, const OpcodeInfo& info) {
    // RLA: Rotate A left through Carry.
    // Flags: Z N H C
    //        0 0 0 C
    BYTE val_a = cpu.getA();
    bool old_carry = cpu.getFlagC();
    bool new_carry = (val_a & 0x80) != 0; // Bit 7
    val_a <<= 1;
    if (old_carry) val_a |= 0x01;

    cpu.getA() = val_a;
    cpu.setFlagZ(false); // Z is always 0
    cpu.setFlagN(false);
    cpu.setFlagH(false);
    cpu.setFlagC(new_carry);
    return info.cycles[0];
}

int RRCA_impl(CPU& cpu, const OpcodeInfo& info) {
    // RRCA: Rotate A right. Bit 0 to Carry and to Bit 7.
    // Flags: Z N H C
    //        0 0 0 C
    BYTE val_a = cpu.getA();
    bool carry = (val_a & 0x01) != 0; // Bit 0
    val_a >>= 1;
    if (carry) val_a |= 0x80;

    cpu.getA() = val_a;
    cpu.setFlagZ(false); // Z is always 0
    cpu.setFlagN(false);
    cpu.setFlagH(false);
    cpu.setFlagC(carry);
    return info.cycles[0];
}

int RRA_impl(CPU& cpu, const OpcodeInfo& info) {
    // RRA: Rotate A right through Carry.
    // Flags: Z N H C
    //        0 0 0 C
    BYTE val_a = cpu.getA();
    bool old_carry = cpu.getFlagC();
    bool new_carry = (val_a & 0x01) != 0; // Bit 0
    val_a >>= 1;
    if (old_carry) val_a |= 0x80;

    cpu.getA() = val_a;
    cpu.setFlagZ(false); // Z is always 0
    cpu.setFlagN(false);
    cpu.setFlagH(false);
    cpu.setFlagC(new_carry);
    return info.cycles[0];
}

// --- CB-Prefixed Instructions ---
// Helper for RLC r / RLC (HL)
BYTE rlc_op(CPU& cpu, BYTE val) {
    bool carry = (val & 0x80) != 0;
    val <<= 1;
    if (carry) val |= 0x01;
    cpu.setFlagZ(val == 0);
    cpu.setFlagN(false);
    cpu.setFlagH(false);
    cpu.setFlagC(carry);
    return val;
}
int RLC_reg_impl(CPU& cpu, const OpcodeInfo& info) {
    BYTE& reg = get_reg_ref(cpu, info.operand1);
    reg = rlc_op(cpu, reg);
    return info.cycles[0];
}
int RLC_memHL_impl(CPU& cpu, const OpcodeInfo& info) {
    BYTE val = cpu.readMemory(cpu.getHL());
    val = rlc_op(cpu, val);
    cpu.writeMemory(cpu.getHL(), val);
    return info.cycles[0];
}

// Helper for RRC r / RRC (HL)
BYTE rrc_op(CPU& cpu, BYTE val) {
    bool carry = (val & 0x01) != 0;
    val >>= 1;
    if (carry) val |= 0x80;
    cpu.setFlagZ(val == 0);
    cpu.setFlagN(false);
    cpu.setFlagH(false);
    cpu.setFlagC(carry);
    return val;
}
int RRC_reg_impl(CPU& cpu, const OpcodeInfo& info) {
    BYTE& reg = get_reg_ref(cpu, info.operand1);
    reg = rrc_op(cpu, reg);
    return info.cycles[0];
}
int RRC_memHL_impl(CPU& cpu, const OpcodeInfo& info) {
    BYTE val = cpu.readMemory(cpu.getHL());
    val = rrc_op(cpu, val);
    cpu.writeMemory(cpu.getHL(), val);
    return info.cycles[0];
}


int RL_reg_impl(CPU& cpu, const OpcodeInfo& info) {
    BYTE& reg = get_reg_ref(cpu, info.operand1);
    bool old_carry = cpu.getFlagC();
    bool new_carry = (reg & 0x80) != 0;
    reg <<= 1;
    if(old_carry) reg |= 0x01;
    cpu.setFlagZ(reg == 0);
    cpu.setFlagN(false);
    cpu.setFlagH(false);
    cpu.setFlagC(new_carry);
    return info.cycles[0];
}
int RL_memHL_impl(CPU& cpu, const OpcodeInfo& info) {
    BYTE val = cpu.readMemory(cpu.getHL());
    bool old_carry = cpu.getFlagC();
    bool new_carry = (val & 0x80) != 0;
    val <<= 1;
    if(old_carry) val |= 0x01;
    cpu.writeMemory(cpu.getHL(), val);
    cpu.setFlagZ(val == 0);
    cpu.setFlagN(false);
    cpu.setFlagH(false);
    cpu.setFlagC(new_carry);
    return info.cycles[0];
}

int RR_reg_impl(CPU& cpu, const OpcodeInfo& info) {
    BYTE& reg = get_reg_ref(cpu, info.operand1);
    bool old_carry = cpu.getFlagC();
    bool new_carry = (reg & 0x01) != 0;
    reg >>= 1;
    if(old_carry) reg |= 0x80;
    cpu.setFlagZ(reg == 0);
    cpu.setFlagN(false);
    cpu.setFlagH(false);
    cpu.setFlagC(new_carry);
    return info.cycles[0];
}
int RR_memHL_impl(CPU& cpu, const OpcodeInfo& info) {
    BYTE val = cpu.readMemory(cpu.getHL());
    bool old_carry = cpu.getFlagC();
    bool new_carry = (val & 0x01) != 0;
    val >>= 1;
    if(old_carry) val |= 0x80;
    cpu.writeMemory(cpu.getHL(), val);
    cpu.setFlagZ(val == 0);
    cpu.setFlagN(false);
    cpu.setFlagH(false);
    cpu.setFlagC(new_carry);
    return info.cycles[0];
}

int SLA_reg_impl(CPU& cpu, const OpcodeInfo& info) {
    BYTE& reg = get_reg_ref(cpu, info.operand1);
    bool carry = (reg & 0x80) != 0;
    reg <<= 1;
    cpu.setFlagZ(reg == 0);
    cpu.setFlagN(false);
    cpu.setFlagH(false);
    cpu.setFlagC(carry);
    return info.cycles[0];
}
int SLA_memHL_impl(CPU& cpu, const OpcodeInfo& info) {
    BYTE val = cpu.readMemory(cpu.getHL());
    bool carry = (val & 0x80) != 0;
    val <<= 1;
    cpu.writeMemory(cpu.getHL(), val);
    cpu.setFlagZ(val == 0);
    cpu.setFlagN(false);
    cpu.setFlagH(false);
    cpu.setFlagC(carry);
    return info.cycles[0];
}

int SRA_reg_impl(CPU& cpu, const OpcodeInfo& info) {
    BYTE& reg = get_reg_ref(cpu, info.operand1);
    bool carry = (reg & 0x01) != 0;
    BYTE msb = reg & 0x80; // Preserve MSB
    reg >>= 1;
    reg |= msb; // Restore MSB
    cpu.setFlagZ(reg == 0);
    cpu.setFlagN(false);
    cpu.setFlagH(false);
    cpu.setFlagC(carry);
    return info.cycles[0];
}
int SRA_memHL_impl(CPU& cpu, const OpcodeInfo& info) {
    BYTE val = cpu.readMemory(cpu.getHL());
    bool carry = (val & 0x01) != 0;
    BYTE msb = val & 0x80;
    val >>= 1;
    val |= msb;
    cpu.writeMemory(cpu.getHL(), val);
    cpu.setFlagZ(val == 0);
    cpu.setFlagN(false);
    cpu.setFlagH(false);
    cpu.setFlagC(carry);
    return info.cycles[0];
}

int SWAP_reg_impl(CPU& cpu, const OpcodeInfo& info) {
    BYTE& reg = get_reg_ref(cpu, info.operand1);
    BYTE temp = (reg >> 4) | (reg << 4);
    reg = temp;
    cpu.setFlagZ(reg == 0);
    cpu.setFlagN(false);
    cpu.setFlagH(false);
    cpu.setFlagC(false);
    return info.cycles[0];
}
int SWAP_memHL_impl(CPU& cpu, const OpcodeInfo& info) {
    BYTE val = cpu.readMemory(cpu.getHL());
    BYTE temp = (val >> 4) | (val << 4);
    cpu.writeMemory(cpu.getHL(), temp);
    cpu.setFlagZ(temp == 0);
    cpu.setFlagN(false);
    cpu.setFlagH(false);
    cpu.setFlagC(false);
    return info.cycles[0];
}

int SRL_reg_impl(CPU& cpu, const OpcodeInfo& info) {
    BYTE& reg = get_reg_ref(cpu, info.operand1);
    bool carry = (reg & 0x01) != 0;
    reg >>= 1;
    cpu.setFlagZ(reg == 0);
    cpu.setFlagN(false);
    cpu.setFlagH(false);
    cpu.setFlagC(carry);
    return info.cycles[0];
}
int SRL_memHL_impl(CPU& cpu, const OpcodeInfo& info) {
    BYTE val = cpu.readMemory(cpu.getHL());
    bool carry = (val & 0x01) != 0;
    val >>= 1;
    cpu.writeMemory(cpu.getHL(), val);
    cpu.setFlagZ(val == 0);
    cpu.setFlagN(false);
    cpu.setFlagH(false);
    cpu.setFlagC(carry);
    return info.cycles[0];
}

int BIT_b_reg_impl(CPU& cpu, const OpcodeInfo& info) {
    BYTE bit_to_test = info.extraData; // Bit number stored in extraData
    BYTE& reg_val = get_reg_ref(cpu, info.operand1);
    cpu.setFlagZ(!((reg_val >> bit_to_test) & 0x01));
    cpu.setFlagN(false);
    cpu.setFlagH(true);
    // C flag is not affected
    return info.cycles[0];
}

int BIT_b_memHL_impl(CPU& cpu, const OpcodeInfo& info) {
    BYTE bit_to_test = info.extraData;
    BYTE mem_val = cpu.readMemory(cpu.getHL());
    cpu.setFlagZ(!((mem_val >> bit_to_test) & 0x01));
    cpu.setFlagN(false);
    cpu.setFlagH(true);
    return info.cycles[0];
}

int RES_b_reg_impl(CPU& cpu, const OpcodeInfo& info) {
    BYTE bit_to_reset = info.extraData;
    BYTE& reg_val = get_reg_ref(cpu, info.operand1);
    reg_val &= ~(1 << bit_to_reset);
    // No flags affected by RES
    return info.cycles[0];
}
int RES_b_memHL_impl(CPU& cpu, const OpcodeInfo& info) {
    BYTE bit_to_reset = info.extraData;
    BYTE mem_val = cpu.readMemory(cpu.getHL());
    mem_val &= ~(1 << bit_to_reset);
    cpu.writeMemory(cpu.getHL(), mem_val);
    return info.cycles[0];
}

int SET_b_reg_impl(CPU& cpu, const OpcodeInfo& info) {
    BYTE bit_to_set = info.extraData;
    BYTE& reg_val = get_reg_ref(cpu, info.operand1);
    reg_val |= (1 << bit_to_set);
    // No flags affected by SET
    return info.cycles[0];
}
int SET_b_memHL_impl(CPU& cpu, const OpcodeInfo& info) {
    BYTE bit_to_set = info.extraData;
    BYTE mem_val = cpu.readMemory(cpu.getHL());
    mem_val |= (1 << bit_to_set);
    cpu.writeMemory(cpu.getHL(), mem_val);
    return info.cycles[0];
}


// --- Control/Branch Instructions ---
int JP_n16_impl(CPU& cpu, const OpcodeInfo& info) {
    // JP a16 : Jump to address a16.
    // Flags: - - - -
    cpu.setPC(cpu.readWordPC());
    // Add extra cycle for jump
    return info.cycles[0]; // 16 cycles
}

int JP_cc_n16_impl(CPU& cpu, const OpcodeInfo& info) {
    // JP cc, a16 : Conditional jump to a16.
    // Flags: - - - -
    WORD new_pc = cpu.readWordPC();
    bool condition_met = false;
    switch (info.condition) {
        case ConditionType::NZ: condition_met = !cpu.getFlagZ(); break;
        case ConditionType::Z:  condition_met = cpu.getFlagZ(); break;
        case ConditionType::NC: condition_met = !cpu.getFlagC(); break;
        case ConditionType::C:  condition_met = cpu.getFlagC(); break;
        default: break; // Should not happen
    }

    if (condition_met) {
        cpu.setPC(new_pc);
        return info.cycles[0]; // e.g., 16 cycles if jump taken
    }
    return info.cycles[1]; // e.g., 12 cycles if not taken
}

int JP_HL_impl(CPU& cpu, const OpcodeInfo& info) {
    // JP HL : Jump to address in HL. (Formerly JP (HL))
    // Flags: - - - -
    cpu.setPC(cpu.getHL());
    return info.cycles[0]; // Typically 4 cycles
}

int JR_e8_impl(CPU& cpu, const OpcodeInfo& info) {
    // JR e8 : Relative jump by signed e8.
    // Flags: - - - -
    signed char offset = static_cast<signed char>(cpu.readBytePC());
    cpu.setPC(cpu.getPC() + offset);
    // Add extra cycle for jump
    return info.cycles[0]; // 12 cycles
}

int JR_cc_e8_impl(CPU& cpu, const OpcodeInfo& info) {
    // JR cc, e8 : Conditional relative jump.
    // Flags: - - - -
    signed char offset = static_cast<signed char>(cpu.readBytePC());
    bool condition_met = false;
    switch (info.condition) {
        case ConditionType::NZ: condition_met = !cpu.getFlagZ(); break;
        case ConditionType::Z:  condition_met = cpu.getFlagZ(); break;
        case ConditionType::NC: condition_met = !cpu.getFlagC(); break;
        case ConditionType::C:  condition_met = cpu.getFlagC(); break;
        default: break;
    }
    if (condition_met) {
        cpu.setPC(cpu.getPC() + offset);
        return info.cycles[0]; // 12 cycles if jump
    }
    return info.cycles[1]; // 8 cycles if no jump
}

int CALL_n16_impl(CPU& cpu, const OpcodeInfo& info) {
    // CALL a16 : Call subroutine at address a16.
    // Flags: - - - -
    WORD call_addr = cpu.readWordPC();
    cpu.pushStackWord(cpu.getPC()); // PC is already past the CALL instruction + operands
    cpu.setPC(call_addr);
    // Add extra cycles for call
    return info.cycles[0]; // 24 cycles
}

int CALL_cc_n16_impl(CPU& cpu, const OpcodeInfo& info) {
    // CALL cc, a16 : Conditional call.
    // Flags: - - - -
    WORD call_addr = cpu.readWordPC();
    bool condition_met = false;
     switch (info.condition) {
        case ConditionType::NZ: condition_met = !cpu.getFlagZ(); break;
        case ConditionType::Z:  condition_met = cpu.getFlagZ(); break;
        case ConditionType::NC: condition_met = !cpu.getFlagC(); break;
        case ConditionType::C:  condition_met = cpu.getFlagC(); break;
        default: break;
    }
    if (condition_met) {
        cpu.pushStackWord(cpu.getPC());
        cpu.setPC(call_addr);
        return info.cycles[0]; // 24 cycles if call
    }
    return info.cycles[1]; // 12 cycles if no call
}

int RET_impl(CPU& cpu, const OpcodeInfo& info) {
    // RET : Return from subroutine.
    // Flags: - - - -
    cpu.setPC(cpu.popStackWord());
    // Add extra cycles for return
    return info.cycles[0]; // 16 cycles
}

int RET_cc_impl(CPU& cpu, const OpcodeInfo& info) {
    // RET cc : Conditional return.
    // Flags: - - - -
    bool condition_met = false;
    switch (info.condition) {
        case ConditionType::NZ: condition_met = !cpu.getFlagZ(); break;
        case ConditionType::Z:  condition_met = cpu.getFlagZ(); break;
        case ConditionType::NC: condition_met = !cpu.getFlagC(); break;
        case ConditionType::C:  condition_met = cpu.getFlagC(); break;
        default: break;
    }
    // Conditional RET takes extra cycles *only if condition is met*
    if (condition_met) {
        cpu.setPC(cpu.popStackWord());
        return info.cycles[0]; // 20 cycles if return
    }
    return info.cycles[1]; // 8 cycles if no return
}

int RETI_impl(CPU& cpu, const OpcodeInfo& info) {
    // RETI : Return from interrupt and enable interrupts.
    // Flags: - - - -
    cpu.setPC(cpu.popStackWord());
    cpu.enableInterrupts(); // IME = 1
    // Add extra cycles for return
    return info.cycles[0]; // 16 cycles
}

int RST_impl(CPU& cpu, const OpcodeInfo& info) {
    // RST n : Call subroutine at address 0x0000 + n.
    // n is stored in info.extraData (0x00, 0x08, 0x10, ..., 0x38)
    // Flags: - - - -
    cpu.pushStackWord(cpu.getPC());
    cpu.setPC(info.extraData);
    // Add extra cycles for call
    return info.cycles[0]; // 16 cycles
}


// --- Miscellaneous Instructions ---
int DAA_impl(CPU& cpu, const OpcodeInfo& info) {
    // DAA: Decimal Adjust Accumulator.
    // Flags: Z N H C
    //        Z - 0 C
    BYTE& a = cpu.getA();
    int correction = 0; // Use int to handle potential intermediate values > 0xFF temporarily
    bool set_c_flag = false; // Flag to track if C should be set by DAA

    // Capture flag state *before* modification
    bool flagN_before = cpu.getFlagN();
    bool flagH_before = cpu.getFlagH();
    bool flagC_before = cpu.getFlagC();

    if (!flagN_before) { // After addition
        if (flagC_before || a > 0x99) {
            correction = 0x60;
            set_c_flag = true; // DAA sets C flag if correction >= 0x60 needed
        }
        if (flagH_before || (a & 0x0F) > 0x09) {
            correction += 0x06;
        }
        a += static_cast<BYTE>(correction);
    } else { // After subtraction
        if (flagC_before) {
            correction = 0x60;
            // C flag is NOT set by DAA on subtraction, it's kept from previous op
            // So, set_c_flag remains false here.
        }
        if (flagH_before) {
            correction += 0x06;
        }
        a -= static_cast<BYTE>(correction);
    }

    cpu.setFlagZ(a == 0);
    cpu.setFlagH(false); // H is always reset
    // N is not affected
    // C is set only if DAA determined it should during addition adjustment
    if(set_c_flag) {
        cpu.setFlagC(true);
    }
    // If it was a subtraction, C flag retains its value from the original SUB/SBC

    return info.cycles[0];
}


int CPL_impl(CPU& cpu, const OpcodeInfo& info) {
    // CPL: Complement Accumulator (A = ~A).
    // Flags: Z N H C
    //        - 1 1 -
    cpu.getA() = ~cpu.getA();
    cpu.setFlagN(true);
    cpu.setFlagH(true);
    // Z and C flags are not affected.
    return info.cycles[0];
}

int SCF_impl(CPU& cpu, const OpcodeInfo& info) {
    // SCF: Set Carry Flag.
    // Flags: Z N H C
    //        - 0 0 1
    cpu.setFlagN(false);
    cpu.setFlagH(false);
    cpu.setFlagC(true);
    // Z flag is not affected.
    return info.cycles[0];
}

int CCF_impl(CPU& cpu, const OpcodeInfo& info) {
    // CCF: Complement Carry Flag.
    // Flags: Z N H C
    //        - 0 0 C
    cpu.setFlagN(false);
    cpu.setFlagH(false);
    cpu.setFlagC(!cpu.getFlagC());
    // Z flag is not affected.
    return info.cycles[0];
}

} // namespace GB
