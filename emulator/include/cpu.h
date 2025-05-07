#pragma once

#include <common.h>
#include <string>
#include <memory>
#include <memory_controller.h>

union Register
{
  WORD reg ;
  struct
  {
   BYTE lo ;
   BYTE hi ;
  };
};

class CPU {
    private:
        BYTE m_ScreenData[SCREEN_PIXELS_WIDTH][SCREEN_PIXELS_HEIGHT][3];
        Register m_RegisterAF;
        Register m_RegisterBC;
        Register m_RegisterDE;
        Register m_RegisterHL; 
        WORD m_ProgramCounter;
        Register m_StackPointer;
        
        std::shared_ptr<MemoryController> memoryController;
        // CPU state
        bool halted;
        bool stopped;
        bool pendingInterruptEnable;  // New flag for handling EI instruction delay
        bool interruptEnabled;


        
    
    public:
        CPU(std::shared_ptr<MemoryController> memory);
        ~CPU() = default;
        
        // Main CPU operations
        int ExecuteNextOpcode();
        void Reset();
        void handleInterrupts(BYTE pendingInterrupts);
    
    private:
        BYTE readMemory(WORD address) const;
        void writeMemory(WORD address, BYTE data);
        void pushStack(WORD value);
        WORD popStack();
        // Program counter helpers
        BYTE readByte();
        WORD readWord();
        void serviceInterrupt(WORD address, BYTE interruptBit);
        // Interrupt handling helpers
        bool checkInterrupts();
        void handleVBlankInterrupt();
        void handleLCDInterrupt();
        void handleTimerInterrupt();
        void handleJoypadInterrupt();
        
        // Opcode execution helpers
        int executeControlOpcode(BYTE opcode);
        int executeLoadOpcode(BYTE opcode);
        int executeALUOpcode(BYTE opcode);
        int executeJumpOpcode(BYTE opcode);
        void logOpcode(BYTE opcode);
        int handleUnknownOpcode(BYTE opcode);
        void clearInterruptFlag(BYTE bit);
        BYTE getPendingInterrupts() const ;
    };