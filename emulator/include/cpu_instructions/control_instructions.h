#pragma once
#include "cpu_instruction_base.h"

class ControlInstructions : public CPUInstructionBase {
public:
    explicit ControlInstructions(CPU& cpuRef) : CPUInstructionBase(cpuRef) {}
    const char* getClassName() const override { return "ControlInstructions"; }

    
    // Main execution method
    int execute(BYTE opcode) override;

private:
    // Control Operations
    int CPU_NOP();
    int CPU_HALT();
    int CPU_STOP();
    int CPU_DI();
    int CPU_EI();
    int CPU_CCF();
    int CPU_SCF();
    int CPU_DAA();
    int CPU_CPL();
};