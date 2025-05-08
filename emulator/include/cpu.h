#pragma once
#include <memory>
#include <array>
#include "memory_controller.h"
#include "logger.h"
#include "cpu_constants.h"
#include "cpu_instructions/cpu_instruction_base.h"

union Register {
    WORD reg;
    struct {
        BYTE lo;
        BYTE hi;
    };
};

class MemoryController;
class CPUInstructionBase;
class CPU {
private:
    // Memory Interface (initialized first)
    std::shared_ptr<MemoryController> memoryController;

    // CPU instruction units (initialized next)
    std::array<std::unique_ptr<CPUInstructionBase>, 
              static_cast<size_t>(CPUConstants::InstructionType::UNKNOWN)> instructionUnits;

    // CPU Registers
    Register m_RegisterAF;  // Accumulator & Flags
    Register m_RegisterBC;  // General Purpose
    Register m_RegisterDE;  // General Purpose
    Register m_RegisterHL;  // General Purpose/Memory Pointer
    WORD m_ProgramCounter; // Program Counter
    Register m_StackPointer; // Stack Pointer

    // CPU State (initialized last)
    bool halted;
    bool stopped;
    bool interruptEnabled;
    bool pendingInterruptEnable;

public:
    // Constructor and Destructor
    CPU(std::shared_ptr<MemoryController> memory);
    ~CPU() = default;

    // Memory Access
    BYTE readMemory(WORD address) const;
    void writeMemory(WORD address, BYTE data);
    BYTE readByte();
    WORD readWord();

    // Register Access (for instruction units)
    Register& getAF() { return m_RegisterAF; }
    Register& getBC() { return m_RegisterBC; }
    Register& getDE() { return m_RegisterDE; }
    Register& getHL() { return m_RegisterHL; }
    WORD& getPC() { return m_ProgramCounter; }
    Register& getSP() { return m_StackPointer; }

    // Main CPU Operations
    int ExecuteNextOpcode();
    void Reset();
    void handleInterrupts(BYTE pendingInterrupts);
    int handleUnknownOpcode(BYTE opcode);

    // State control methods
    void setHaltState(bool state) { halted = state; }
    void setStopState(bool state) { stopped = state; }
    void setInterruptState(bool state) { interruptEnabled = state; }
    void setPendingInterruptEnable(bool state) { pendingInterruptEnable = state; }
    bool isHalted() const { return halted; }
    bool isStopped() const { return stopped; }
    bool isInterruptEnabled() const { return interruptEnabled; }
    bool hasPendingInterruptEnable() const { return pendingInterruptEnable; }

    // Stack operations
    WORD popFromStack() { return popStack(); }
    void pushToStack(WORD value) { pushStack(value); }

private:
    // Initialize instruction units
    void initInstructionUnits();

    // Stack Operations
    void pushStack(WORD value);
    WORD popStack();

    // Interrupt Handling
    void serviceInterrupt(WORD address, BYTE interruptBit);
    bool checkInterrupts();
    void handleVBlankInterrupt();
    void handleLCDInterrupt();
    void handleTimerInterrupt();
    void handleJoypadInterrupt();
    void clearInterruptFlag(BYTE bit);
    BYTE getPendingInterrupts() const;

    // Opcode Execution
    int ExecuteOpcode(BYTE opcode);
    int ExecuteExtendedOpcode();

    // Debug Helpers
    void logOpcode(BYTE opcode);
};