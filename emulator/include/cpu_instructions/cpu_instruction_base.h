#pragma once
#include "../common.h"
#include "../cpu_constants.h"
#include "../cpu.h"
class CPU;  // Forward declaration of CPU class

class CPUInstructionBase {
public:
    explicit CPUInstructionBase(CPU& cpu) : cpu(cpu) {}
    virtual ~CPUInstructionBase() = default;
    virtual const char* getClassName() const = 0; // Pure virtual function to get class name
    
    // Main execution interface
    virtual int execute(BYTE opcode) = 0;

protected:
    CPU& cpu;  // Reference to the CPU this instruction unit belongs to
    

    // Flag manipulation helpers - declarations only
    void setZeroFlag(bool set);
    void setSubtractFlag(bool set);
    void setHalfCarryFlag(bool set);
    void setCarryFlag(bool set);
    bool checkFlag(BYTE flag) const;
    void logUnhandledOpcode(BYTE opcode); 

};