#pragma once
#include "cpu_instruction_base.h"
// Forward declare CPU if only used as a reference/pointer in the header
// class CPU; 

class ControlInstructions : public CPUInstructionBase {
public:
    explicit ControlInstructions(CPU& cpuRef) : CPUInstructionBase(cpuRef) {}
    const char* getClassName() const override { return "ControlInstructions"; }

    
    // Main execution method
    int execute(BYTE opcode) override;

private:
    // Control Operations
    void CPU_NOP();
    void CPU_HALT();
    void CPU_STOP();
    void CPU_DI();  // Disable Interrupts
    void CPU_EI();  // Enable Interrupts

    // These are often categorized as ALU/Misc, but if they are part of ControlInstructions:
    void CPU_CCF(); // Complement Carry Flag
    void CPU_SCF(); // Set Carry Flag
    void CPU_DAA(); // Decimal Adjust Accumulator
    void CPU_CPL(); // Complement Accumulator (A = ~A)
};