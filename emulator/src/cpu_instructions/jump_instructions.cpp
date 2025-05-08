#include "cpu_instructions/jump_instructions.h"
#include "../cpu.h"
#include "../common.h"

namespace {
    // Jump condition flags
    constexpr BYTE FLAG_Z = 0x80;  // Zero Flag
    constexpr BYTE FLAG_C = 0x10;  // Carry Flag
    
    // Jump opcodes
    constexpr BYTE JP_NN = 0xC3;    // JP nn
    constexpr BYTE JP_NZ = 0xC2;    // JP NZ,nn
    constexpr BYTE JP_Z = 0xCA;     // JP Z,nn
    constexpr BYTE JP_NC = 0xD2;    // JP NC,nn
    constexpr BYTE JP_C = 0xDA;     // JP C,nn
    constexpr BYTE JP_HL = 0xE9;    // JP (HL)
    
    constexpr BYTE JR_N = 0x18;     // JR n
    constexpr BYTE JR_NZ = 0x20;    // JR NZ,n
    constexpr BYTE JR_Z = 0x28;     // JR Z,n
    constexpr BYTE JR_NC = 0x30;    // JR NC,n
    constexpr BYTE JR_C = 0x38;     // JR C,n
}

int JumpInstructions::execute(BYTE opcode) {
    switch(opcode) {
        case JP_NN: return CPU_JP(cpu.readWord());
        case JP_NZ: return CPU_JP_CC(cpu.readWord(), !checkFlag(FLAG_Z));
        case JP_Z:  return CPU_JP_CC(cpu.readWord(), checkFlag(FLAG_Z));
        case JP_NC: return CPU_JP_CC(cpu.readWord(), !checkFlag(FLAG_C));
        case JP_C:  return CPU_JP_CC(cpu.readWord(), checkFlag(FLAG_C));
        case JP_HL: return CPU_JP(cpu.getHL().reg);
        
        case JR_N:  return CPU_JR(cpu.readByte());
        case JR_NZ: return CPU_JR_CC(cpu.readByte(), !checkFlag(FLAG_Z));
        case JR_Z:  return CPU_JR_CC(cpu.readByte(), checkFlag(FLAG_Z));
        case JR_NC: return CPU_JR_CC(cpu.readByte(), !checkFlag(FLAG_C));
        case JR_C:  return CPU_JR_CC(cpu.readByte(), checkFlag(FLAG_C));
        case 0xC7: // RST 00H
            return CPU_RST(0x00);
        case 0xCF: // RST 08H
            return CPU_RST(0x08);
            
        case 0xD7: // RST 10H
            return CPU_RST(0x10);
            
        case 0xDF: // RST 18H
            return CPU_RST(0x18);
            
        case 0xE7: // RST 20H
            return CPU_RST(0x20);
            
        case 0xEF: // RST 28H
            return CPU_RST(0x28);
            
        case 0xF7: // RST 30H
            return CPU_RST(0x30);
            
        case 0xFF: // RST 38H
            return CPU_RST(0x38);
        case 0xCD: // CALL nn - Call unconditionally
            return CPU_CALL_nn();
        
        default: 
        // Log error for unhandled opcode within this unit
        // Or, if this should never happen, assert or throw
        logUnhandledOpcode(opcode);
        return cpu.handleUnknownOpcode(opcode); // Or return a fixed cycle count like 4
    }
}
int JumpInstructions::CPU_CALL_nn() {
    // Read the 16-bit immediate address
    WORD address = cpu.readWord();
    
    // Push current PC to stack
    cpu.pushToStack(cpu.getPC());
    
    // Set PC to the target address
    cpu.getPC() = address;
    
    // This instruction takes 24 cycles (6 M-cycles)
    return 24;
}

int JumpInstructions::CPU_JP(WORD address) {
    cpu.getPC() = address;
    return 16;  // 4 cycles
}

int JumpInstructions::CPU_JP_CC(WORD address, bool condition) {
    if (condition) {
        cpu.getPC() = address;
        return 16;  // Jump taken: 4 cycles
    }
    return 12;  // Jump not taken: 3 cycles
}

int JumpInstructions::CPU_JR(BYTE offset) {
    cpu.getPC() += static_cast<signed char>(offset);
    return 12;  // 3 cycles
}

int JumpInstructions::CPU_JR_CC(BYTE offset, bool condition) {
    if (condition) {
        cpu.getPC() += static_cast<signed char>(offset);
        return 12;  // Jump taken: 3 cycles
    }
    return 8;  // Jump not taken: 2 cycles
}

int JumpInstructions::CPU_CALL(WORD address) {
    cpu.pushToStack(cpu.getPC());
    cpu.getPC() = address;
    return 24;  // 6 cycles
}

int JumpInstructions::CPU_CALL_CC(WORD address, bool condition) {
    if (condition) {
        cpu.pushToStack(cpu.getPC());
        cpu.getPC() = address;
        return 24;  // Call taken: 6 cycles
    }
    return 12;  // Call not taken: 3 cycles
}

int JumpInstructions::CPU_RET() {
    cpu.getPC() = cpu.popFromStack();
    return 16;  // 4 cycles
}

int JumpInstructions::CPU_RET_CC(bool condition) {
    if (condition) {
        cpu.getPC() = cpu.popFromStack();
        return 20;  // Return taken: 5 cycles
    }
    return 8;  // Return not taken: 2 cycles
}

int JumpInstructions::CPU_RST(BYTE vector) {
    cpu.pushToStack(cpu.getPC());
    cpu.getPC() = vector;
    return 16;  // 4 cycles
}