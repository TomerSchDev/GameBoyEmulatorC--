#pragma once
#include "cpu_instruction_base.h"

class BitInstructions : public CPUInstructionBase {
public:
    explicit BitInstructions(CPU& cpuRef) : CPUInstructionBase(cpuRef) {}
    
    // Main execution method (override from base class)
    int execute(BYTE opcode) override;
    const char* getClassName() const override { return "BitInstructions"; }


private:
    // Bit Operations
    void CPU_BIT(BYTE reg, BYTE bit);
    void CPU_SET(BYTE& reg, BYTE bit);
    void CPU_RES(BYTE& reg, BYTE bit);
    void CPU_RL(BYTE& reg);
    void CPU_RR(BYTE& reg);
    void CPU_RLC(BYTE& reg);
    void CPU_RRC(BYTE& reg);
    void CPU_SLA(BYTE& reg);
    void CPU_SRA(BYTE& reg);
    void CPU_SRL(BYTE& reg);
    void CPU_SWAP(BYTE& reg);
};