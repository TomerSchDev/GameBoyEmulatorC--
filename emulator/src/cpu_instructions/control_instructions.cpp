#include "cpu_instructions/control_instructions.h"
#include "../cpu.h"
#include "../cpu_constants.h" // For FULL_OPCODE_TABLE
#include "../common.h"      // For logUnhandledOpcode (assuming)

// Note: using namespace CPUConstants; was in the original, ensure it's intended or qualify names.
// For clarity, I will qualify CPUConstants::FULL_OPCODE_TABLE

int ControlInstructions::execute(BYTE opcode) {
    switch(opcode) {
        case 0x00: // NOP
            CPU_NOP();
            break;
        case 0x10: // STOP
            // STOP 00 is a 2-byte instruction. The 0x00 is a dummy byte.
            // The CPU's readByte() in ExecuteNextOpcode would have fetched 0x10.
            // The instruction unit's execute (this function) is called.
            // If STOP needs to consume the next byte, the CPU's main loop or
            // this instruction needs to advance PC.
            // However, typically the length from OpcodeTableEntry handles PC advancement
            // by the main CPU loop *after* this function returns.
            // For STOP, the PC should be advanced by 1 more if it's truly 2 bytes
            // and the table length is 2.
            // If OpcodeTableEntry for 0x10 has length 2, CPU's main fetch logic
            // might read the 0x10, then this execute is called. The PC is then advanced by
            // table_entry.length AFTER this.
            // For STOP 00, the 00 byte is often ignored by emulators or handled as part of STOP.
            // Let's assume the OpcodeTableEntry for 0x10 correctly states its length.
            // If length is 1, then it's just STOP. If length is 2, the CPU's main loop
            // will fetch the next byte (the 00) as part of fetching operands,
            // which STOP doesn't explicitly use but is part of its encoding.
            // The CPU's PC will be advanced by getInstructionLength(opcode) after this.
            CPU_STOP(); // The 0x00 dummy byte is handled by PC increment based on instruction length
            break;
        case 0x76: // HALT
            CPU_HALT();
            break;
        case 0xF3: // DI
            CPU_DI();
            break;
        case 0xFB: // EI
            CPU_EI();
            break;
        case 0x3F: // CCF
            CPU_CCF();
            break;
        case 0x37: // SCF
            CPU_SCF();
            break;
        case 0x27: // DAA
            CPU_DAA();
            break;
        case 0x2F: // CPL
            CPU_CPL();
            break;
        default:
            logUnhandledOpcode(opcode); // Log error for unhandled opcode within this unit
            // Return cycles for unknown opcode from the main table, or let CPU handle it
            // If cpu.handleUnknownOpcode advances PC and returns cycles, that's fine.
            // Otherwise, return cycles from table for this specific unknown opcode.
            return CPUConstants::FULL_OPCODE_TABLE[opcode].duration_cycles; // Or a default like 4
    }
    return CPUConstants::FULL_OPCODE_TABLE[opcode].duration_cycles;
}
// Implementations:
void ControlInstructions::CPU_CCF() {
    BYTE f = cpu.getFlags();
    bool current_c = (f & CPU::FLAG_C_MASK) != 0;
    f &= ~(CPU::FLAG_N_MASK | CPU::FLAG_H_MASK); // Reset N and H
    if (!current_c) { // If C was 0, it becomes 1. If 1, it becomes 0.
        f |= CPU::FLAG_C_MASK;
    } else {
        f &= ~CPU::FLAG_C_MASK;
    }
    // Z flag is not affected by CCF directly, but by previous operations.
    // The problem statement says "Z - N 0 H 0 C C" for CCF, meaning Z is not changed by this instruction.
    // N is reset, H is reset, C is complemented.
    // My previous statement "Z is not affected" was slightly off. The Z flag is *not modified by this instruction*.
    // The flags should be: Z unchanged, N=0, H=0, C=~C
    // So, we need to preserve Z.
    BYTE current_z_flag = cpu.getFlags() & CPU::FLAG_Z_MASK;
    BYTE new_f = current_z_flag; // Start with current Z
    new_f &= ~(CPU::FLAG_N_MASK | CPU::FLAG_H_MASK); // Reset N and H

    if (!(cpu.getFlagC())) { // If current C is 0, set C
        new_f |= CPU::FLAG_C_MASK;
    } else { // If current C is 1, clear C (it's already clear in new_f if not set)
        // No action needed to clear C if it was 1, as new_f started with Z and N,H cleared.
    }
    // Correction: CCF inverts C. N=0, H=0. Z is not affected.
    // Let's re-evaluate CCF based on common sources:
    // CCF: Z N H C
    //      - 0 0 !C
    // Z is not affected. N is reset. H is reset. C is complemented.
    BYTE flags = cpu.getFlags();
    flags &= CPU::FLAG_Z_MASK; // Preserve Z
    // N and H are cleared
    if (cpu.getFlagC()) { // If C was set, clear it (it's already clear in flags)
        // flags &= ~CPU::FLAG_C_MASK; // This line is redundant if flags started with only Z
    } else {
        flags |= CPU::FLAG_C_MASK; // If C was clear, set it
    }
    cpu.setFlags(flags);
}

void ControlInstructions::CPU_SCF() {
    // SCF: Z N H C
    //      - 0 0 1
    // Z is not affected. N is reset. H is reset. C is set.
    BYTE flags = cpu.getFlags();
    flags &= CPU::FLAG_Z_MASK; // Preserve Z
    // N and H are cleared
    flags |= CPU::FLAG_C_MASK; // Set C
    cpu.setFlags(flags);
}

void ControlInstructions::CPU_CPL() {
    // CPL: Z N H C
    //      - 1 1 -
    // Z is not affected. N is set. H is set. C is not affected.
    cpu.getA() = ~cpu.getA();
    BYTE flags = cpu.getFlags();
    flags |= (CPU::FLAG_N_MASK | CPU::FLAG_H_MASK); // Set N and H
    // Z and C are not affected by CPL itself.
    cpu.setFlags(flags);
}

void ControlInstructions::CPU_DAA() {
    // DAA is complex. Flags: Z - 0 C (Z set if result is 0, H reset, C set or preserved)
    // This is a simplified DAA, refer to detailed Game Boy DAA logic.
    BYTE& a = cpu.getA();
    BYTE f = cpu.getFlags();
    BYTE correction = 0;
    bool set_c_flag = false;

    if (!cpu.getFlagN()) { // After addition
        if (cpu.getFlagC() || a > 0x99) {
            correction = 0x60;
            set_c_flag = true;
        }
        if (cpu.getFlagH() || (a & 0x0F) > 0x09) {
            correction |= 0x06;
        }
    } else { // After subtraction (N flag is set)
        if (cpu.getFlagC()) { // If C wass set from previous SUB/SBC
            correction = 0x60; // Subtract 0x60 in effect (add 0xA0 then handle carry, or direct logic)
                               // For DAA after subtraction, it's more like undoing BCD borrow.
                               // If C is set, it means a borrow occurred from the upper nibble.
            set_c_flag = true; // C remains set
        }
        if (cpu.getFlagH()) { // If H was set from previous SUB/SBC
            correction |= 0x06; // Subtract 0x06 in effect
        }
    }

    if (!cpu.getFlagN()) {
        a += correction;
    } else {
        a -= correction; // This is the tricky part for DAA after subtraction.
                         // Many sources say DAA should only be used after ADD/ADC.
                         // Game Boy DAA works for SUB/SBC too.
    }

    BYTE new_f = 0;
    if (a == 0) new_f |= CPU::FLAG_Z_MASK;
    // H flag is reset by DAA
    if (set_c_flag) new_f |= CPU::FLAG_C_MASK;
    else new_f |= (f & CPU::FLAG_C_MASK); // Preserve original C if not set by DAA

    if (cpu.getFlagN()) new_f |= CPU::FLAG_N_MASK; // N flag is preserved

    cpu.setFlags(new_f);
}

void ControlInstructions::CPU_NOP() {
    // No operation
}

void ControlInstructions::CPU_HALT() {
    // HALT behavior:
    // If interrupts are disabled (IME=0) AND no pending interrupts (IE & IF == 0),
    // HALT can behave like NOP or cause a HALT bug if the next instruction is EI.
    // For simplicity here, we just set the halted flag.
    // Proper HALT bug emulation is more complex.
    cpu.setHaltState(true);
}

void ControlInstructions::CPU_STOP() {
    // STOP mode is entered. Screen turns off.
    // Waits for a button press to resume.
    // Consumes the following 0x00 byte. PC should be advanced by 2 in total.
    // The CPU's main loop should advance PC by getInstructionLength(0x10)
    // which should be 2 for "STOP 00".
    cpu.setStopState(true);
    // The CPU's main fetch mechanism should have advanced PC by 1 for the 0x10.
    // If the instruction length for 0x10 is 2, the CPU will advance PC by 1 more
    // after this instruction's execution, effectively consuming the 0x00.
    // If your CPU's readByte() in ExecuteNextOpcode fetches the 0x10,
    // and then getInstructionLength(0x10) is 2, the PC will be advanced by 2
    // in total by the time the next instruction is fetched.
}

void ControlInstructions::CPU_DI() {
    cpu.setInterruptState(false);
    // DI takes effect immediately.
}

void ControlInstructions::CPU_EI() {
    // EI schedules interrupt master enable (IME) to be set after the instruction
    // *following* EI.
    cpu.setPendingInterruptEnable(true);
}