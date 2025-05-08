#pragma once
#include <common.h>
#include <vector>
#include <array>

namespace CPUConstants {
    // Opcode group masks
    constexpr BYTE OPCODE_GROUP_MASK = 0xF0;

    // Opcode groups (first nibble)
    constexpr BYTE CONTROL_GROUP = 0x00;  // 0x00-0x0F
    constexpr BYTE JUMP_GROUP_1 = 0x10;   // 0x10-0x1F
    constexpr BYTE JUMP_GROUP_2 = 0x20;   // 0x20-0x2F
    constexpr BYTE JUMP_GROUP_3 = 0x30;   // 0x30-0x3F
    constexpr BYTE LOAD_GROUP_1 = 0x40;   // 0x40-0x4F
    constexpr BYTE LOAD_GROUP_2 = 0x50;   // 0x50-0x5F
    constexpr BYTE LOAD_GROUP_3 = 0x60;   // 0x60-0x6F
    constexpr BYTE LOAD_GROUP_4 = 0x70;   // 0x70-0x7F
    constexpr BYTE ALU_GROUP_1 = 0x80;    // 0x80-0x8F
    constexpr BYTE ALU_GROUP_2 = 0x90;    // 0x90-0x9F
    constexpr BYTE ALU_GROUP_3 = 0xA0;    // 0xA0-0xAF
    constexpr BYTE ALU_GROUP_4 = 0xB0;    // 0xB0-0xBF

    enum class InstructionType {
        CONTROL,
        JUMP,
        LOAD,
        ALU,
        BIT,
        UNKNOWN
    };

    struct OpcodeTableEntry {
        BYTE opcode_value;
        const char* mnemonic;
        BYTE length_in_bytes;
        BYTE duration_cycles;
        BYTE duration_cycles_conditional;
        char flag_Z_char;
        char flag_N_char;
        char flag_H_char;
        char flag_C_char;
        BYTE affected_flags_summary_mask;
        InstructionType type;

        OpcodeTableEntry(BYTE op_val, const char* mnem, BYTE len, BYTE cyc, BYTE cyc_cond,
                         char fZ, char fN, char fH, char fC, InstructionType instr_type)
            : opcode_value(op_val), mnemonic(mnem), length_in_bytes(len),
              duration_cycles(cyc), duration_cycles_conditional(cyc_cond),
              flag_Z_char(fZ), flag_N_char(fN), flag_H_char(fH), flag_C_char(fC),
              type(instr_type) {
            affected_flags_summary_mask = 0;
            if (flag_Z_char != '-') affected_flags_summary_mask |= (1 << 3);
            if (flag_N_char != '-') affected_flags_summary_mask |= (1 << 2);
            if (flag_H_char != '-') affected_flags_summary_mask |= (1 << 1);
            if (flag_C_char != '-') affected_flags_summary_mask |= (1 << 0);
        }

        OpcodeTableEntry() // Default for undefined opcodes
            : opcode_value(0xFF), mnemonic("UNDEFINED"), length_in_bytes(1),
              duration_cycles(4), duration_cycles_conditional(0), // Typically 4 cycles for undefined
              flag_Z_char('-'), flag_N_char('-'), flag_H_char('-'), flag_C_char('-'),
              affected_flags_summary_mask(0), type(InstructionType::UNKNOWN) {}
    };
    
    // const std::vector<OpcodeMapping> OPCODE_MAP = { ... }; // This will be removed

    // New Full Opcode Table (256 entries)
    const std::array<OpcodeTableEntry, 256> FULL_OPCODE_TABLE = {{
        // 0x0X
        OpcodeTableEntry(0x00, "NOP", 1, 4, 0, '-', '-', '-', '-', InstructionType::CONTROL),
        OpcodeTableEntry(0x01, "LD BC,d16", 3, 12, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x02, "LD (BC),A", 1, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x03, "INC BC", 1, 8, 0, '-', '-', '-', '-', InstructionType::ALU),
        OpcodeTableEntry(0x04, "INC B", 1, 4, 0, 'Z', '0', 'H', '-', InstructionType::ALU),
        OpcodeTableEntry(0x05, "DEC B", 1, 4, 0, 'Z', '1', 'H', '-', InstructionType::ALU),
        OpcodeTableEntry(0x06, "LD B,d8", 2, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x07, "RLCA", 1, 4, 0, '0', '0', '0', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x08, "LD (a16),SP", 3, 20, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x09, "ADD HL,BC", 1, 8, 0, '-', '0', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x0A, "LD A,(BC)", 1, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x0B, "DEC BC", 1, 8, 0, '-', '-', '-', '-', InstructionType::ALU),
        OpcodeTableEntry(0x0C, "INC C", 1, 4, 0, 'Z', '0', 'H', '-', InstructionType::ALU),
        OpcodeTableEntry(0x0D, "DEC C", 1, 4, 0, 'Z', '1', 'H', '-', InstructionType::ALU),
        OpcodeTableEntry(0x0E, "LD C,d8", 2, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x0F, "RRCA", 1, 4, 0, '0', '0', '0', 'C', InstructionType::ALU),

        // 0x1X
        OpcodeTableEntry(0x10, "STOP", 2, 4, 0, '-', '-', '-', '-', InstructionType::CONTROL), // STOP 0 (length 2: 0x10, 0x00)
        OpcodeTableEntry(0x11, "LD DE,d16", 3, 12, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x12, "LD (DE),A", 1, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x13, "INC DE", 1, 8, 0, '-', '-', '-', '-', InstructionType::ALU),
        OpcodeTableEntry(0x14, "INC D", 1, 4, 0, 'Z', '0', 'H', '-', InstructionType::ALU),
        OpcodeTableEntry(0x15, "DEC D", 1, 4, 0, 'Z', '1', 'H', '-', InstructionType::ALU),
        OpcodeTableEntry(0x16, "LD D,d8", 2, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x17, "RLA", 1, 4, 0, '0', '0', '0', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x18, "JR r8", 2, 12, 0, '-', '-', '-', '-', InstructionType::JUMP),
        OpcodeTableEntry(0x19, "ADD HL,DE", 1, 8, 0, '-', '0', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x1A, "LD A,(DE)", 1, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x1B, "DEC DE", 1, 8, 0, '-', '-', '-', '-', InstructionType::ALU),
        OpcodeTableEntry(0x1C, "INC E", 1, 4, 0, 'Z', '0', 'H', '-', InstructionType::ALU),
        OpcodeTableEntry(0x1D, "DEC E", 1, 4, 0, 'Z', '1', 'H', '-', InstructionType::ALU),
        OpcodeTableEntry(0x1E, "LD E,d8", 2, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x1F, "RRA", 1, 4, 0, '0', '0', '0', 'C', InstructionType::ALU),

        // 0x2X
        OpcodeTableEntry(0x20, "JR NZ,r8", 2, 12, 8, '-', '-', '-', '-', InstructionType::JUMP),
        OpcodeTableEntry(0x21, "LD HL,d16", 3, 12, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x22, "LD (HL+),A", 1, 8, 0, '-', '-', '-', '-', InstructionType::LOAD), // LDI (HL),A
        OpcodeTableEntry(0x23, "INC HL", 1, 8, 0, '-', '-', '-', '-', InstructionType::ALU),
        OpcodeTableEntry(0x24, "INC H", 1, 4, 0, 'Z', '0', 'H', '-', InstructionType::ALU),
        OpcodeTableEntry(0x25, "DEC H", 1, 4, 0, 'Z', '1', 'H', '-', InstructionType::ALU),
        OpcodeTableEntry(0x26, "LD H,d8", 2, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x27, "DAA", 1, 4, 0, 'Z', '-', '0', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x28, "JR Z,r8", 2, 12, 8, '-', '-', '-', '-', InstructionType::JUMP),
        OpcodeTableEntry(0x29, "ADD HL,HL", 1, 8, 0, '-', '0', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x2A, "LD A,(HL+)", 1, 8, 0, '-', '-', '-', '-', InstructionType::LOAD), // LDI A,(HL)
        OpcodeTableEntry(0x2B, "DEC HL", 1, 8, 0, '-', '-', '-', '-', InstructionType::ALU),
        OpcodeTableEntry(0x2C, "INC L", 1, 4, 0, 'Z', '0', 'H', '-', InstructionType::ALU),
        OpcodeTableEntry(0x2D, "DEC L", 1, 4, 0, 'Z', '1', 'H', '-', InstructionType::ALU),
        OpcodeTableEntry(0x2E, "LD L,d8", 2, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x2F, "CPL", 1, 4, 0, '-', '1', '1', '-', InstructionType::ALU),

        // 0x3X
        OpcodeTableEntry(0x30, "JR NC,r8", 2, 12, 8, '-', '-', '-', '-', InstructionType::JUMP),
        OpcodeTableEntry(0x31, "LD SP,d16", 3, 12, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x32, "LD (HL-),A", 1, 8, 0, '-', '-', '-', '-', InstructionType::LOAD), // LDD (HL),A
        OpcodeTableEntry(0x33, "INC SP", 1, 8, 0, '-', '-', '-', '-', InstructionType::ALU),
        OpcodeTableEntry(0x34, "INC (HL)", 1, 12, 0, 'Z', '0', 'H', '-', InstructionType::ALU),
        OpcodeTableEntry(0x35, "DEC (HL)", 1, 12, 0, 'Z', '1', 'H', '-', InstructionType::ALU),
        OpcodeTableEntry(0x36, "LD (HL),d8", 2, 12, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x37, "SCF", 1, 4, 0, '-', '0', '0', '1', InstructionType::ALU),
        OpcodeTableEntry(0x38, "JR C,r8", 2, 12, 8, '-', '-', '-', '-', InstructionType::JUMP),
        OpcodeTableEntry(0x39, "ADD HL,SP", 1, 8, 0, '-', '0', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x3A, "LD A,(HL-)", 1, 8, 0, '-', '-', '-', '-', InstructionType::LOAD), // LDD A,(HL)
        OpcodeTableEntry(0x3B, "DEC SP", 1, 8, 0, '-', '-', '-', '-', InstructionType::ALU),
        OpcodeTableEntry(0x3C, "INC A", 1, 4, 0, 'Z', '0', 'H', '-', InstructionType::ALU),
        OpcodeTableEntry(0x3D, "DEC A", 1, 4, 0, 'Z', '1', 'H', '-', InstructionType::ALU),
        OpcodeTableEntry(0x3E, "LD A,d8", 2, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x3F, "CCF", 1, 4, 0, '-', '0', '0', 'C', InstructionType::ALU),

        // 0x4X
        OpcodeTableEntry(0x40, "LD B,B", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x41, "LD B,C", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x42, "LD B,D", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x43, "LD B,E", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x44, "LD B,H", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x45, "LD B,L", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x46, "LD B,(HL)", 1, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x47, "LD B,A", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x48, "LD C,B", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x49, "LD C,C", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x4A, "LD C,D", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x4B, "LD C,E", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x4C, "LD C,H", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x4D, "LD C,L", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x4E, "LD C,(HL)", 1, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x4F, "LD C,A", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),

        // 0x5X
        OpcodeTableEntry(0x50, "LD D,B", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x51, "LD D,C", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x52, "LD D,D", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x53, "LD D,E", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x54, "LD D,H", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x55, "LD D,L", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x56, "LD D,(HL)", 1, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x57, "LD D,A", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x58, "LD E,B", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x59, "LD E,C", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x5A, "LD E,D", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x5B, "LD E,E", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x5C, "LD E,H", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x5D, "LD E,L", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x5E, "LD E,(HL)", 1, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x5F, "LD E,A", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),

        // 0x6X
        OpcodeTableEntry(0x60, "LD H,B", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x61, "LD H,C", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x62, "LD H,D", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x63, "LD H,E", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x64, "LD H,H", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x65, "LD H,L", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x66, "LD H,(HL)", 1, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x67, "LD H,A", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x68, "LD L,B", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x69, "LD L,C", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x6A, "LD L,D", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x6B, "LD L,E", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x6C, "LD L,H", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x6D, "LD L,L", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x6E, "LD L,(HL)", 1, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x6F, "LD L,A", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),

        // 0x7X
        OpcodeTableEntry(0x70, "LD (HL),B", 1, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x71, "LD (HL),C", 1, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x72, "LD (HL),D", 1, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x73, "LD (HL),E", 1, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x74, "LD (HL),H", 1, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x75, "LD (HL),L", 1, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x76, "HALT", 1, 4, 0, '-', '-', '-', '-', InstructionType::CONTROL),
        OpcodeTableEntry(0x77, "LD (HL),A", 1, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x78, "LD A,B", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x79, "LD A,C", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x7A, "LD A,D", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x7B, "LD A,E", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x7C, "LD A,H", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x7D, "LD A,L", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x7E, "LD A,(HL)", 1, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0x7F, "LD A,A", 1, 4, 0, '-', '-', '-', '-', InstructionType::LOAD),

        // 0x8X
        OpcodeTableEntry(0x80, "ADD A,B", 1, 4, 0, 'Z', '0', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x81, "ADD A,C", 1, 4, 0, 'Z', '0', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x82, "ADD A,D", 1, 4, 0, 'Z', '0', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x83, "ADD A,E", 1, 4, 0, 'Z', '0', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x84, "ADD A,H", 1, 4, 0, 'Z', '0', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x85, "ADD A,L", 1, 4, 0, 'Z', '0', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x86, "ADD A,(HL)", 1, 8, 0, 'Z', '0', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x87, "ADD A,A", 1, 4, 0, 'Z', '0', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x88, "ADC A,B", 1, 4, 0, 'Z', '0', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x89, "ADC A,C", 1, 4, 0, 'Z', '0', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x8A, "ADC A,D", 1, 4, 0, 'Z', '0', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x8B, "ADC A,E", 1, 4, 0, 'Z', '0', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x8C, "ADC A,H", 1, 4, 0, 'Z', '0', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x8D, "ADC A,L", 1, 4, 0, 'Z', '0', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x8E, "ADC A,(HL)", 1, 8, 0, 'Z', '0', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x8F, "ADC A,A", 1, 4, 0, 'Z', '0', 'H', 'C', InstructionType::ALU),

        // 0x9X
        OpcodeTableEntry(0x90, "SUB B", 1, 4, 0, 'Z', '1', 'H', 'C', InstructionType::ALU), // SUB A,B
        OpcodeTableEntry(0x91, "SUB C", 1, 4, 0, 'Z', '1', 'H', 'C', InstructionType::ALU), // SUB A,C
        OpcodeTableEntry(0x92, "SUB D", 1, 4, 0, 'Z', '1', 'H', 'C', InstructionType::ALU), // SUB A,D
        OpcodeTableEntry(0x93, "SUB E", 1, 4, 0, 'Z', '1', 'H', 'C', InstructionType::ALU), // SUB A,E
        OpcodeTableEntry(0x94, "SUB H", 1, 4, 0, 'Z', '1', 'H', 'C', InstructionType::ALU), // SUB A,H
        OpcodeTableEntry(0x95, "SUB L", 1, 4, 0, 'Z', '1', 'H', 'C', InstructionType::ALU), // SUB A,L
        OpcodeTableEntry(0x96, "SUB (HL)", 1, 8, 0, 'Z', '1', 'H', 'C', InstructionType::ALU), // SUB A,(HL)
        OpcodeTableEntry(0x97, "SUB A", 1, 4, 0, 'Z', '1', 'H', 'C', InstructionType::ALU),    // SUB A,A
        OpcodeTableEntry(0x98, "SBC A,B", 1, 4, 0, 'Z', '1', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x99, "SBC A,C", 1, 4, 0, 'Z', '1', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x9A, "SBC A,D", 1, 4, 0, 'Z', '1', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x9B, "SBC A,E", 1, 4, 0, 'Z', '1', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x9C, "SBC A,H", 1, 4, 0, 'Z', '1', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x9D, "SBC A,L", 1, 4, 0, 'Z', '1', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x9E, "SBC A,(HL)", 1, 8, 0, 'Z', '1', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0x9F, "SBC A,A", 1, 4, 0, 'Z', '1', 'H', 'C', InstructionType::ALU),

        // 0xAX
        OpcodeTableEntry(0xA0, "AND B", 1, 4, 0, 'Z', '0', '1', '0', InstructionType::ALU), // AND A,B
        OpcodeTableEntry(0xA1, "AND C", 1, 4, 0, 'Z', '0', '1', '0', InstructionType::ALU), // AND A,C
        OpcodeTableEntry(0xA2, "AND D", 1, 4, 0, 'Z', '0', '1', '0', InstructionType::ALU), // AND A,D
        OpcodeTableEntry(0xA3, "AND E", 1, 4, 0, 'Z', '0', '1', '0', InstructionType::ALU), // AND A,E
        OpcodeTableEntry(0xA4, "AND H", 1, 4, 0, 'Z', '0', '1', '0', InstructionType::ALU), // AND A,H
        OpcodeTableEntry(0xA5, "AND L", 1, 4, 0, 'Z', '0', '1', '0', InstructionType::ALU), // AND A,L
        OpcodeTableEntry(0xA6, "AND (HL)", 1, 8, 0, 'Z', '0', '1', '0', InstructionType::ALU),// AND A,(HL)
        OpcodeTableEntry(0xA7, "AND A", 1, 4, 0, 'Z', '0', '1', '0', InstructionType::ALU),   // AND A,A
        OpcodeTableEntry(0xA8, "XOR B", 1, 4, 0, 'Z', '0', '0', '0', InstructionType::ALU), // XOR A,B
        OpcodeTableEntry(0xA9, "XOR C", 1, 4, 0, 'Z', '0', '0', '0', InstructionType::ALU), // XOR A,C
        OpcodeTableEntry(0xAA, "XOR D", 1, 4, 0, 'Z', '0', '0', '0', InstructionType::ALU), // XOR A,D
        OpcodeTableEntry(0xAB, "XOR E", 1, 4, 0, 'Z', '0', '0', '0', InstructionType::ALU), // XOR A,E
        OpcodeTableEntry(0xAC, "XOR H", 1, 4, 0, 'Z', '0', '0', '0', InstructionType::ALU), // XOR A,H
        OpcodeTableEntry(0xAD, "XOR L", 1, 4, 0, 'Z', '0', '0', '0', InstructionType::ALU), // XOR A,L
        OpcodeTableEntry(0xAE, "XOR (HL)", 1, 8, 0, 'Z', '0', '0', '0', InstructionType::ALU),// XOR A,(HL)
        OpcodeTableEntry(0xAF, "XOR A", 1, 4, 0, 'Z', '0', '0', '0', InstructionType::ALU),   // XOR A,A

        // 0xBX
        OpcodeTableEntry(0xB0, "OR B", 1, 4, 0, 'Z', '0', '0', '0', InstructionType::ALU),  // OR A,B
        OpcodeTableEntry(0xB1, "OR C", 1, 4, 0, 'Z', '0', '0', '0', InstructionType::ALU),  // OR A,C
        OpcodeTableEntry(0xB2, "OR D", 1, 4, 0, 'Z', '0', '0', '0', InstructionType::ALU),  // OR A,D
        OpcodeTableEntry(0xB3, "OR E", 1, 4, 0, 'Z', '0', '0', '0', InstructionType::ALU),  // OR A,E
        OpcodeTableEntry(0xB4, "OR H", 1, 4, 0, 'Z', '0', '0', '0', InstructionType::ALU),  // OR A,H
        OpcodeTableEntry(0xB5, "OR L", 1, 4, 0, 'Z', '0', '0', '0', InstructionType::ALU),  // OR A,L
        OpcodeTableEntry(0xB6, "OR (HL)", 1, 8, 0, 'Z', '0', '0', '0', InstructionType::ALU), // OR A,(HL)
        OpcodeTableEntry(0xB7, "OR A", 1, 4, 0, 'Z', '0', '0', '0', InstructionType::ALU),    // OR A,A
        OpcodeTableEntry(0xB8, "CP B", 1, 4, 0, 'Z', '1', 'H', 'C', InstructionType::ALU),  // CP A,B
        OpcodeTableEntry(0xB9, "CP C", 1, 4, 0, 'Z', '1', 'H', 'C', InstructionType::ALU),  // CP A,C
        OpcodeTableEntry(0xBA, "CP D", 1, 4, 0, 'Z', '1', 'H', 'C', InstructionType::ALU),  // CP A,D
        OpcodeTableEntry(0xBB, "CP E", 1, 4, 0, 'Z', '1', 'H', 'C', InstructionType::ALU),  // CP A,E
        OpcodeTableEntry(0xBC, "CP H", 1, 4, 0, 'Z', '1', 'H', 'C', InstructionType::ALU),  // CP A,H
        OpcodeTableEntry(0xBD, "CP L", 1, 4, 0, 'Z', '1', 'H', 'C', InstructionType::ALU),  // CP A,L
        OpcodeTableEntry(0xBE, "CP (HL)", 1, 8, 0, 'Z', '1', 'H', 'C', InstructionType::ALU), // CP A,(HL)
        OpcodeTableEntry(0xBF, "CP A", 1, 4, 0, 'Z', '1', 'H', 'C', InstructionType::ALU),    // CP A,A

        // 0xCX
        OpcodeTableEntry(0xC0, "RET NZ", 1, 20, 8, '-', '-', '-', '-', InstructionType::JUMP),
        OpcodeTableEntry(0xC1, "POP BC", 1, 12, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0xC2, "JP NZ,a16", 3, 16, 12, '-', '-', '-', '-', InstructionType::JUMP),
        OpcodeTableEntry(0xC3, "JP a16", 3, 16, 0, '-', '-', '-', '-', InstructionType::JUMP),
        OpcodeTableEntry(0xC4, "CALL NZ,a16", 3, 24, 12, '-', '-', '-', '-', InstructionType::JUMP),
        OpcodeTableEntry(0xC5, "PUSH BC", 1, 16, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0xC6, "ADD A,d8", 2, 8, 0, 'Z', '0', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0xC7, "RST 00H", 1, 16, 0, '-', '-', '-', '-', InstructionType::JUMP),
        OpcodeTableEntry(0xC8, "RET Z", 1, 20, 8, '-', '-', '-', '-', InstructionType::JUMP),
        OpcodeTableEntry(0xC9, "RET", 1, 16, 0, '-', '-', '-', '-', InstructionType::JUMP),
        OpcodeTableEntry(0xCA, "JP Z,a16", 3, 16, 12, '-', '-', '-', '-', InstructionType::JUMP),
        OpcodeTableEntry(0xCB, "PREFIX CB", 1, 4, 0, '-', '-', '-', '-', InstructionType::BIT), // CB Prefix
        OpcodeTableEntry(0xCC, "CALL Z,a16", 3, 24, 12, '-', '-', '-', '-', InstructionType::JUMP),
        OpcodeTableEntry(0xCD, "CALL a16", 3, 24, 0, '-', '-', '-', '-', InstructionType::JUMP),
        OpcodeTableEntry(0xCE, "ADC A,d8", 2, 8, 0, 'Z', '0', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0xCF, "RST 08H", 1, 16, 0, '-', '-', '-', '-', InstructionType::JUMP),

        // 0xDX
        OpcodeTableEntry(0xD0, "RET NC", 1, 20, 8, '-', '-', '-', '-', InstructionType::JUMP),
        OpcodeTableEntry(0xD1, "POP DE", 1, 12, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0xD2, "JP NC,a16", 3, 16, 12, '-', '-', '-', '-', InstructionType::JUMP),
        OpcodeTableEntry(0xD3, "UNDEFINED", 1, 4, 0, '-', '-', '-', '-', InstructionType::UNKNOWN), // Illegal/Undefined
        OpcodeTableEntry(0xD4, "CALL NC,a16", 3, 24, 12, '-', '-', '-', '-', InstructionType::JUMP),
        OpcodeTableEntry(0xD5, "PUSH DE", 1, 16, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0xD6, "SUB d8", 2, 8, 0, 'Z', '1', 'H', 'C', InstructionType::ALU), // SUB A,d8
        OpcodeTableEntry(0xD7, "RST 10H", 1, 16, 0, '-', '-', '-', '-', InstructionType::JUMP),
        OpcodeTableEntry(0xD8, "RET C", 1, 20, 8, '-', '-', '-', '-', InstructionType::JUMP),
        OpcodeTableEntry(0xD9, "RETI", 1, 16, 0, '-', '-', '-', '-', InstructionType::JUMP),
        OpcodeTableEntry(0xDA, "JP C,a16", 3, 16, 12, '-', '-', '-', '-', InstructionType::JUMP),
        OpcodeTableEntry(0xDB, "UNDEFINED", 1, 4, 0, '-', '-', '-', '-', InstructionType::UNKNOWN), // Illegal/Undefined
        OpcodeTableEntry(0xDC, "CALL C,a16", 3, 24, 12, '-', '-', '-', '-', InstructionType::JUMP),
        OpcodeTableEntry(0xDD, "UNDEFINED", 1, 4, 0, '-', '-', '-', '-', InstructionType::UNKNOWN), // Illegal/Undefined
        OpcodeTableEntry(0xDE, "SBC A,d8", 2, 8, 0, 'Z', '1', 'H', 'C', InstructionType::ALU),
        OpcodeTableEntry(0xDF, "RST 18H", 1, 16, 0, '-', '-', '-', '-', InstructionType::JUMP),

        // 0xEX
        OpcodeTableEntry(0xE0, "LDH (a8),A", 2, 12, 0, '-', '-', '-', '-', InstructionType::LOAD), // LD ($FF00+a8),A
        OpcodeTableEntry(0xE1, "POP HL", 1, 12, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0xE2, "LD (C),A", 1, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),   // LD ($FF00+C),A - Note: length 1 in some docs, 2 in others for mnemonic. Opcode is 1 byte.
        OpcodeTableEntry(0xE3, "UNDEFINED", 1, 4, 0, '-', '-', '-', '-', InstructionType::UNKNOWN), // Illegal/Undefined
        OpcodeTableEntry(0xE4, "UNDEFINED", 1, 4, 0, '-', '-', '-', '-', InstructionType::UNKNOWN), // Illegal/Undefined
        OpcodeTableEntry(0xE5, "PUSH HL", 1, 16, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0xE6, "AND d8", 2, 8, 0, 'Z', '0', '1', '0', InstructionType::ALU),    // AND A,d8
        OpcodeTableEntry(0xE7, "RST 20H", 1, 16, 0, '-', '-', '-', '-', InstructionType::JUMP),
        OpcodeTableEntry(0xE8, "ADD SP,r8", 2, 16, 0, '0', '0', 'H', 'C', InstructionType::ALU), // r8 is signed
        OpcodeTableEntry(0xE9, "JP (HL)", 1, 4, 0, '-', '-', '-', '-', InstructionType::JUMP),   // JP HL
        OpcodeTableEntry(0xEA, "LD (a16),A", 3, 16, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0xEB, "UNDEFINED", 1, 4, 0, '-', '-', '-', '-', InstructionType::UNKNOWN), // Illegal/Undefined
        OpcodeTableEntry(0xEC, "UNDEFINED", 1, 4, 0, '-', '-', '-', '-', InstructionType::UNKNOWN), // Illegal/Undefined
        OpcodeTableEntry(0xED, "UNDEFINED", 1, 4, 0, '-', '-', '-', '-', InstructionType::UNKNOWN), // Illegal/Undefined
        OpcodeTableEntry(0xEE, "XOR d8", 2, 8, 0, 'Z', '0', '0', '0', InstructionType::ALU),    // XOR A,d8
        OpcodeTableEntry(0xEF, "RST 28H", 1, 16, 0, '-', '-', '-', '-', InstructionType::JUMP),

        // 0xFX
        OpcodeTableEntry(0xF0, "LDH A,(a8)", 2, 12, 0, '-', '-', '-', '-', InstructionType::LOAD), // LD A,($FF00+a8)
        OpcodeTableEntry(0xF1, "POP AF", 1, 12, 0, 'Z', 'N', 'H', 'C', InstructionType::LOAD), // Flags from popped value
        OpcodeTableEntry(0xF2, "LD A,(C)", 1, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),   // LD A,($FF00+C) - Note: length 1 in some docs, 2 in others for mnemonic. Opcode is 1 byte.
        OpcodeTableEntry(0xF3, "DI", 1, 4, 0, '-', '-', '-', '-', InstructionType::CONTROL),
        OpcodeTableEntry(0xF4, "UNDEFINED", 1, 4, 0, '-', '-', '-', '-', InstructionType::UNKNOWN), // Illegal/Undefined
        OpcodeTableEntry(0xF5, "PUSH AF", 1, 16, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0xF6, "OR d8", 2, 8, 0, 'Z', '0', '0', '0', InstructionType::ALU),     // OR A,d8
        OpcodeTableEntry(0xF7, "RST 30H", 1, 16, 0, '-', '-', '-', '-', InstructionType::JUMP),
        OpcodeTableEntry(0xF8, "LD HL,SP+r8", 2, 12, 0, '0', '0', 'H', 'C', InstructionType::LOAD), // LDHL SP,r8 (r8 is signed)
        OpcodeTableEntry(0xF9, "LD SP,HL", 1, 8, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0xFA, "LD A,(a16)", 3, 16, 0, '-', '-', '-', '-', InstructionType::LOAD),
        OpcodeTableEntry(0xFB, "EI", 1, 4, 0, '-', '-', '-', '-', InstructionType::CONTROL),
        OpcodeTableEntry(0xFC, "UNDEFINED", 1, 4, 0, '-', '-', '-', '-', InstructionType::UNKNOWN), // Illegal/Undefined
        OpcodeTableEntry(0xFD, "UNDEFINED", 1, 4, 0, '-', '-', '-', '-', InstructionType::UNKNOWN), // Illegal/Undefined
        OpcodeTableEntry(0xFE, "CP d8", 2, 8, 0, 'Z', '1', 'H', 'C', InstructionType::ALU),     // CP A,d8
        OpcodeTableEntry(0xFF, "RST 38H", 1, 16, 0, '-', '-', '-', '-', InstructionType::JUMP)
    }};


    const std::array<OpcodeTableEntry, 256> CB_OPCODE_TABLE = {{
        // 0x0X: Rotates and Shifts
        OpcodeTableEntry(0x00, "RLC B",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x01, "RLC C",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x02, "RLC D",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x03, "RLC E",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x04, "RLC H",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x05, "RLC L",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x06, "RLC (HL)", 2, 16, 0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x07, "RLC A",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x08, "RRC B",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x09, "RRC C",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x0A, "RRC D",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x0B, "RRC E",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x0C, "RRC H",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x0D, "RRC L",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x0E, "RRC (HL)", 2, 16, 0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x0F, "RRC A",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),

        // 0x1X: Rotates and Shifts
        OpcodeTableEntry(0x10, "RL B",     2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x11, "RL C",     2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x12, "RL D",     2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x13, "RL E",     2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x14, "RL H",     2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x15, "RL L",     2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x16, "RL (HL)",  2, 16, 0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x17, "RL A",     2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x18, "RR B",     2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x19, "RR C",     2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x1A, "RR D",     2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x1B, "RR E",     2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x1C, "RR H",     2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x1D, "RR L",     2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x1E, "RR (HL)",  2, 16, 0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x1F, "RR A",     2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),

        // 0x2X: Shifts
        OpcodeTableEntry(0x20, "SLA B",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x21, "SLA C",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x22, "SLA D",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x23, "SLA E",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x24, "SLA H",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x25, "SLA L",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x26, "SLA (HL)", 2, 16, 0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x27, "SLA A",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x28, "SRA B",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x29, "SRA C",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x2A, "SRA D",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x2B, "SRA E",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x2C, "SRA H",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x2D, "SRA L",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x2E, "SRA (HL)", 2, 16, 0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x2F, "SRA A",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),

        // 0x3X: SWAP and SRL
        OpcodeTableEntry(0x30, "SWAP B",   2, 8,  0, 'Z', '0', '0', '0', InstructionType::BIT),
        OpcodeTableEntry(0x31, "SWAP C",   2, 8,  0, 'Z', '0', '0', '0', InstructionType::BIT),
        OpcodeTableEntry(0x32, "SWAP D",   2, 8,  0, 'Z', '0', '0', '0', InstructionType::BIT),
        OpcodeTableEntry(0x33, "SWAP E",   2, 8,  0, 'Z', '0', '0', '0', InstructionType::BIT),
        OpcodeTableEntry(0x34, "SWAP H",   2, 8,  0, 'Z', '0', '0', '0', InstructionType::BIT),
        OpcodeTableEntry(0x35, "SWAP L",   2, 8,  0, 'Z', '0', '0', '0', InstructionType::BIT),
        OpcodeTableEntry(0x36, "SWAP (HL)",2, 16, 0, 'Z', '0', '0', '0', InstructionType::BIT),
        OpcodeTableEntry(0x37, "SWAP A",   2, 8,  0, 'Z', '0', '0', '0', InstructionType::BIT),
        OpcodeTableEntry(0x38, "SRL B",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x39, "SRL C",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x3A, "SRL D",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x3B, "SRL E",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x3C, "SRL H",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x3D, "SRL L",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x3E, "SRL (HL)", 2, 16, 0, 'Z', '0', '0', 'C', InstructionType::BIT),
        OpcodeTableEntry(0x3F, "SRL A",    2, 8,  0, 'Z', '0', '0', 'C', InstructionType::BIT),

        // 0x4X: BIT 0,r to BIT 1,r
        OpcodeTableEntry(0x40, "BIT 0,B",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x41, "BIT 0,C",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x42, "BIT 0,D",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x43, "BIT 0,E",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x44, "BIT 0,H",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x45, "BIT 0,L",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x46, "BIT 0,(HL)",2,12, 0, 'Z', '0', '1', '-', InstructionType::BIT), // BIT (HL) is 12 cycles
        OpcodeTableEntry(0x47, "BIT 0,A",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x48, "BIT 1,B",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x49, "BIT 1,C",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x4A, "BIT 1,D",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x4B, "BIT 1,E",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x4C, "BIT 1,H",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x4D, "BIT 1,L",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x4E, "BIT 1,(HL)",2,12, 0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x4F, "BIT 1,A",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),

        // 0x5X: BIT 2,r to BIT 3,r
        OpcodeTableEntry(0x50, "BIT 2,B",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x51, "BIT 2,C",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x52, "BIT 2,D",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x53, "BIT 2,E",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x54, "BIT 2,H",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x55, "BIT 2,L",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x56, "BIT 2,(HL)",2,12, 0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x57, "BIT 2,A",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x58, "BIT 3,B",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x59, "BIT 3,C",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x5A, "BIT 3,D",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x5B, "BIT 3,E",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x5C, "BIT 3,H",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x5D, "BIT 3,L",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x5E, "BIT 3,(HL)",2,12, 0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x5F, "BIT 3,A",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),

        // 0x6X: BIT 4,r to BIT 5,r
        OpcodeTableEntry(0x60, "BIT 4,B",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x61, "BIT 4,C",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x62, "BIT 4,D",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x63, "BIT 4,E",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x64, "BIT 4,H",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x65, "BIT 4,L",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x66, "BIT 4,(HL)",2,12, 0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x67, "BIT 4,A",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x68, "BIT 5,B",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x69, "BIT 5,C",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x6A, "BIT 5,D",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x6B, "BIT 5,E",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x6C, "BIT 5,H",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x6D, "BIT 5,L",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x6E, "BIT 5,(HL)",2,12, 0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x6F, "BIT 5,A",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),

        // 0x7X: BIT 6,r to BIT 7,r
        OpcodeTableEntry(0x70, "BIT 6,B",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x71, "BIT 6,C",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x72, "BIT 6,D",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x73, "BIT 6,E",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x74, "BIT 6,H",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x75, "BIT 6,L",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x76, "BIT 6,(HL)",2,12, 0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x77, "BIT 6,A",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x78, "BIT 7,B",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x79, "BIT 7,C",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x7A, "BIT 7,D",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x7B, "BIT 7,E",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x7C, "BIT 7,H",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x7D, "BIT 7,L",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x7E, "BIT 7,(HL)",2,12, 0, 'Z', '0', '1', '-', InstructionType::BIT),
        OpcodeTableEntry(0x7F, "BIT 7,A",  2, 8,  0, 'Z', '0', '1', '-', InstructionType::BIT),

        // 0x8X: RES 0,r to RES 1,r (Flags: ----)
        OpcodeTableEntry(0x80, "RES 0,B",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x81, "RES 0,C",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x82, "RES 0,D",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x83, "RES 0,E",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x84, "RES 0,H",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x85, "RES 0,L",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x86, "RES 0,(HL)",2,16, 0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x87, "RES 0,A",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x88, "RES 1,B",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x89, "RES 1,C",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x8A, "RES 1,D",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x8B, "RES 1,E",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x8C, "RES 1,H",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x8D, "RES 1,L",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x8E, "RES 1,(HL)",2,16, 0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x8F, "RES 1,A",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),

        // 0x9X: RES 2,r to RES 3,r
        OpcodeTableEntry(0x90, "RES 2,B",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x91, "RES 2,C",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x92, "RES 2,D",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x93, "RES 2,E",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x94, "RES 2,H",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x95, "RES 2,L",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x96, "RES 2,(HL)",2,16, 0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x97, "RES 2,A",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x98, "RES 3,B",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x99, "RES 3,C",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x9A, "RES 3,D",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x9B, "RES 3,E",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x9C, "RES 3,H",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x9D, "RES 3,L",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x9E, "RES 3,(HL)",2,16, 0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0x9F, "RES 3,A",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),

        // 0xAX: RES 4,r to RES 5,r
        OpcodeTableEntry(0xA0, "RES 4,B",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xA1, "RES 4,C",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xA2, "RES 4,D",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xA3, "RES 4,E",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xA4, "RES 4,H",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xA5, "RES 4,L",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xA6, "RES 4,(HL)",2,16, 0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xA7, "RES 4,A",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xA8, "RES 5,B",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xA9, "RES 5,C",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xAA, "RES 5,D",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xAB, "RES 5,E",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xAC, "RES 5,H",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xAD, "RES 5,L",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xAE, "RES 5,(HL)",2,16, 0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xAF, "RES 5,A",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),

        // 0xBX: RES 6,r to RES 7,r
        OpcodeTableEntry(0xB0, "RES 6,B",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xB1, "RES 6,C",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xB2, "RES 6,D",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xB3, "RES 6,E",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xB4, "RES 6,H",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xB5, "RES 6,L",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xB6, "RES 6,(HL)",2,16, 0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xB7, "RES 6,A",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xB8, "RES 7,B",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xB9, "RES 7,C",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xBA, "RES 7,D",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xBB, "RES 7,E",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xBC, "RES 7,H",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xBD, "RES 7,L",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xBE, "RES 7,(HL)",2,16, 0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xBF, "RES 7,A",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),

        // 0xCX: SET 0,r to SET 1,r
        OpcodeTableEntry(0xC0, "SET 0,B",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xC1, "SET 0,C",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xC2, "SET 0,D",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xC3, "SET 0,E",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xC4, "SET 0,H",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xC5, "SET 0,L",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xC6, "SET 0,(HL)",2,16, 0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xC7, "SET 0,A",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xC8, "SET 1,B",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xC9, "SET 1,C",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xCA, "SET 1,D",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xCB, "SET 1,E",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xCC, "SET 1,H",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xCD, "SET 1,L",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xCE, "SET 1,(HL)",2,16, 0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xCF, "SET 1,A",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),

        // 0xDX: SET 2,r to SET 3,r
        OpcodeTableEntry(0xD0, "SET 2,B",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xD1, "SET 2,C",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xD2, "SET 2,D",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xD3, "SET 2,E",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xD4, "SET 2,H",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xD5, "SET 2,L",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xD6, "SET 2,(HL)",2,16, 0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xD7, "SET 2,A",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xD8, "SET 3,B",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xD9, "SET 3,C",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xDA, "SET 3,D",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xDB, "SET 3,E",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xDC, "SET 3,H",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xDD, "SET 3,L",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xDE, "SET 3,(HL)",2,16, 0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xDF, "SET 3,A",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),

        // 0xEX: SET 4,r to SET 5,r
        OpcodeTableEntry(0xE0, "SET 4,B",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xE1, "SET 4,C",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xE2, "SET 4,D",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xE3, "SET 4,E",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xE4, "SET 4,H",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xE5, "SET 4,L",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xE6, "SET 4,(HL)",2,16, 0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xE7, "SET 4,A",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xE8, "SET 5,B",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xE9, "SET 5,C",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xEA, "SET 5,D",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xEB, "SET 5,E",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xEC, "SET 5,H",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xED, "SET 5,L",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xEE, "SET 5,(HL)",2,16, 0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xEF, "SET 5,A",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),

        // 0xFX: SET 6,r to SET 7,r
        OpcodeTableEntry(0xF0, "SET 6,B",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xF1, "SET 6,C",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xF2, "SET 6,D",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xF3, "SET 6,E",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xF4, "SET 6,H",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xF5, "SET 6,L",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xF6, "SET 6,(HL)",2,16, 0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xF7, "SET 6,A",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xF8, "SET 7,B",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xF9, "SET 7,C",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xFA, "SET 7,D",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xFB, "SET 7,E",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xFC, "SET 7,H",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xFD, "SET 7,L",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xFE, "SET 7,(HL)",2,16, 0, '-', '-', '-', '-', InstructionType::BIT),
        OpcodeTableEntry(0xFF, "SET 7,A",  2, 8,  0, '-', '-', '-', '-', InstructionType::BIT)
    }};


    // Helper functions updated to use FULL_OPCODE_TABLE
    inline InstructionType getInstructionType(BYTE opcode) {
        return FULL_OPCODE_TABLE[opcode].type;
    }

    // Returns primary cycle count. Caller must check conditional_cycles if needed.
    inline BYTE getInstructionCycles(BYTE opcode) {
        return FULL_OPCODE_TABLE[opcode].duration_cycles;
    }
    
    // Returns conditional cycle count if applicable, otherwise 0.
    inline BYTE getInstructionConditionalCycles(BYTE opcode) {
        return FULL_OPCODE_TABLE[opcode].duration_cycles_conditional;
    }

    inline const char* getInstructionMnemonic(BYTE opcode) { // Renamed from getInstructionDescription
        return FULL_OPCODE_TABLE[opcode].mnemonic;
    }

    inline BYTE getInstructionLength(BYTE opcode) {
        return FULL_OPCODE_TABLE[opcode].length_in_bytes;
    }

    const int INTERRUPT_CYCLES = 20;
    const int HALT_CYCLES = 4;
    const int UNKNOWN_OPCODE_CYCLES = 4; // Default cycles for unknown opcodes
    // ALU Operation Cycles
    constexpr BYTE ALU_REGULAR_CYCLES = 4;
    constexpr BYTE ALU_MEMORY_CYCLES = 8;
    inline const char* instructionTypeToString(InstructionType type) {
        switch (type) {
            case InstructionType::ALU: return "ALU";
            case InstructionType::LOAD: return "LOAD";
            case InstructionType::JUMP: return "JUMP";
            case InstructionType::BIT: return "BIT";
            case InstructionType::CONTROL: return "CONTROL";
            case InstructionType::UNKNOWN: return "UNKNOWN";
            default: return "INVALID_TYPE";
        }
    }

    // ALU Opcodes
    namespace ALU {
        // ADD A,r opcodes
        constexpr BYTE ADD_A_B = 0x80;
        constexpr BYTE ADD_A_C = 0x81;
        constexpr BYTE ADD_A_D = 0x82;
        constexpr BYTE ADD_A_E = 0x83;
        constexpr BYTE ADD_A_H = 0x84;
        constexpr BYTE ADD_A_L = 0x85;
        constexpr BYTE ADD_A_HL = 0x86;
        constexpr BYTE ADD_A_A = 0x87;

        // SUB r opcodes
        constexpr BYTE SUB_B = 0x90;
        constexpr BYTE SUB_C = 0x91;
        constexpr BYTE SUB_D = 0x92;
        constexpr BYTE SUB_E = 0x93;
        constexpr BYTE SUB_H = 0x94;
        constexpr BYTE SUB_L = 0x95;
        constexpr BYTE SUB_HL = 0x96;
        constexpr BYTE SUB_A = 0x97;

        // CP r opcodes
        constexpr BYTE CP_B = 0xB8;
        constexpr BYTE CP_C = 0xB9;
        constexpr BYTE CP_D = 0xBA;
        constexpr BYTE CP_E = 0xBB;
        constexpr BYTE CP_H = 0xBC;
        constexpr BYTE CP_L = 0xBD;
        constexpr BYTE CP_HL = 0xBE;
        constexpr BYTE CP_A = 0xBF;
        constexpr BYTE CP_N = 0xFE;

        // ADD A,n opcode
        constexpr BYTE ADD_A_N = 0xC6;

        // ADC A,r opcodes
        constexpr BYTE ADC_A_B = 0x88;
        constexpr BYTE ADC_A_C = 0x89;
        constexpr BYTE ADC_A_D = 0x8A;
        constexpr BYTE ADC_A_E = 0x8B;
        constexpr BYTE ADC_A_H = 0x8C;
        constexpr BYTE ADC_A_L = 0x8D;
        constexpr BYTE ADC_A_HL = 0x8E;
        constexpr BYTE ADC_A_A = 0x8F;

        // ADC A,n opcode
        constexpr BYTE ADC_A_N = 0xCE;

        // SUB n opcode
        constexpr BYTE SUB_N = 0xD6;

        // SBC A,r opcodes
        constexpr BYTE SBC_A_B = 0x98;
        constexpr BYTE SBC_A_C = 0x99;
        constexpr BYTE SBC_A_D = 0x9A;
        constexpr BYTE SBC_A_E = 0x9B;
        constexpr BYTE SBC_A_H = 0x9C;
        constexpr BYTE SBC_A_L = 0x9D;
        constexpr BYTE SBC_A_HL = 0x9E;
        constexpr BYTE SBC_A_A = 0x9F;

        // SBC A,n opcode
        constexpr BYTE SBC_A_N = 0xDE;

        // AND r opcodes
        constexpr BYTE AND_B = 0xA0;
        constexpr BYTE AND_C = 0xA1;
        constexpr BYTE AND_D = 0xA2;
        constexpr BYTE AND_E = 0xA3;
        constexpr BYTE AND_H = 0xA4;
        constexpr BYTE AND_L = 0xA5;
        constexpr BYTE AND_HL = 0xA6;
        constexpr BYTE AND_A = 0xA7;

        // AND n opcode
        constexpr BYTE AND_N = 0xE6;

        // OR r opcodes
        constexpr BYTE OR_B = 0xB0;
        constexpr BYTE OR_C = 0xB1;
        constexpr BYTE OR_D = 0xB2;
        constexpr BYTE OR_E = 0xB3;
        constexpr BYTE OR_H = 0xB4;
        constexpr BYTE OR_L = 0xB5;
        constexpr BYTE OR_HL = 0xB6;
        constexpr BYTE OR_A = 0xB7;

        // OR n opcode
        constexpr BYTE OR_N = 0xF6;

        // XOR r opcodes
        constexpr BYTE XOR_B = 0xA8;
        constexpr BYTE XOR_C = 0xA9;
        constexpr BYTE XOR_D = 0xAA;
        constexpr BYTE XOR_E = 0xAB;
        constexpr BYTE XOR_H = 0xAC;
        constexpr BYTE XOR_L = 0xAD;
        constexpr BYTE XOR_HL = 0xAE;
        constexpr BYTE XOR_A = 0xAF;
 
         // XOR n opcode
         constexpr BYTE XOR_N = 0xEE;
          // INC r opcodes
        constexpr BYTE INC_B = 0x04;
        constexpr BYTE INC_C = 0x0C;
        constexpr BYTE INC_D = 0x14;
        constexpr BYTE INC_E = 0x1C;
        constexpr BYTE INC_H = 0x24;
        constexpr BYTE INC_L = 0x2C;
        constexpr BYTE INC_A = 0x3C;

        // DEC r opcodes
        constexpr BYTE DEC_B = 0x05;
        constexpr BYTE DEC_C = 0x0D;
        constexpr BYTE DEC_D = 0x15;
        constexpr BYTE DEC_E = 0x1D;
        constexpr BYTE DEC_H = 0x25;
        constexpr BYTE DEC_L = 0x2D;
        constexpr BYTE DEC_A = 0x3D;
    }
}