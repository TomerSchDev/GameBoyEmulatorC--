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
    // Tests 'bit' of 'value_to_test', affects CPU flags.
    void CPU_BIT(BYTE value_to_test, BYTE bit);
    // Sets 'bit' of 'data_ref'. For operations on (HL) (e.g., SET 7, (HL) - opcode 0xCBFE),
    // the execute() method is expected to read the byte from (HL), call this function
    // with a reference to that byte, and then write the modified byte back to (HL).
    void CPU_SET(BYTE& data_ref, BYTE bit);
    // Resets 'bit' of 'data_ref'. Similar (HL) handling as CPU_SET.
    void CPU_RES(BYTE& data_ref, BYTE bit);
    void CPU_RL(BYTE& data_ref);    // Rotate Left
    void CPU_RR(BYTE& data_ref);    // Rotate Right
    void CPU_RLC(BYTE& data_ref);   // Rotate Left through Carry
    void CPU_RRC(BYTE& data_ref);   // Rotate Right through Carry
    void CPU_SLA(BYTE& data_ref);   // Shift Left Arithmetic
    void CPU_SRA(BYTE& data_ref);   // Shift Right Arithmetic
    void CPU_SRL(BYTE& data_ref);   // Shift Right Logical
    void CPU_SWAP(BYTE& data_ref);  // Swap nibbles
};