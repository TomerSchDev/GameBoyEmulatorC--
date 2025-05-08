#pragma once
#include "cpu_instruction_base.h"

class JumpInstructions : public CPUInstructionBase {
public:
    explicit JumpInstructions(CPU& cpuRef) : CPUInstructionBase(cpuRef) {}
    
    // Main execution method
    int execute(BYTE opcode) override;
    const char* getClassName() const override { return "JumpInstructions"; }


private:
    // Jump Operations
    int CPU_JP(WORD address);
    int CPU_JP_CC(WORD address, bool condition);
    int CPU_JR(BYTE offset);
    int CPU_JR_CC(BYTE offset, bool condition);
    int CPU_CALL(WORD address);
    int CPU_CALL_CC(WORD address, bool condition);
    int CPU_RET();
    int CPU_RET_CC(bool condition);
    int CPU_RST(BYTE vector);
    int CPU_CALL_nn();
};