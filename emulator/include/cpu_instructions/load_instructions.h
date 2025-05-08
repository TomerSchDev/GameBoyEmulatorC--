#pragma once
#include "cpu_instruction_base.h"

class LoadInstructions : public CPUInstructionBase { // <<< इंश्योर दिस इज LoadInstructions
public:
    explicit LoadInstructions(CPU& cpuRef) : CPUInstructionBase(cpuRef) {}

    int execute(BYTE opcode) override;
    const char* getClassName() const override { return "LoadInstructions"; }

private:
    // Helper methods for LD r,r' ; LD r,(HL) ; LD (HL),r
    void CPU_LD_R_R(BYTE& dest_reg_ref, BYTE src_val);
    void CPU_LD_R_HL(BYTE& dest_reg_ref); 
    void CPU_LD_HL_R(BYTE src_val);       

    // Helpers for specific load instructions
    void CPU_LD_BC_A();
    void CPU_LD_DE_A();
    void CPU_LD_HLI_A(); 
    void CPU_LD_HLD_A(); 

    void CPU_LD_A_BC();
    void CPU_LD_A_DE();
    void CPU_LD_A_HLI(); 
    void CPU_LD_A_HLD(); 

    void CPU_LD_A_FF00_N(); 
    void CPU_LD_FF00_N_A(); 
    void CPU_LD_A_FF00_C(); 
    void CPU_LD_FF00_C_A(); 

    void CPU_LD_A_NN();     
    void CPU_LD_NN_A();     

    // 8-bit immediate loads to registers
    void CPU_LD_B_d8();
    void CPU_LD_C_d8();
    void CPU_LD_D_d8();
    void CPU_LD_E_d8();
    void CPU_LD_H_d8();
    void CPU_LD_L_d8();
    void CPU_LD_A_d8();
    void CPU_LD_HL_d8();    

    // 16-bit immediate loads
    void CPU_LD_BC_d16();
    void CPU_LD_DE_d16();
    void CPU_LD_HL_d16();
    void CPU_LD_SP_d16();

    // Misc loads
    void CPU_LD_SP_HL();    
    void CPU_LDHL_SP_N();   

    BYTE& getRegisterReference(BYTE reg_index);
    BYTE getRegisterValue(BYTE reg_index, bool& is_hl_memory); 
};