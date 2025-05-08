#include "cpu.h"
#include "logger.h"
#include "cpu_instructions/load_instructions.h"
#include "cpu_instructions/alu_instructions.h"
#include "cpu_instructions/jump_instructions.h"
#include "cpu_instructions/bit_instructions.h"
#include "cpu_instructions/control_instructions.h"
#include <sstream>
#include <iomanip>
#include <cpu_constants.h>

// Keep constructor
CPU::CPU(std::shared_ptr<MemoryController> memory)
    : memoryController(memory)
    , halted(false)
    , stopped(false)
    , interruptEnabled(false)
    , pendingInterruptEnable(false)
{
    initInstructionUnits();
    Reset();
    LOG_INFO("CPU initialized");
}
void CPU::initInstructionUnits() {
    // Initialize all instruction units
    instructionUnits[static_cast<size_t>(CPUConstants::InstructionType::ALU)] = 
        std::make_unique<ALUInstructions>(*this);
    instructionUnits[static_cast<size_t>(CPUConstants::InstructionType::LOAD)] = 
        std::make_unique<LoadInstructions>(*this);
    instructionUnits[static_cast<size_t>(CPUConstants::InstructionType::JUMP)] = 
        std::make_unique<JumpInstructions>(*this);
    instructionUnits[static_cast<size_t>(CPUConstants::InstructionType::BIT)] = 
        std::make_unique<BitInstructions>(*this);
    instructionUnits[static_cast<size_t>(CPUConstants::InstructionType::CONTROL)] = 
        std::make_unique<ControlInstructions>(*this);
}
// Keep Reset method
void CPU::Reset() {
    m_RegisterAF.reg = 0x01B0;
    m_RegisterBC.reg = 0x0013;
    m_RegisterDE.reg = 0x00D8;
    m_RegisterHL.reg = 0x014D;
    m_ProgramCounter = 0x0100;
    m_StackPointer.reg = 0xFFFE;
    
    halted = false;
    stopped = false;
    interruptEnabled = false;
    pendingInterruptEnable = false;
    
    LOG_INFO("CPU reset to initial state");
}

// Keep all memory access methods
BYTE CPU::readMemory(WORD address) const {
    BYTE value = memoryController->read(address);
    std::stringstream ss;
    ss << "CPU Memory Read - "
       << "Address: 0x" << std::hex << std::setw(4) << std::setfill('0') << address
       << " Value: 0x" << std::setw(2) << static_cast<int>(value);
    LOG_DEBUG(ss.str());
    return value;
}

void CPU::writeMemory(WORD address, BYTE data) {
    std::stringstream ss;
    ss << "CPU Memory Write - "
       << "Address: 0x" << std::hex << std::setw(4) << std::setfill('0') << address
       << " Data: 0x" << std::setw(2) << static_cast<int>(data);
    LOG_DEBUG(ss.str());
    memoryController->write(address, data);
}

BYTE CPU::readByte() {
    return readMemory(m_ProgramCounter++);
}

WORD CPU::readWord() {
    WORD word = readMemory(m_ProgramCounter) | (readMemory(m_ProgramCounter + 1) << 8);
    m_ProgramCounter += 2;
    return word;
}

// Keep stack operations
void CPU::pushStack(WORD value) {
    m_StackPointer.reg -= 2;
    writeMemory(m_StackPointer.reg + 1, (value >> 8) & BYTE_MASK);
    writeMemory(m_StackPointer.reg, value & BYTE_MASK);
}

WORD CPU::popStack() {
    WORD value = (readMemory(m_StackPointer.reg + 1) << 8) | readMemory(m_StackPointer.reg);
    m_StackPointer.reg += 2;
    return value;
}

int CPU::ExecuteNextOpcode() {
    if (checkInterrupts()) {
        return 20;  // Interrupt handling takes 20 cycles
    }

    if (halted) {
        return 4;   // HALT mode takes 4 cycles per step
    }

    BYTE opcode = readByte();
    return ExecuteOpcode(opcode);
}

int CPU::ExecuteExtendedOpcode() {
    BYTE opcode = readByte();
    auto& bitUnit = instructionUnits[static_cast<size_t>(CPUConstants::InstructionType::BIT)];
    if (bitUnit) {
        return bitUnit->execute(opcode);
    }
    return handleUnknownOpcode(opcode);
}

int CPU::ExecuteOpcode(BYTE opcode) {
    logOpcode(opcode);

    if (opcode == 0xCB) {
        return ExecuteExtendedOpcode();
    }

    // Get instruction type from opcode map
    auto instructionType = CPUConstants::getInstructionType(opcode);
    
    // Execute instruction if we have a valid type and unit
    if (instructionType != CPUConstants::InstructionType::UNKNOWN) {
        auto& unit = instructionUnits[static_cast<size_t>(instructionType)];
        if (unit) {
            return unit->execute(opcode);
        }
    }

    return handleUnknownOpcode(opcode);
}

// Keep all interrupt handling methods
void CPU::handleInterrupts(BYTE pendingInterrupts) {
    if (pendingInterrupts) {
        LOG_DEBUG("Pending interrupts: 0x" + std::to_string(pendingInterrupts));
    }
}

bool CPU::checkInterrupts() {
    if (!interruptEnabled) {
        LOG_DEBUG("Interrupts disabled, skipping check");
        return false;
    }

    BYTE pendingInterrupts = getPendingInterrupts();
    if (!pendingInterrupts) {
        return false;
    }

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

    for (const auto& handler : handlers) {
        if (pendingInterrupts & handler.bit) {
            (this->*handler.handler)();
            LOG_DEBUG("Handling interrupt with bit: 0x" + std::to_string(handler.bit));
            return true;
        }
    }
    return false;
}

// Keep all interrupt service routines
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

void CPU::serviceInterrupt(WORD address, BYTE interruptBit) {
    bool wasInterruptEnabled = interruptEnabled;
    interruptEnabled = false;
    pushStack(m_ProgramCounter);
    clearInterruptFlag(interruptBit);
    m_ProgramCounter = address;
    LOG_DEBUG("Servicing interrupt at: 0x" + std::to_string(address) + 
              " Previous interrupt state: " + (wasInterruptEnabled ? "enabled" : "disabled"));
}

// Keep helper methods
void CPU::clearInterruptFlag(BYTE bit) {
    BYTE flags = readMemory(IF_REGISTER);
    writeMemory(IF_REGISTER, flags & ~bit);
    LOG_DEBUG("Cleared interrupt flag: 0x" + std::to_string(bit));
}

BYTE CPU::getPendingInterrupts() const {
    BYTE flags = readMemory(IF_REGISTER);
    BYTE enable = readMemory(IE_REGISTER);
    return flags & enable;
}

void CPU::logOpcode(BYTE opcode) {
    std::stringstream ss;
    ss << "PC: 0x" << std::hex << std::setw(4) << std::setfill('0') << (m_ProgramCounter - 1) 
       << " OP: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(opcode)
       << " (" << CPUConstants::getInstructionDescription(opcode) << ")"; // Added description
    LOG_DEBUG(ss.str());
}

int CPU::handleUnknownOpcode(BYTE opcode) {
    std::stringstream ss;
    // Correctly format the opcode as a 2-digit hex number
    // Also, log the program counter (PC) where the unknown opcode was encountered
    ss << "Unknown opcode: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(opcode)
       << " at PC: 0x" << std::hex << std::setw(4) << std::setfill('0') << (m_ProgramCounter - 1); // m_ProgramCounter was already incremented by readByte()
    LOG_ERROR(ss.str());

    // Return a small number of cycles (e.g., 4, similar to NOP or HALT)
    // instead of 0. Returning 0 can cause the main emulation loop in
    // Emulator::update to get stuck if it continuously encounters unknown opcodes,
    // as cyclesThisUpdate would not progress, potentially leading to the
    // program counter incrementing indefinitely and causing a segmentation fault.
    return 4;
}

// Remove the old CPU_8BIT_LOAD method as it's now handled by LoadInstructions class