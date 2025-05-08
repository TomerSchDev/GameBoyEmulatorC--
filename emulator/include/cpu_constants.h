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

    struct OpcodeMapping {
        BYTE opcode;
        InstructionType type;
        BYTE cycles;
        const char* description;  // Added description for debugging
    };

    const std::vector<OpcodeMapping> OPCODE_MAP = {
        // Control/Misc Instructions
        {0x00, InstructionType::CONTROL, 4, "NOP"},
        {0x10, InstructionType::CONTROL, 4, "STOP"}, // 0x10 0x00
        {0x27, InstructionType::ALU, 4, "DAA"},
        {0x2F, InstructionType::ALU, 4, "CPL"},
        {0x37, InstructionType::ALU, 4, "SCF"},
        {0x3F, InstructionType::ALU, 4, "CCF"},
        {0x76, InstructionType::CONTROL, 4, "HALT"},
        {0xF3, InstructionType::CONTROL, 4, "DI"},
        {0xFB, InstructionType::CONTROL, 4, "EI"},

        // 8-bit Load Instructions
        // LD r, r'
        {0x40, InstructionType::LOAD, 4, "LD B,B"},
        {0x41, InstructionType::LOAD, 4, "LD B,C"},
        {0x42, InstructionType::LOAD, 4, "LD B,D"},
        {0x43, InstructionType::LOAD, 4, "LD B,E"},
        {0x44, InstructionType::LOAD, 4, "LD B,H"},
        {0x45, InstructionType::LOAD, 4, "LD B,L"},
        {0x46, InstructionType::LOAD, 8, "LD B,(HL)"},
        {0x47, InstructionType::LOAD, 4, "LD B,A"},
        {0x48, InstructionType::LOAD, 4, "LD C,B"},
        {0x49, InstructionType::LOAD, 4, "LD C,C"},
        {0x4A, InstructionType::LOAD, 4, "LD C,D"},
        {0x4B, InstructionType::LOAD, 4, "LD C,E"},
        {0x4C, InstructionType::LOAD, 4, "LD C,H"},
        {0x4D, InstructionType::LOAD, 4, "LD C,L"},
        {0x4E, InstructionType::LOAD, 8, "LD C,(HL)"},
        {0x4F, InstructionType::LOAD, 4, "LD C,A"},
        {0x50, InstructionType::LOAD, 4, "LD D,B"},
        {0x51, InstructionType::LOAD, 4, "LD D,C"},
        {0x52, InstructionType::LOAD, 4, "LD D,D"},
        {0x53, InstructionType::LOAD, 4, "LD D,E"},
        {0x54, InstructionType::LOAD, 4, "LD D,H"},
        {0x55, InstructionType::LOAD, 4, "LD D,L"},
        {0x56, InstructionType::LOAD, 8, "LD D,(HL)"},
        {0x57, InstructionType::LOAD, 4, "LD D,A"},
        {0x58, InstructionType::LOAD, 4, "LD E,B"},
        {0x59, InstructionType::LOAD, 4, "LD E,C"},
        {0x5A, InstructionType::LOAD, 4, "LD E,D"},
        {0x5B, InstructionType::LOAD, 4, "LD E,E"},
        {0x5C, InstructionType::LOAD, 4, "LD E,H"},
        {0x5D, InstructionType::LOAD, 4, "LD E,L"},
        {0x5E, InstructionType::LOAD, 8, "LD E,(HL)"},
        {0x5F, InstructionType::LOAD, 4, "LD E,A"},
        {0x60, InstructionType::LOAD, 4, "LD H,B"},
        {0x61, InstructionType::LOAD, 4, "LD H,C"},
        {0x62, InstructionType::LOAD, 4, "LD H,D"},
        {0x63, InstructionType::LOAD, 4, "LD H,E"},
        {0x64, InstructionType::LOAD, 4, "LD H,H"},
        {0x65, InstructionType::LOAD, 4, "LD H,L"},
        {0x66, InstructionType::LOAD, 8, "LD H,(HL)"},
        {0x67, InstructionType::LOAD, 4, "LD H,A"},
        {0x68, InstructionType::LOAD, 4, "LD L,B"},
        {0x69, InstructionType::LOAD, 4, "LD L,C"},
        {0x6A, InstructionType::LOAD, 4, "LD L,D"},
        {0x6B, InstructionType::LOAD, 4, "LD L,E"},
        {0x6C, InstructionType::LOAD, 4, "LD L,H"},
        {0x6D, InstructionType::LOAD, 4, "LD L,L"},
        {0x6E, InstructionType::LOAD, 8, "LD L,(HL)"},
        {0x6F, InstructionType::LOAD, 4, "LD L,A"},
        {0x70, InstructionType::LOAD, 8, "LD (HL),B"},
        {0x71, InstructionType::LOAD, 8, "LD (HL),C"},
        {0x72, InstructionType::LOAD, 8, "LD (HL),D"},
        {0x73, InstructionType::LOAD, 8, "LD (HL),E"},
        {0x74, InstructionType::LOAD, 8, "LD (HL),H"},
        {0x75, InstructionType::LOAD, 8, "LD (HL),L"},
        // 0x76 is HALT
        {0x77, InstructionType::LOAD, 8, "LD (HL),A"},
        {0x78, InstructionType::LOAD, 4, "LD A,B"},
        {0x79, InstructionType::LOAD, 4, "LD A,C"},
        {0x7A, InstructionType::LOAD, 4, "LD A,D"},
        {0x7B, InstructionType::LOAD, 4, "LD A,E"},
        {0x7C, InstructionType::LOAD, 4, "LD A,H"},
        {0x7D, InstructionType::LOAD, 4, "LD A,L"},
        {0x7E, InstructionType::LOAD, 8, "LD A,(HL)"},
        {0x7F, InstructionType::LOAD, 4, "LD A,A"},

        // LD r, n
        {0x06, InstructionType::LOAD, 8, "LD B,n"},
        {0x0E, InstructionType::LOAD, 8, "LD C,n"},
        {0x16, InstructionType::LOAD, 8, "LD D,n"},
        {0x1E, InstructionType::LOAD, 8, "LD E,n"},
        {0x26, InstructionType::LOAD, 8, "LD H,n"},
        {0x2E, InstructionType::LOAD, 8, "LD L,n"},
        {0x36, InstructionType::LOAD, 12, "LD (HL),n"},
        {0x3E, InstructionType::LOAD, 8, "LD A,n"},

        // LD A, (rr) / LD A, (nn) / LD (rr), A / LD (nn), A
        {0x0A, InstructionType::LOAD, 8, "LD A,(BC)"},
        {0x1A, InstructionType::LOAD, 8, "LD A,(DE)"},
        {0xFA, InstructionType::LOAD, 16, "LD A,(nn)"},
        {0x02, InstructionType::LOAD, 8, "LD (BC),A"},
        {0x12, InstructionType::LOAD, 8, "LD (DE),A"},
        {0xEA, InstructionType::LOAD, 16, "LD (nn),A"},
        
        // LD A,(C) / LD (C),A - I/O
        {0xF2, InstructionType::LOAD, 8, "LD A,(C)"}, // LD A,($FF00+C)
        {0xE2, InstructionType::LOAD, 8, "LD (C),A"}, // LD ($FF00+C),A

        // LDD A,(HL) / LDI A,(HL) / LDD (HL),A / LDI (HL),A
        {0x3A, InstructionType::LOAD, 8, "LDD A,(HL)"}, // LD A,(HL), DEC HL
        {0x2A, InstructionType::LOAD, 8, "LDI A,(HL)"}, // LD A,(HL), INC HL
        {0x32, InstructionType::LOAD, 8, "LDD (HL),A"}, // LD (HL),A, DEC HL
        {0x22, InstructionType::LOAD, 8, "LDI (HL),A"}, // LD (HL),A, INC HL

        // LDH (n),A / LDH A,(n) - I/O
        {0xE0, InstructionType::LOAD, 12, "LDH (n),A"}, // LD ($FF00+n),A
        {0xF0, InstructionType::LOAD, 12, "LDH A,(n)"}, // LD A,($FF00+n)

        // 16-bit Load Instructions
        {0x01, InstructionType::LOAD, 12, "LD BC,nn"},
        {0x11, InstructionType::LOAD, 12, "LD DE,nn"},
        {0x21, InstructionType::LOAD, 12, "LD HL,nn"},
        {0x31, InstructionType::LOAD, 12, "LD SP,nn"},
        {0x08, InstructionType::LOAD, 20, "LD (nn),SP"},
        {0xF9, InstructionType::LOAD, 8, "LD SP,HL"},
        {0xF8, InstructionType::LOAD, 12, "LD HL,SP+n"}, // LDHL SP,n

        // PUSH rr / POP rr
        {0xC5, InstructionType::LOAD, 16, "PUSH BC"},
        {0xD5, InstructionType::LOAD, 16, "PUSH DE"},
        {0xE5, InstructionType::LOAD, 16, "PUSH HL"},
        {0xF5, InstructionType::LOAD, 16, "PUSH AF"},
        {0xC1, InstructionType::LOAD, 12, "POP BC"},
        {0xD1, InstructionType::LOAD, 12, "POP DE"},
        {0xE1, InstructionType::LOAD, 12, "POP HL"},
        {0xF1, InstructionType::LOAD, 12, "POP AF"},

        // 8-bit ALU Instructions
        // ADD A,r / ADD A,n / ADD A,(HL)
        {0x80, InstructionType::ALU, 4, "ADD A,B"},
        {0x81, InstructionType::ALU, 4, "ADD A,C"},
        {0x82, InstructionType::ALU, 4, "ADD A,D"},
        {0x83, InstructionType::ALU, 4, "ADD A,E"},
        {0x84, InstructionType::ALU, 4, "ADD A,H"},
        {0x85, InstructionType::ALU, 4, "ADD A,L"},
        {0x86, InstructionType::ALU, 8, "ADD A,(HL)"},
        {0x87, InstructionType::ALU, 4, "ADD A,A"},
        {0xC6, InstructionType::ALU, 8, "ADD A,n"},

        // ADC A,r / ADC A,n / ADC A,(HL)
        {0x88, InstructionType::ALU, 4, "ADC A,B"},
        {0x89, InstructionType::ALU, 4, "ADC A,C"},
        {0x8A, InstructionType::ALU, 4, "ADC A,D"},
        {0x8B, InstructionType::ALU, 4, "ADC A,E"},
        {0x8C, InstructionType::ALU, 4, "ADC A,H"},
        {0x8D, InstructionType::ALU, 4, "ADC A,L"},
        {0x8E, InstructionType::ALU, 8, "ADC A,(HL)"},
        {0x8F, InstructionType::ALU, 4, "ADC A,A"},
        {0xCE, InstructionType::ALU, 8, "ADC A,n"},

        // SUB r / SUB n / SUB (HL)
        {0x90, InstructionType::ALU, 4, "SUB B"},
        {0x91, InstructionType::ALU, 4, "SUB C"},
        {0x92, InstructionType::ALU, 4, "SUB D"},
        {0x93, InstructionType::ALU, 4, "SUB E"},
        {0x94, InstructionType::ALU, 4, "SUB H"},
        {0x95, InstructionType::ALU, 4, "SUB L"},
        {0x96, InstructionType::ALU, 8, "SUB (HL)"},
        {0x97, InstructionType::ALU, 4, "SUB A"},
        {0xD6, InstructionType::ALU, 8, "SUB n"},

        // SBC A,r / SBC A,n / SBC A,(HL)
        {0x98, InstructionType::ALU, 4, "SBC A,B"},
        {0x99, InstructionType::ALU, 4, "SBC A,C"},
        {0x9A, InstructionType::ALU, 4, "SBC A,D"},
        {0x9B, InstructionType::ALU, 4, "SBC A,E"},
        {0x9C, InstructionType::ALU, 4, "SBC A,H"},
        {0x9D, InstructionType::ALU, 4, "SBC A,L"},
        {0x9E, InstructionType::ALU, 8, "SBC A,(HL)"},
        {0x9F, InstructionType::ALU, 4, "SBC A,A"},
        {0xDE, InstructionType::ALU, 8, "SBC A,n"},

        // AND r / AND n / AND (HL)
        {0xA0, InstructionType::ALU, 4, "AND B"},
        {0xA1, InstructionType::ALU, 4, "AND C"},
        {0xA2, InstructionType::ALU, 4, "AND D"},
        {0xA3, InstructionType::ALU, 4, "AND E"},
        {0xA4, InstructionType::ALU, 4, "AND H"},
        {0xA5, InstructionType::ALU, 4, "AND L"},
        {0xA6, InstructionType::ALU, 8, "AND (HL)"},
        {0xA7, InstructionType::ALU, 4, "AND A"},
        {0xE6, InstructionType::ALU, 8, "AND n"},

        // OR r / OR n / OR (HL)
        {0xB0, InstructionType::ALU, 4, "OR B"},
        {0xB1, InstructionType::ALU, 4, "OR C"},
        {0xB2, InstructionType::ALU, 4, "OR D"},
        {0xB3, InstructionType::ALU, 4, "OR E"},
        {0xB4, InstructionType::ALU, 4, "OR H"},
        {0xB5, InstructionType::ALU, 4, "OR L"},
        {0xB6, InstructionType::ALU, 8, "OR (HL)"},
        {0xB7, InstructionType::ALU, 4, "OR A"},
        {0xF6, InstructionType::ALU, 8, "OR n"},

        // XOR r / XOR n / XOR (HL)
        {0xA8, InstructionType::ALU, 4, "XOR B"},
        {0xA9, InstructionType::ALU, 4, "XOR C"},
        {0xAA, InstructionType::ALU, 4, "XOR D"},
        {0xAB, InstructionType::ALU, 4, "XOR E"},
        {0xAC, InstructionType::ALU, 4, "XOR H"},
        {0xAD, InstructionType::ALU, 4, "XOR L"},
        {0xAE, InstructionType::ALU, 8, "XOR (HL)"},
        {0xAF, InstructionType::ALU, 4, "XOR A"},
        {0xEE, InstructionType::ALU, 8, "XOR n"},

        // CP r / CP n / CP (HL)
        {0xB8, InstructionType::ALU, 4, "CP B"},
        {0xB9, InstructionType::ALU, 4, "CP C"},
        {0xBA, InstructionType::ALU, 4, "CP D"},
        {0xBB, InstructionType::ALU, 4, "CP E"},
        {0xBC, InstructionType::ALU, 4, "CP H"},
        {0xBD, InstructionType::ALU, 4, "CP L"},
        {0xBE, InstructionType::ALU, 8, "CP (HL)"},
        {0xBF, InstructionType::ALU, 4, "CP A"},
        {0xFE, InstructionType::ALU, 8, "CP n"},

        // INC r / INC (HL)
        {0x04, InstructionType::ALU, 4, "INC B"},
        {0x0C, InstructionType::ALU, 4, "INC C"},
        {0x14, InstructionType::ALU, 4, "INC D"},
        {0x1C, InstructionType::ALU, 4, "INC E"},
        {0x24, InstructionType::ALU, 4, "INC H"},
        {0x2C, InstructionType::ALU, 4, "INC L"},
        {0x34, InstructionType::ALU, 12, "INC (HL)"},
        {0x3C, InstructionType::ALU, 4, "INC A"},

        // DEC r / DEC (HL)
        {0x05, InstructionType::ALU, 4, "DEC B"},
        {0x0D, InstructionType::ALU, 4, "DEC C"},
        {0x15, InstructionType::ALU, 4, "DEC D"},
        {0x1D, InstructionType::ALU, 4, "DEC E"},
        {0x25, InstructionType::ALU, 4, "DEC H"},
        {0x2D, InstructionType::ALU, 4, "DEC L"},
        {0x35, InstructionType::ALU, 12, "DEC (HL)"},
        {0x3D, InstructionType::ALU, 4, "DEC A"},

        // 16-bit ALU Instructions
        // ADD HL,rr
        {0x09, InstructionType::ALU, 8, "ADD HL,BC"},
        {0x19, InstructionType::ALU, 8, "ADD HL,DE"},
        {0x29, InstructionType::ALU, 8, "ADD HL,HL"},
        {0x39, InstructionType::ALU, 8, "ADD HL,SP"},
        // ADD SP,n
        {0xE8, InstructionType::ALU, 16, "ADD SP,n"}, // n is signed byte

        // INC rr / DEC rr
        {0x03, InstructionType::ALU, 8, "INC BC"},
        {0x13, InstructionType::ALU, 8, "INC DE"},
        {0x23, InstructionType::ALU, 8, "INC HL"},
        {0x33, InstructionType::ALU, 8, "INC SP"},
        {0x0B, InstructionType::ALU, 8, "DEC BC"},
        {0x1B, InstructionType::ALU, 8, "DEC DE"},
        {0x2B, InstructionType::ALU, 8, "DEC HL"},
        {0x3B, InstructionType::ALU, 8, "DEC SP"},

        // Rotates and Shifts (A register)
        {0x07, InstructionType::ALU, 4, "RLCA"},
        {0x0F, InstructionType::ALU, 4, "RRCA"},
        {0x17, InstructionType::ALU, 4, "RLA"},
        {0x1F, InstructionType::ALU, 4, "RRA"},
        
        // Jump Instructions
        // JP nn / JP HL
        {0xC3, InstructionType::JUMP, 16, "JP nn"}, // Cycles are 16 if no jump, 12 if jump
        {0xE9, InstructionType::JUMP, 4, "JP (HL)"}, // JP HL
        // JP cc,nn
        {0xC2, InstructionType::JUMP, 12, "JP NZ,nn"}, // 12/16
        {0xCA, InstructionType::JUMP, 12, "JP Z,nn"},  // 12/16
        {0xD2, InstructionType::JUMP, 12, "JP NC,nn"}, // 12/16
        {0xDA, InstructionType::JUMP, 12, "JP C,nn"},  // 12/16
        // JR n
        {0x18, InstructionType::JUMP, 12, "JR n"},
        // JR cc,n
        {0x20, InstructionType::JUMP, 8, "JR NZ,n"}, // 8/12
        {0x28, InstructionType::JUMP, 8, "JR Z,n"},  // 8/12
        {0x30, InstructionType::JUMP, 8, "JR NC,n"}, // 8/12
        {0x38, InstructionType::JUMP, 8, "JR C,n"},  // 8/12

        // Call Instructions
        // CALL nn
        {0xCD, InstructionType::JUMP, 24, "CALL nn"},
        // CALL cc,nn
        {0xC4, InstructionType::JUMP, 12, "CALL NZ,nn"}, // 12/24
        {0xCC, InstructionType::JUMP, 12, "CALL Z,nn"},  // 12/24
        {0xD4, InstructionType::JUMP, 12, "CALL NC,nn"}, // 12/24
        {0xDC, InstructionType::JUMP, 12, "CALL C,nn"},  // 12/24

        // Return Instructions
        // RET
        {0xC9, InstructionType::JUMP, 16, "RET"},
        // RET cc
        {0xC0, InstructionType::JUMP, 8, "RET NZ"}, // 8/20
        {0xC8, InstructionType::JUMP, 8, "RET Z"},  // 8/20
        {0xD0, InstructionType::JUMP, 8, "RET NC"}, // 8/20
        {0xD8, InstructionType::JUMP, 8, "RET C"},  // 8/20
        // RETI
        {0xD9, InstructionType::JUMP, 16, "RETI"},

        // Restart Instructions (RST)
        {0xC7, InstructionType::JUMP, 16, "RST 00H"},
        {0xCF, InstructionType::JUMP, 16, "RST 08H"},
        {0xD7, InstructionType::JUMP, 16, "RST 10H"},
        {0xDF, InstructionType::JUMP, 16, "RST 18H"},
        {0xE7, InstructionType::JUMP, 16, "RST 20H"},
        {0xEF, InstructionType::JUMP, 16, "RST 28H"},
        {0xF7, InstructionType::JUMP, 16, "RST 30H"},
        {0xFF, InstructionType::JUMP, 16, "RST 38H"},
        
        // CB Prefix for extended operations
        {0xCB, InstructionType::BIT, 4, "PREFIX CB"} // Cycles for CB prefix itself
    };


    // Helper functions
    inline InstructionType getInstructionType(BYTE opcode) {
        for (const auto& mapping : OPCODE_MAP) {
            if (mapping.opcode == opcode) {
                return mapping.type;
            }
        }
        return InstructionType::UNKNOWN;
    }

    inline BYTE getInstructionCycles(BYTE opcode) {
        for (const auto& mapping : OPCODE_MAP) {
            if (mapping.opcode == opcode) {
                return mapping.cycles;
            }
        }
        return 0;
    }

    inline const char* getInstructionDescription(BYTE opcode) {
        for (const auto& mapping : OPCODE_MAP) {
            if (mapping.opcode == opcode) {
                return mapping.description;
            }
        }
        return "Unknown instruction";
    }

    // ALU Operation Cycles
    constexpr BYTE ALU_REGULAR_CYCLES = 4;
    constexpr BYTE ALU_MEMORY_CYCLES = 8;


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
    }
}