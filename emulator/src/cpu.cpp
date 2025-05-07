#include "cpu.h"
#include "logger.h"
#include <sstream>
#include <iomanip>
#include <common.h>
CPU::CPU(std::shared_ptr<MemoryController> memory)
    : memoryController(memory)
    , halted(false)
    , stopped(false)
{
    Reset();
    LOG_INFO("CPU initialized");
}

void CPU::Reset() {
    // Initialize registers with GameBoy boot values
    m_RegisterAF.reg = 0x01B0;
    m_RegisterBC.reg = 0x0013;
    m_RegisterDE.reg = 0x00D8;
    m_RegisterHL.reg = 0x014D;
    m_ProgramCounter = 0x0100;  // Start at ROM entry point
    m_StackPointer.reg = 0xFFFE;
    
    halted = false;
    stopped = false;
    
    LOG_INFO("CPU reset to initial state");
}

void CPU::pushStack(WORD value) {
    m_StackPointer.reg -= 2;
    writeMemory(m_StackPointer.reg + 1, (value >> 8) & 0xFF);
    writeMemory(m_StackPointer.reg, value & 0xFF);
}

WORD CPU::popStack() {
    WORD value = (readMemory(m_StackPointer.reg + 1) << 8) | readMemory(m_StackPointer.reg);
    m_StackPointer.reg += 2;
    return value;
}
BYTE CPU::readByte() {
    return readMemory(m_ProgramCounter++);
}
void CPU::handleInterrupts(BYTE pendingInterrupts)
{
    // Check if interrupts are enabled
    if (pendingInterrupts) {
        // Handle the pending interrupts here
        // For now, just log the pending interrupts
        LOG_DEBUG("Pending interrupts: 0x" + std::to_string(pendingInterrupts));
    }
}

WORD CPU::readWord() {
    WORD word = readMemory(m_ProgramCounter) | (readMemory(m_ProgramCounter + 1) << 8);
    m_ProgramCounter += 2;
    return word;
}


BYTE CPU::readMemory(WORD address) const {
    // Use memory controller to read the memory
    BYTE value = memoryController->read(address);

    // Create detailed log entry
    std::stringstream ss;
    ss << "CPU Memory Read - "
       << "Address: 0x" << std::hex << std::setw(4) << std::setfill('0') << address
       << " Value: 0x" << std::setw(2) << static_cast<int>(value);
    LOG_DEBUG(ss.str());

    return value;
}
void CPU::writeMemory(WORD address, BYTE data) {
    // Log before writing
    std::stringstream ss;
    ss << "CPU Memory Write - "
       << "Address: 0x" << std::hex << std::setw(4) << std::setfill('0') << address
       << " Data: 0x" << std::setw(2) << static_cast<int>(data);
    LOG_DEBUG(ss.str());

    // Delegate write to memory controller
    memoryController->write(address, data);
}



int CPU::ExecuteNextOpcode() {
    if (halted) {
        return 4;
    }
    if (pendingInterruptEnable) {
        interruptEnabled = true;
        pendingInterruptEnable = false;
        LOG_DEBUG("Interrupts enabled after instruction");
    }
    if (checkInterrupts()) {
        return 20;  // Interrupt handling cycles
    }

    BYTE opcode = readByte();
    logOpcode(opcode);

    // Group opcodes by type
    switch (opcode & 0xF0) {
        case 0x00: return executeControlOpcode(opcode);
        case 0x40: 
        case 0x50:
        case 0x60:
        case 0x70: return executeLoadOpcode(opcode);
        case 0x80:
        case 0x90: return executeALUOpcode(opcode);
        case 0xC0: return executeJumpOpcode(opcode);
        default:   return handleUnknownOpcode(opcode);
    }
}
bool CPU::checkInterrupts() {
    // Early return if interrupts are disabled
    if (!interruptEnabled) {
        LOG_DEBUG("Interrupts disabled, skipping check");
        return false;
    }

    BYTE pendingInterrupts = getPendingInterrupts();
    if (!pendingInterrupts) {
        return false;
    }

    // Check interrupts in priority order (bits 0-4)
    struct InterruptHandler {
        BYTE bit;
        WORD address;
        void (CPU::*handler)();
    };

    const InterruptHandler handlers[] = {
        {VBLANK_INTERRUPT_BIT, VBLANK_ISR_ADDR, &CPU::handleVBlankInterrupt},
        {LCD_INTERRUPT_BIT, LCD_ISR_ADDR, &CPU::handleLCDInterrupt},
        {TIMER_INTERRUPT_BIT, TIMER_ISR_ADDR, &CPU::handleTimerInterrupt},
        {JOYPAD_INTERRUPT_BIT, JOYPAD_ISR_ADDR, &CPU::handleJoypadInterrupt}
    };

    // Process highest priority interrupt first
    for (const auto& handler : handlers) {
        if (pendingInterrupts & handler.bit) {
            (this->*handler.handler)();
            LOG_DEBUG("Handling interrupt with bit: 0x" + 
                     std::to_string(handler.bit));
            return true;
        }
    }

    return false;
}
void CPU::handleVBlankInterrupt() {
    serviceInterrupt(VBLANK_ISR_ADDR, VBLANK_INTERRUPT_BIT);
}
void CPU::handleLCDInterrupt() {
    serviceInterrupt(LCD_ISR_ADDR, LCD_INTERRUPT_BIT);
}

void CPU::handleTimerInterrupt() {
    serviceInterrupt(TIMER_ISR_ADDR, TIMER_INTERRUPT_BIT);
}

void CPU::handleJoypadInterrupt() {
    serviceInterrupt(JOYPAD_ISR_ADDR, JOYPAD_INTERRUPT_BIT);
}

int CPU::executeLoadOpcode(BYTE opcode) {
    switch (opcode) {
        // Placeholder for load instructions
        default:
            return handleUnknownOpcode(opcode);
    }
}

int CPU::executeALUOpcode(BYTE opcode) {
    switch (opcode) {
        // Placeholder for arithmetic instructions
        default:
            return handleUnknownOpcode(opcode);
    }
}

int CPU::executeJumpOpcode(BYTE opcode) {
    switch (opcode) {
        // Placeholder for jump instructions
        default:
            return handleUnknownOpcode(opcode);
    }
}
int CPU::executeControlOpcode(BYTE opcode) {
    switch (opcode) {
        case 0x00: return 4;                    // NOP
        case 0x76: halted = true; return 4;     // HALT
        case 0xF3: interruptEnabled = false; return 4;  // DI
        case 0xFB:  // EI - Enable interrupts
            pendingInterruptEnable = true;  // Will be enabled after next instruction
            return 4;
        case 0xD9:  // RETI - Return from interrupt and enable interrupts
            m_ProgramCounter = popStack();
            interruptEnabled = true;  // Re-enable interrupts immediately
            LOG_DEBUG("RETI - Returning to 0x" + std::to_string(m_ProgramCounter));
            return 16;
        default: return handleUnknownOpcode(opcode);
    }
}
void CPU::serviceInterrupt(WORD address, BYTE interruptBit) {
    bool wasInterruptEnabled = interruptEnabled;
    
    interruptEnabled = false;  // Disable interrupts while handling
    pushStack(m_ProgramCounter);
    clearInterruptFlag(interruptBit);
    m_ProgramCounter = address;

    LOG_DEBUG("Servicing interrupt at: 0x" + std::to_string(address) + 
              " Previous interrupt state: " + (wasInterruptEnabled ? "enabled" : "disabled"));
}



void CPU::clearInterruptFlag(BYTE bit) {
    BYTE flags = readMemory(IF_REGISTER);
    writeMemory(IF_REGISTER, flags & ~bit);
    LOG_DEBUG("Cleared interrupt flag: 0x" + std::to_string(bit));

}

void CPU::logOpcode(BYTE opcode) {
    std::stringstream ss;
    ss << "PC: 0x" << std::hex << (m_ProgramCounter - 1) 
        << " OP: 0x" << static_cast<int>(opcode);
    LOG_DEBUG(ss.str());
}
int CPU::handleUnknownOpcode(BYTE opcode)
{
    LOG_ERROR("Unknown opcode: 0x" + std::to_string(opcode));
    // Handle unknown opcode (e.g., log, halt, etc.)
    return 0;
}
BYTE CPU::getPendingInterrupts() const
{
    BYTE flags = readMemory(IF_REGISTER);
    BYTE enable = readMemory(IE_REGISTER);
    return flags & enable;
}
