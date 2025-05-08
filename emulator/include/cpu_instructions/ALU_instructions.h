#pragma once
#include "cpu_instruction_base.h"

// Forward declaration
// class CPU; // Already in cpu_instruction_base.h likely

class ALUInstructions : public CPUInstructionBase {
public:
    explicit ALUInstructions(CPU& cpuRef) : CPUInstructionBase(cpuRef) {}
    
    // Main execution method (override from base class)
    int execute(BYTE opcode) override;
    const char* getClassName() const override { return "ALUInstructions"; }

private:
    // 8-bit ALU Operations (mostly on Accumulator A)
    void CPU_ADD_A(BYTE value, bool use_carry);
    void CPU_SUB_A(BYTE value, bool use_carry); // Also base for CP
    void CPU_AND_A(BYTE value);
    void CPU_OR_A(BYTE value);
    void CPU_XOR_A(BYTE value);
    void CPU_CP_A(BYTE value);  // Compare A with value

    // 8-bit Increment/Decrement
    void CPU_INC_REG(BYTE& reg_ref); // For registers B,C,D,E,H,L,A
    void CPU_DEC_REG(BYTE& reg_ref); // For registers B,C,D,E,H,L,A
    void CPU_INC_HL_MEM();           // For (HL)
    void CPU_DEC_HL_MEM();           // For (HL)

    // 16-bit ALU Operations
    void CPU_ADD_HL_RR(WORD rr_value); // For ADD HL,BC; ADD HL,DE; ADD HL,HL; ADD HL,SP
    // ADD SP, n (0xE8) is also a 16-bit ALU op, often handled separately or with a dedicated helper
    void CPU_ADD_SP_N();


    // Special ALU Operations
    void CPU_DAA(); // Decimal Adjust Accumulator
    void CPU_DAA_UserLogic(); // User logic for DAA, if needed
    // CCF, SCF, CPL are sometimes here, sometimes in Control. Assuming they are in Control as per previous discussion.
};