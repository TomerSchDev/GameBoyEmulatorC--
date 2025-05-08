#pragma once
#include "cpu_instruction_base.h"

class LoadInstructions : public CPUInstructionBase {
public:
    explicit LoadInstructions(CPU& cpuRef) : CPUInstructionBase(cpuRef) {}
    const char* getClassName() const override { return "LoadInstructions"; }

    
    // Main execution method
    int execute(BYTE opcode) override;

private:
    // Load Operations
    int CPU_LD_R_R(BYTE& dest, BYTE src);
    int CPU_LD_R_HL(BYTE& reg);
    int CPU_LD_A_BC();
    int CPU_LD_A_DE();
    int CPU_LD_BC_A();
    int CPU_LD_DE_A();
    int CPU_LD_A_FF00_N();
    int CPU_LD_FF00_N_A();
    int CPU_LD_A_HLI();    // Load A from (HL) and increment HL
    int CPU_LD_A_HLD();    // Load A from (HL) and decrement HL
    int CPU_LD_HLI_A();    // Load (HL) from A and increment HL
    int CPU_LD_HLD_A();    // Load (HL) from A and decrement HL
    int CPU_LD_HL_R(BYTE src_val);  // Load HL from SP + n
};