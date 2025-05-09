#pragma once
#include "common.h"  // Add this at the top

#include <memory>
#include <string> // Required for std::string in OpcodeInfo
#include "memory_controller.h"
#include "logger.h"
// #include "cpu_constants.h" // Assuming this provides general constants if needed, but not opcode tables
#include "OpcodeTables.h" // For OpcodeInfo and OpcodeTables class

// Forward declaration
class MemoryController;

namespace GB {

// Union for CPU register pairs (e.g., AF, BC, DE, HL, SP)
// Renamed from 'Register' in your original cpu.h to 'RegisterPair'
// to avoid conflict with GB::Register enum from OpcodeTables.h
union RegisterPair {
    WORD reg;
    struct {
        BYTE lo;
        BYTE hi;
    };
};

class CPU {
public:
    // --- Public Constants for Flags ---
    static const BYTE FLAG_Z_BIT = 7; // Zero Flag bit position
    static const BYTE FLAG_N_BIT = 6; // Subtract Flag bit position
    static const BYTE FLAG_H_BIT = 5; // Half Carry Flag bit position
    static const BYTE FLAG_C_BIT = 4; // Carry Flag bit position

    // Flag masks for easy manipulation
    static const BYTE FLAG_Z_MASK = (1 << FLAG_Z_BIT);
    static const BYTE FLAG_N_MASK = (1 << FLAG_N_BIT);
    static const BYTE FLAG_H_MASK = (1 << FLAG_H_BIT);
    static const BYTE FLAG_C_MASK = (1 << FLAG_C_BIT);

private:
    // --- CPU Members ---
    std::shared_ptr<MemoryController> memoryController; // Interface to memory

    // CPU Registers
    RegisterPair m_RegisterAF;  // Accumulator (A) & Flags (F)
    RegisterPair m_RegisterBC;  // General Purpose Register Pair BC
    RegisterPair m_RegisterDE;  // General Purpose Register Pair DE
    RegisterPair m_RegisterHL;  // General Purpose Register Pair HL / Memory Pointer
    WORD m_ProgramCounter;      // Program Counter (PC)
    RegisterPair m_StackPointer;  // Stack Pointer (SP)

    // CPU State
    bool halted;                // Is CPU in HALT state?
    bool stopped;               // Is CPU in STOP state?
    bool interruptEnabled;      // Master Interrupt Enable Flag (IME)
    bool pendingInterruptEnable; // EI instruction sets this to enable interrupts after the *next* instruction

    OpcodeTables& opcodeTables; // Reference to the singleton opcode table instance

public:
    // --- Constructor & Destructor ---
    CPU(std::shared_ptr<MemoryController> memory);
    ~CPU() = default;

    // --- Core CPU Operations ---
    int ExecuteNextOpcode(); // Fetches, decodes, and executes the next opcode
    void Reset();            // Resets CPU to its initial state
    void RequestInterrupt(BYTE interruptBit); // Request an interrupt (sets bit in IF register)

    // --- Memory Access ---
    // These are used by instruction implementations
    BYTE readMemory(WORD address) const;
    void writeMemory(WORD address, BYTE data);
    BYTE readBytePC(); // Reads byte at current PC and increments PC
    WORD readWordPC(); // Reads word at current PC and increments PC by 2

    // --- Register Access ---
    // Getters for individual 8-bit registers
    BYTE& getA() { return m_RegisterAF.hi; }
    BYTE& getF() { return m_RegisterAF.lo; } // Note: Lower 4 bits of F are always 0
    BYTE& getB() { return m_RegisterBC.hi; }
    BYTE& getC() { return m_RegisterBC.lo; }
    BYTE& getD() { return m_RegisterDE.hi; }
    BYTE& getE() { return m_RegisterDE.lo; }
    BYTE& getH() { return m_RegisterHL.hi; }
    BYTE& getL() { return m_RegisterHL.lo; }

    // Getters/Setters for 16-bit register pairs
    WORD getAF() const { return m_RegisterAF.reg; }
    void setAF(WORD value) { m_RegisterAF.reg = value; m_RegisterAF.lo &= 0xF0; /* Ensure lower 4 bits of F are zero */ }
    WORD getBC() const { return m_RegisterBC.reg; }
    void setBC(WORD value) { m_RegisterBC.reg = value; }
    WORD getDE() const { return m_RegisterDE.reg; }
    void setDE(WORD value) { m_RegisterDE.reg = value; }
    WORD getHL() const { return m_RegisterHL.reg; }
    void setHL(WORD value) { m_RegisterHL.reg = value; }

    WORD getPC() const { return m_ProgramCounter; }
    void setPC(WORD value) { m_ProgramCounter = value; }
    WORD getSP() const { return m_StackPointer.reg; }
    void setSP(WORD value) { m_StackPointer.reg = value; }

    // --- Flag Management ---
    void setFlagZ(bool value);
    void setFlagN(bool value);
    void setFlagH(bool value);
    void setFlagC(bool value);

    bool getFlagZ() const;
    bool getFlagN() const;
    bool getFlagH() const;
    bool getFlagC() const;

    // --- CPU State Control ---
    void setHaltState(bool state) { halted = state; }
    bool isHalted() const { return halted; }
    void setStopState(bool state) { stopped = state; } // Note: STOP also involves LCD behavior
    bool isStopped() const { return stopped; }
    void enableInterrupts() { interruptEnabled = true; } // For EI instruction effect
    void disableInterrupts() { interruptEnabled = false;} // For DI instruction effect
    void scheduleInterruptEnable() { pendingInterruptEnable = true; } // For EI instruction
    bool isInterruptMasterEnabled() const { return interruptEnabled; }
    int handleInterrupts();    // Checks and services pending interrupts



    // --- Stack Operations ---
    // Used by PUSH, POP, CALL, RET instructions
    void pushStackWord(WORD value);
    WORD popStackWord();

private:
    // --- Internal Helper Methods ---
    int processInstruction(const OpcodeInfo& info); // Dispatches to specific instruction handlers
    int executePrefixedInstruction(); // Handles CB-prefixed instructions
    int handleUnknownOpcode(BYTE opcode, bool prefixed); // Handles undefined opcodes

    // Debug Helpers
    void logOpcodeExecution(BYTE opcode_val, bool is_prefixed, const OpcodeInfo& info, WORD current_pc_before_fetch);
};

} // namespace GB
