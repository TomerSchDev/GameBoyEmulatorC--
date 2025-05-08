#include "cpu_instructions/cpu_instruction_base.h"
#include "../cpu.h"
#include <sstream>
#include <iomanip>

void CPUInstructionBase::setZeroFlag(bool set) {
    if (set) 
        cpu.getAF().lo |= FLAG_MASK_Z;
    else 
        cpu.getAF().lo &= ~FLAG_MASK_Z;
}

void CPUInstructionBase::setSubtractFlag(bool set) {
    if (set) 
        cpu.getAF().lo |= FLAG_MASK_N;
    else 
        cpu.getAF().lo &= ~FLAG_MASK_N;
}

void CPUInstructionBase::setHalfCarryFlag(bool set) {
    if (set) 
        cpu.getAF().lo |= FLAG_MASK_H;
    else 
        cpu.getAF().lo &= ~FLAG_MASK_H;
}

void CPUInstructionBase::setCarryFlag(bool set) {
    if (set) 
        cpu.getAF().lo |= FLAG_MASK_C;
    else 
        cpu.getAF().lo &= ~FLAG_MASK_C;
}

bool CPUInstructionBase::checkFlag(BYTE flag) const {
    return (cpu.getAF().lo & flag) != 0;
}
void CPUInstructionBase::logUnhandledOpcode(BYTE opcode) {
    std::stringstream ss;
    ss << getClassName() << ": Unhandled opcode 0x" 
       << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(opcode);
    LOG_ERROR(ss.str());
}