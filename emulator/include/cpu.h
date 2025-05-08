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
    public:
        static const BYTE FLAG_Z_BIT = 7; // Zero Flag
        static const BYTE FLAG_N_BIT = 6; // Subtract Flag
        static const BYTE FLAG_H_BIT = 5; // Half Carry Flag
        static const BYTE FLAG_C_BIT = 4; // Carry Flag

        // Flag masks
        static const BYTE FLAG_Z_MASK = (1 << FLAG_Z_BIT);
        static const BYTE FLAG_N_MASK = (1 << FLAG_N_BIT);
        static const BYTE FLAG_H_MASK = (1 << FLAG_H_BIT);
        static const BYTE FLAG_C_MASK = (1 << FLAG_C_BIT);
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
        // Provide access to individual registers if needed by instruction units
        BYTE& getA() { return m_RegisterAF.hi; }
        BYTE& getB() { return m_RegisterBC.hi; }
        BYTE& getC() { return m_RegisterBC.lo; }
        BYTE& getD() { return m_RegisterDE.hi; }
        BYTE& getE() { return m_RegisterDE.lo; }
        BYTE& getH() { return m_RegisterHL.hi; }
        BYTE& getL() { return m_RegisterHL.lo; }

        // Access to register pairs
        Register& getAF_ref() { return m_RegisterAF; } // If direct access to pair is needed
        Register& getBC_ref() { return m_RegisterBC; }
        Register& getDE_ref() { return m_RegisterDE; }
        Register& getHL_ref() { return m_RegisterHL; }


        WORD& getPC() { return m_ProgramCounter; }
        Register& getSP_ref() { return m_StackPointer; } // If direct access to pair is needed
        WORD getSP() const { return m_StackPointer.reg; }
        void setSP(WORD value) { m_StackPointer.reg = value; }
        // --- Flag Management ---
        // Primary public interface for flags
        void setFlags(BYTE new_f_value);
        BYTE getFlags() const;
        
        // Helper getters using the primary getFlags()
        bool getFlagZ() const;
        bool getFlagN() const;
        bool getFlagH() const;
        bool getFlagC() const;

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
        WORD popFromStack() { return popStackInternal(); }
        void pushToStack(WORD value) { pushStackInternal(value);}

    private:
        // Initialize instruction units
        void initInstructionUnits();

        // Stack Operations
        WORD popStackInternal();
        void pushStackInternal(WORD value);

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