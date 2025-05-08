#include "cpu_instructions/cpu_instruction_base.h"
#include "../cpu.h"
#include <sstream>
#include <iomanip>

void CPUInstructionBase::setZeroFlag(bool set) {
    BYTE flags = cpu.getFlags();
    if (set) {
        flags |= CPU::FLAG_Z_MASK;
    } else {
        flags &= ~CPU::FLAG_Z_MASK;
    }
    cpu.setFlags(flags);
}

void CPUInstructionBase::setSubtractFlag(bool set) {
    BYTE flags = cpu.getFlags();
    if (set) {
        flags |= CPU::FLAG_N_MASK;
    } else {
        flags &= ~CPU::FLAG_N_MASK;
    }
    cpu.setFlags(flags);
}

void CPUInstructionBase::setHalfCarryFlag(bool set) {
    BYTE flags = cpu.getFlags();
    if (set) {
        flags |= CPU::FLAG_H_MASK;
    } else {
        flags &= ~CPU::FLAG_H_MASK;
    }
    cpu.setFlags(flags);
}

void CPUInstructionBase::setCarryFlag(bool set) {
    BYTE flags = cpu.getFlags();
    if (set) {
        flags |= CPU::FLAG_C_MASK;
    } else {
        flags &= ~CPU::FLAG_C_MASK;
    }
    cpu.setFlags(flags);
}

bool CPUInstructionBase::checkFlag(BYTE flag) const {
    // Directly use the CPU's getFlags() method and check against the provided flag
    return (cpu.getFlags() & flag) != 0;
}

void CPUInstructionBase::logUnhandledOpcode(BYTE opcode) {
    std::stringstream ss;
    ss << getClassName() << ": Unhandled opcode 0x" 
       << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(opcode);
    LOG_ERROR(ss.str());
}