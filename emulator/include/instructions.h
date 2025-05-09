#pragma once
#include "common.h"  // Add this at the top
#include "OpcodeTables.h" // For OpcodeInfo and enums like GB::Register

namespace GB {

// Forward declaration of CPU class to avoid circular include with cpu.h
class CPU;

// --- Instruction Implementations Signatures ---
// Each function takes a reference to the CPU and the OpcodeInfo for the current instruction.
// They return the number of T-cycles the instruction took.

// Group: CONTROL_MISC
int NOP_impl(CPU& cpu, const OpcodeInfo& info);
int HALT_impl(CPU& cpu, const OpcodeInfo& info);
int STOP_impl(CPU& cpu, const OpcodeInfo& info); // Note: STOP has specific hardware behavior
int DI_impl(CPU& cpu, const OpcodeInfo& info);
int EI_impl(CPU& cpu, const OpcodeInfo& info);
int DAA_impl(CPU& cpu, const OpcodeInfo& info);
int CPL_impl(CPU& cpu, const OpcodeInfo& info);
int SCF_impl(CPU& cpu, const OpcodeInfo& info);
int CCF_impl(CPU& cpu, const OpcodeInfo& info);

// Group: X8_LSM (8-bit Load/Store/Move)
int LD_reg_reg_impl(CPU& cpu, const OpcodeInfo& info);      // LD r, r'
int LD_reg_n8_impl(CPU& cpu, const OpcodeInfo& info);       // LD r, n8
int LD_reg_memHL_impl(CPU& cpu, const OpcodeInfo& info);    // LD r, (HL)
int LD_memHL_reg_impl(CPU& cpu, const OpcodeInfo& info);    // LD (HL), r
int LD_memHL_n8_impl(CPU& cpu, const OpcodeInfo& info);     // LD (HL), n8
int LD_A_memBC_impl(CPU& cpu, const OpcodeInfo& info);      // LD A, (BC)
int LD_A_memDE_impl(CPU& cpu, const OpcodeInfo& info);      // LD A, (DE)
int LD_A_memA16_impl(CPU& cpu, const OpcodeInfo& info);     // LD A, (a16)
int LD_memBC_A_impl(CPU& cpu, const OpcodeInfo& info);      // LD (BC), A
int LD_memDE_A_impl(CPU& cpu, const OpcodeInfo& info);      // LD (DE), A
int LD_memA16_A_impl(CPU& cpu, const OpcodeInfo& info);     // LD (a16), A
int LDH_memA8_A_impl(CPU& cpu, const OpcodeInfo& info);     // LDH (a8), A
int LDH_A_memA8_impl(CPU& cpu, const OpcodeInfo& info);     // LDH A, (a8)
int LDH_memC_A_impl(CPU& cpu, const OpcodeInfo& info);      // LDH (C), A  (same as LD (0xFF00+C), A)
int LDH_A_memC_impl(CPU& cpu, const OpcodeInfo& info);      // LDH A, (C)  (same as LD A, (0xFF00+C))
int LD_A_memHLI_impl(CPU& cpu, const OpcodeInfo& info);     // LD A, (HL+)
int LD_A_memHLD_impl(CPU& cpu, const OpcodeInfo& info);     // LD A, (HL-)
int LD_memHLI_A_impl(CPU& cpu, const OpcodeInfo& info);     // LD (HL+), A
int LD_memHLD_A_impl(CPU& cpu, const OpcodeInfo& info);     // LD (HL-), A

// Group: X16_LSM (16-bit Load/Store/Move)
int LD_rr_n16_impl(CPU& cpu, const OpcodeInfo& info);       // LD rr, n16 (rr = BC, DE, HL, SP)
int LD_SP_HL_impl(CPU& cpu, const OpcodeInfo& info);        // LD SP, HL
int LD_memA16_SP_impl(CPU& cpu, const OpcodeInfo& info);    // LD (a16), SP
int LD_HL_SP_e8_impl(CPU& cpu, const OpcodeInfo& info);     // LD HL, SP+e8 (e8 is signed immediate)
int PUSH_rr_impl(CPU& cpu, const OpcodeInfo& info);         // PUSH rr (rr = AF, BC, DE, HL)
int POP_rr_impl(CPU& cpu, const OpcodeInfo& info);          // POP rr (rr = AF, BC, DE, HL)

// Group: X8_ALU (8-bit Arithmetic/Logic)
int ADD_A_reg_impl(CPU& cpu, const OpcodeInfo& info);       // ADD A, r
int ADD_A_n8_impl(CPU& cpu, const OpcodeInfo& info);        // ADD A, n8
int ADD_A_memHL_impl(CPU& cpu, const OpcodeInfo& info);     // ADD A, (HL)
int ADC_A_reg_impl(CPU& cpu, const OpcodeInfo& info);       // ADC A, r
int ADC_A_n8_impl(CPU& cpu, const OpcodeInfo& info);        // ADC A, n8
int ADC_A_memHL_impl(CPU& cpu, const OpcodeInfo& info);     // ADC A, (HL)
int SUB_A_reg_impl(CPU& cpu, const OpcodeInfo& info);       // SUB A, r  (or SUB r)
int SUB_A_n8_impl(CPU& cpu, const OpcodeInfo& info);        // SUB A, n8 (or SUB n8)
int SUB_A_memHL_impl(CPU& cpu, const OpcodeInfo& info);     // SUB A, (HL) (or SUB (HL))
int SBC_A_reg_impl(CPU& cpu, const OpcodeInfo& info);       // SBC A, r
int SBC_A_n8_impl(CPU& cpu, const OpcodeInfo& info);        // SBC A, n8
int SBC_A_memHL_impl(CPU& cpu, const OpcodeInfo& info);     // SBC A, (HL)
int AND_A_reg_impl(CPU& cpu, const OpcodeInfo& info);       // AND A, r (or AND r)
int AND_A_n8_impl(CPU& cpu, const OpcodeInfo& info);        // AND A, n8 (or AND n8)
int AND_A_memHL_impl(CPU& cpu, const OpcodeInfo& info);     // AND A, (HL) (or AND (HL))
int XOR_A_reg_impl(CPU& cpu, const OpcodeInfo& info);       // XOR A, r (or XOR r)
int XOR_A_n8_impl(CPU& cpu, const OpcodeInfo& info);        // XOR A, n8 (or XOR n8)
int XOR_A_memHL_impl(CPU& cpu, const OpcodeInfo& info);     // XOR A, (HL) (or XOR (HL))
int OR_A_reg_impl(CPU& cpu, const OpcodeInfo& info);        // OR A, r (or OR r)
int OR_A_n8_impl(CPU& cpu, const OpcodeInfo& info);         // OR A, n8 (or OR n8)
int OR_A_memHL_impl(CPU& cpu, const OpcodeInfo& info);      // OR A, (HL) (or OR (HL))
int CP_A_reg_impl(CPU& cpu, const OpcodeInfo& info);        // CP A, r (or CP r)
int CP_A_n8_impl(CPU& cpu, const OpcodeInfo& info);         // CP A, n8 (or CP n8)
int CP_A_memHL_impl(CPU& cpu, const OpcodeInfo& info);      // CP A, (HL) (or CP (HL))
int INC_reg_impl(CPU& cpu, const OpcodeInfo& info);         // INC r (8-bit register)
int INC_memHL_impl(CPU& cpu, const OpcodeInfo& info);       // INC (HL)
int DEC_reg_impl(CPU& cpu, const OpcodeInfo& info);         // DEC r (8-bit register)
int DEC_memHL_impl(CPU& cpu, const OpcodeInfo& info);       // DEC (HL)

// Group: X16_ALU (16-bit Arithmetic/Logic)
int ADD_HL_rr_impl(CPU& cpu, const OpcodeInfo& info);       // ADD HL, rr (rr = BC, DE, HL, SP)
int ADD_SP_e8_impl(CPU& cpu, const OpcodeInfo& info);       // ADD SP, e8 (e8 is signed immediate)
int INC_rr_impl(CPU& cpu, const OpcodeInfo& info);          // INC rr (16-bit register: BC, DE, HL, SP)
int DEC_rr_impl(CPU& cpu, const OpcodeInfo& info);          // DEC rr (16-bit register: BC, DE, HL, SP)

// Group: X8_RSB (8-bit Rotate/Shift/Bit - Non-CB prefixed)
int RLCA_impl(CPU& cpu, const OpcodeInfo& info);
int RLA_impl(CPU& cpu, const OpcodeInfo& info);
int RRCA_impl(CPU& cpu, const OpcodeInfo& info);
int RRA_impl(CPU& cpu, const OpcodeInfo& info);

// Group: X8_RSB (CB-Prefixed Instructions)
int RLC_reg_impl(CPU& cpu, const OpcodeInfo& info);         // RLC r
int RLC_memHL_impl(CPU& cpu, const OpcodeInfo& info);       // RLC (HL)
int RRC_reg_impl(CPU& cpu, const OpcodeInfo& info);         // RRC r
int RRC_memHL_impl(CPU& cpu, const OpcodeInfo& info);       // RRC (HL)
int RL_reg_impl(CPU& cpu, const OpcodeInfo& info);          // RL r
int RL_memHL_impl(CPU& cpu, const OpcodeInfo& info);        // RL (HL)
int RR_reg_impl(CPU& cpu, const OpcodeInfo& info);          // RR r
int RR_memHL_impl(CPU& cpu, const OpcodeInfo& info);        // RR (HL)
int SLA_reg_impl(CPU& cpu, const OpcodeInfo& info);         // SLA r
int SLA_memHL_impl(CPU& cpu, const OpcodeInfo& info);       // SLA (HL)
int SRA_reg_impl(CPU& cpu, const OpcodeInfo& info);         // SRA r
int SRA_memHL_impl(CPU& cpu, const OpcodeInfo& info);       // SRA (HL)
int SWAP_reg_impl(CPU& cpu, const OpcodeInfo& info);        // SWAP r
int SWAP_memHL_impl(CPU& cpu, const OpcodeInfo& info);      // SWAP (HL)
int SRL_reg_impl(CPU& cpu, const OpcodeInfo& info);         // SRL r
int SRL_memHL_impl(CPU& cpu, const OpcodeInfo& info);       // SRL (HL)
int BIT_b_reg_impl(CPU& cpu, const OpcodeInfo& info);       // BIT b, r
int BIT_b_memHL_impl(CPU& cpu, const OpcodeInfo& info);     // BIT b, (HL)
int RES_b_reg_impl(CPU& cpu, const OpcodeInfo& info);       // RES b, r
int RES_b_memHL_impl(CPU& cpu, const OpcodeInfo& info);     // RES b, (HL)
int SET_b_reg_impl(CPU& cpu, const OpcodeInfo& info);       // SET b, r
int SET_b_memHL_impl(CPU& cpu, const OpcodeInfo& info);     // SET b, (HL)

// Group: CONTROL_BR (Control/Branch)
int JP_n16_impl(CPU& cpu, const OpcodeInfo& info);          // JP a16
int JP_cc_n16_impl(CPU& cpu, const OpcodeInfo& info);       // JP cc, a16
int JP_HL_impl(CPU& cpu, const OpcodeInfo& info);           // JP HL
int JR_e8_impl(CPU& cpu, const OpcodeInfo& info);           // JR e8
int JR_cc_e8_impl(CPU& cpu, const OpcodeInfo& info);        // JR cc, e8
int CALL_n16_impl(CPU& cpu, const OpcodeInfo& info);        // CALL a16
int CALL_cc_n16_impl(CPU& cpu, const OpcodeInfo& info);     // CALL cc, a16
int RET_impl(CPU& cpu, const OpcodeInfo& info);             // RET
int RET_cc_impl(CPU& cpu, const OpcodeInfo& info);          // RET cc
int RETI_impl(CPU& cpu, const OpcodeInfo& info);            // RETI
int RST_impl(CPU& cpu, const OpcodeInfo& info);             // RST n

} // namespace GB
