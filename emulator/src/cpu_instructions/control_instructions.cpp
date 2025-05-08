#include "cpu_instructions/control_instructions.h"
#include "../cpu.h"
using namespace CPUConstants;

int ControlInstructions::execute(BYTE opcode) {
    switch(opcode) {
        case 0x00: return CPU_NOP();     // NOP
        case 0x76: return CPU_HALT();    // HALT
        case 0x10: return CPU_STOP();    // STOP
        case 0xF3: return CPU_DI();      // DI
        case 0xFB: return CPU_EI();      // EI
        default: 
        // Log error for unhandled opcode within this unit
        // Or, if this should never happen, assert or throw
        logUnhandledOpcode(opcode);
        return cpu.handleUnknownOpcode(opcode); // Or return a fixed cycle count like 4
    }
}

int ControlInstructions::CPU_NOP() {
    return 4;  // NOP takes 4 cycles
}

int ControlInstructions::CPU_HALT() {
    cpu.setHaltState(true);
    return 4;
}

int ControlInstructions::CPU_STOP() {
    cpu.setStopState(true);
    return 4;
}

int ControlInstructions::CPU_DI() {
    cpu.setInterruptState(false);
    return 4;
}

int ControlInstructions::CPU_EI() {
    cpu.setPendingInterruptEnable(true);  // Will be enabled after next instruction
    return 4;
}