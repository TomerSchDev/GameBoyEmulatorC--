#pragma once
#include "cpu_instruction_base.h"

class ALUInstructions : public CPUInstructionBase {
public:
    explicit ALUInstructions(CPU& cpuRef) : CPUInstructionBase(cpuRef) {}
    
    // Main execution method (override from base class)
    int execute(BYTE opcode) override;
    const char* getClassName() const override { return "ALUInstructions"; }


private:
    // ALU Operations
    int CPU_8BIT_ADD(BYTE& reg1, BYTE reg2, bool useCarry);
    int CPU_8BIT_SUB(BYTE& reg1, BYTE reg2, bool useCarry);
    int CPU_8BIT_AND(BYTE& reg1, BYTE reg2);
    int CPU_8BIT_OR(BYTE& reg1, BYTE reg2);
    int CPU_8BIT_XOR(BYTE& reg1, BYTE reg2);
    int CPU_8BIT_CP(BYTE reg1, BYTE reg2);
    int CPU_8BIT_INC(BYTE& reg);
    int CPU_8BIT_DEC(BYTE& reg);
};