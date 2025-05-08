#pragma once
#include "cpu_instruction_base.h"
// Forward declare CPU if only used as a reference/pointer in the header

class JumpInstructions : public CPUInstructionBase {
public:
    explicit JumpInstructions(CPU& cpuRef) : CPUInstructionBase(cpuRef) {}
    
    // Main execution method
    int execute(BYTE opcode) override;
    const char* getClassName() const override { return "JumpInstructions"; }


private:
    // Helper to check flag conditions
    // condition_type: 00:NZ, 01:Z, 02:NC, 03:C
    bool checkCondition(BYTE condition_type);

    // Jump Operations
    void CPU_JP_nn();    // JP nn
    void CPU_JP_HL();    // JP (HL)
    bool CPU_JP_cc_nn(BYTE condition_type); // JP cc, nn

    void CPU_JR_n();     // JR n
    bool CPU_JR_cc_n(BYTE condition_type);  // JR cc, n

    void CPU_CALL_nn();  // CALL nn
    bool CPU_CALL_cc_nn(BYTE condition_type); // CALL cc, nn

    void CPU_RET();      // RET
    void CPU_RETI();     // RETI
    bool CPU_RET_cc(BYTE condition_type);   // RET cc

    void CPU_RST(BYTE vector); // RST n
};