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
    LOG_INFO("Initial PC value: 0x" + std::to_string(m_ProgramCounter)); // Add this line
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
void CPU::pushStackInternal(WORD value) {
    m_StackPointer.reg -= 2;
    writeMemory(m_StackPointer.reg, value & 0xFF);
    writeMemory(m_StackPointer.reg + 1, (value >> 8) & 0xFF);
    std::stringstream ss_push;
    ss_push << "Pushed value to stack: 0x" << std::hex << std::setw(4) << std::setfill('0') << value;
    LOG_DEBUG(ss_push.str());
}

WORD CPU::popStackInternal() {
    BYTE lo = readMemory(m_StackPointer.reg);
    BYTE hi = readMemory(m_StackPointer.reg + 1);
    m_StackPointer.reg += 2;
    WORD poppedValue = (hi << 8) | lo;
    std::stringstream ss_pop;
    ss_pop << "Popped value from stack: 0x" << std::hex << std::setw(4) << std::setfill('0') << poppedValue;
    LOG_DEBUG(ss_pop.str());

    // Add logging if desired
    return poppedValue;
}

// --- Flag Management Implementations ---
void CPU::setFlags(BYTE new_f_value) {
    // The lower 4 bits of the F register are always zero on Game Boy
    m_RegisterAF.lo = new_f_value & 0xF0;
}

BYTE CPU::getFlags() const {
    return m_RegisterAF.lo;
}

bool CPU::getFlagZ() const {
    return (getFlags() & FLAG_Z_MASK) != 0;
}

bool CPU::getFlagN() const {
    return (getFlags() & FLAG_N_MASK) != 0;
}

bool CPU::getFlagH() const {
    return (getFlags() & FLAG_H_MASK) != 0;
}

bool CPU::getFlagC() const {
    return (getFlags() & FLAG_C_MASK) != 0;
}
int CPU::ExecuteExtendedOpcode() {
    BYTE extendedOpcode = readByte(); // Fetches the byte after CB, increments PC

    // Log the CB-prefixed instruction
    // PC at this point is after reading the extendedOpcode.
    // The address of the 0xCB instruction was (m_ProgramCounter - 2).
    // The address of the extendedOpcode itself was (m_ProgramCounter - 1).
    std::stringstream ss_log_cb;
    ss_log_cb << "PC: 0x" << std::hex << std::setw(4) << std::setfill('0') << (m_ProgramCounter - 2)
              << " OP: 0xCB " << std::setw(2) << std::setfill('0') << static_cast<int>(extendedOpcode)
              << " (" << CPUConstants::CB_OPCODE_TABLE[extendedOpcode].mnemonic << ")";
    LOG_DEBUG(ss_log_cb.str());
    const auto& entry = CPUConstants::CB_OPCODE_TABLE[extendedOpcode];

    // All CB opcodes should be of type BIT.
    if (entry.type != CPUConstants::InstructionType::BIT) {
        std::stringstream error_ss;
        error_ss << "CB Opcode 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(extendedOpcode)
                 << " is not of type BIT. Mnemonic: " << entry.mnemonic;
        LOG_ERROR(error_ss.str());
        // It's an unknown/malformed CB instruction.
        // m_ProgramCounter is already past this extendedOpcode.
        return handleUnknownOpcode(extendedOpcode); // Or a specific value for unknown CB.
    }

    auto& bitUnit = instructionUnits[static_cast<size_t>(CPUConstants::InstructionType::BIT)];
    if (bitUnit) {
        // The BitInstructions unit's execute method will handle the specific CB operation
        // and should return the correct number of cycles, likely by consulting CB_OPCODE_TABLE[extendedOpcode].duration_cycles.
        return bitUnit->execute(extendedOpcode);
    }

    std::stringstream error_ss_unit;
    error_ss_unit << "BitInstruction unit missing for CB opcode: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(extendedOpcode);
    LOG_ERROR(error_ss_unit.str());
    return handleUnknownOpcode(extendedOpcode); // Pass the extended opcode
}


int CPU::ExecuteOpcode(BYTE opcode) {
    // Logging for primary opcodes is done in ExecuteNextOpcode.
    // This function handles non-CB prefixed opcodes.

    const auto& entry = CPUConstants::FULL_OPCODE_TABLE[opcode];
    
    // Check for UNKNOWN type (which would include 0xED if not defined or defined as UNKNOWN)
    if (entry.type == CPUConstants::InstructionType::UNKNOWN) {
        return handleUnknownOpcode(opcode);
    }

    // Get the appropriate instruction unit
    auto& unit = instructionUnits[static_cast<size_t>(entry.type)];
    
    if (unit) {
        // The unit's execute method is responsible for:
        // 1. Performing the instruction's logic.
        // 2. Reading any additional operands using cpu.readByte() or cpu.readWord(), which advances PC.
        // 3. Returning the total cycles taken for this instruction,
        //    likely by using entry.duration_cycles and entry.duration_cycles_conditional.
        return unit->execute(opcode);
    }

    // This case should ideally not be reached if all instruction types have corresponding units initialized.
    std::stringstream error_ss_op_unit;
    error_ss_op_unit << "Instruction unit missing for type: " << static_cast<int>(entry.type)
                     << " for opcode: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(opcode);
    LOG_ERROR(error_ss_op_unit.str());
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
    pushStackInternal(m_ProgramCounter);
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

int CPU::ExecuteNextOpcode() {
    if (halted) {
        return 4; // HALT consumes 4 cycles per "do nothing"
    }
    if (pendingInterruptEnable) {
        interruptEnabled = true;
        pendingInterruptEnable = false;
        LOG_DEBUG("Interrupts enabled after instruction");
    }
    if (checkInterrupts()) {
        return 20;  // Interrupt handling consumes 20 cycles
    }

    BYTE opcode = readByte();
    logOpcode(opcode);

    if (opcode == 0xCB) {
        // CB prefix indicates an extended opcode
        LOG_INFO("Extended opcode detected: 0xCB");
        return ExecuteExtendedOpcode();
    } else {
        // Handle normal opcode
        LOG_INFO("Normal opcode detected: 0x" + std::to_string(opcode));
        return ExecuteOpcode(opcode);
    }
}
void CPU::logOpcode(BYTE opcode) {
    std::stringstream ss;
    // m_ProgramCounter was already incremented, so this shows where the instruction was fetched from
    WORD opcode_addr = m_ProgramCounter - 1;
    
    ss << "PC: 0x" << std::hex << std::setw(4) << std::setfill('0') << opcode_addr
       << " OP: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(opcode)
       << " (" << CPUConstants::getInstructionMnemonic(opcode) << ")"
       << " B=" << static_cast<int>(getB()) 
       << " D=" << static_cast<int>(getD())
       << " HL=0x" << std::hex << getHL_ref().reg
       << " A=" << static_cast<int>(getA());
    LOG_DEBUG(ss.str());
}

int CPU::handleUnknownOpcode(BYTE opcode) {
    std::stringstream ss;
    // (m_ProgramCounter - 1) is the address of the unknown 'opcode'
    // because m_ProgramCounter was incremented by the readByte() that fetched it.
    ss << "Unknown opcode: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(opcode)
       << " at PC: 0x" << std::hex << std::setw(4) << std::setfill('0') << (m_ProgramCounter - 1);
    LOG_ERROR(ss.str());

    // Return a default number of cycles for unknown opcodes.
    // This prevents the emulator from getting stuck if it continuously encounters unknown opcodes.
    // The PC has already been advanced by 1 (by the readByte that fetched this unknown opcode).
    // No further PC increment is done here, assuming unknown opcodes are treated as 1-byte long
    // as per the default OpcodeTableEntry constructor.
    return CPUConstants::UNKNOWN_OPCODE_CYCLES;
}

// Remove the old CPU_8BIT_LOAD method as it's now handled by LoadInstructions class