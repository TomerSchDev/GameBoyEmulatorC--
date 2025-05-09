#include "cpu.h"
#include "instructions.h" // For declarations of *_impl functions
#include "OpcodeTables.h"
#include "logger.h"
#include <sstream>
#include <iomanip>
#include <stdexcept> // For potential use if needed
#include "emulator.h"

// Define IF and IE register addresses (example, ensure these are correct for your memory map)
#define IF_REGISTER 0xFF0F
#define IE_REGISTER 0xFFFF

namespace GB {

// --- Constructor ---
CPU::CPU(std::shared_ptr<MemoryController> memory)
    : memoryController(memory),
      halted(false),
      stopped(false),
      interruptEnabled(false),
      pendingInterruptEnable(false),
      opcodeTables(OpcodeTables::getInstance()) // Initialize reference to singleton
{
    Reset();
    LOG_INFO("CPU initialized and reset.");
}

// --- CPU Reset ---
void CPU::Reset() {
    // Initial register values for DMG
    m_RegisterAF.reg = 0x01B0;
    m_RegisterBC.reg = 0x0013;
    m_RegisterDE.reg = 0x00D8;
    m_RegisterHL.reg = 0x014D;
    m_ProgramCounter = 0x0100;
    m_StackPointer.reg = 0xFFFE;

    halted = false;
    stopped = false;
    interruptEnabled = false;
    pendingInterruptEnable = false;

    LOG_INFO("CPU reset to initial state. PC=0x0100, SP=0xFFFE");
}

// --- Memory Access ---
BYTE CPU::readMemory(WORD address) const {
    return memoryController->read(address);
}

void CPU::writeMemory(WORD address, BYTE data) {
    memoryController->write(address, data);
}

BYTE CPU::readBytePC() {
    BYTE value = readMemory(m_ProgramCounter);
    m_ProgramCounter++;
    return value;
}

WORD CPU::readWordPC() {
    BYTE lo = readBytePC(); // Reads at PC, increments PC
    BYTE hi = readBytePC(); // Reads at new PC, increments PC
    return (static_cast<WORD>(hi) << 8) | lo;
}

// --- Flag Management ---
// ... (Flag setters/getters remain the same) ...
void CPU::setFlagZ(bool value) {
    if (value) m_RegisterAF.lo |= FLAG_Z_MASK;
    else m_RegisterAF.lo &= ~FLAG_Z_MASK;
}
void CPU::setFlagN(bool value) {
    if (value) m_RegisterAF.lo |= FLAG_N_MASK;
    else m_RegisterAF.lo &= ~FLAG_N_MASK;
}
void CPU::setFlagH(bool value) {
    if (value) m_RegisterAF.lo |= FLAG_H_MASK;
    else m_RegisterAF.lo &= ~FLAG_H_MASK;
}
void CPU::setFlagC(bool value) {
    if (value) m_RegisterAF.lo |= FLAG_C_MASK;
    else m_RegisterAF.lo &= ~FLAG_C_MASK;
}

bool CPU::getFlagZ() const { return (m_RegisterAF.lo & FLAG_Z_MASK) != 0; }
bool CPU::getFlagN() const { return (m_RegisterAF.lo & FLAG_N_MASK) != 0; }
bool CPU::getFlagH() const { return (m_RegisterAF.lo & FLAG_H_MASK) != 0; }
bool CPU::getFlagC() const { return (m_RegisterAF.lo & FLAG_C_MASK) != 0; }


// --- Stack Operations ---
void CPU::pushStackWord(WORD value) {
    m_StackPointer.reg--;
    writeMemory(m_StackPointer.reg, (value >> 8) & 0xFF); // Push MSB
    m_StackPointer.reg--;
    writeMemory(m_StackPointer.reg, value & 0xFF);        // Push LSB
}

WORD CPU::popStackWord() {
    BYTE lo = readMemory(m_StackPointer.reg);        // Pop LSB
    m_StackPointer.reg++;
    BYTE hi = readMemory(m_StackPointer.reg);        // Pop MSB
    m_StackPointer.reg++;
    return (static_cast<WORD>(hi) << 8) | lo;
}

// --- Interrupt Handling ---
// ... (Interrupt handling code remains the same) ...
void CPU::RequestInterrupt(BYTE interruptBit) {
    BYTE currentIF = readMemory(IF_REGISTER);
    writeMemory(IF_REGISTER, currentIF | interruptBit);
}

int CPU::handleInterrupts() {
    if (!interruptEnabled && !halted) {
        return 0; // No interrupts to handle if interrupts are disabled and not halted
    }

    BYTE IE = readMemory(IE_REGISTER);
    BYTE IF = readMemory(IF_REGISTER);
    BYTE requestedAndEnabled = IE & IF;
    // *** ADD THIS LOG ***
    if ((IE & IF) != 0) { // Log only if there's potential for an interrupt
        std::stringstream ss;
        ss << "HandleInterrupts Check: IME=" << interruptEnabled
        << " IE=0x" << std::hex << static_cast<int>(IE)
        << " IF=0x" << static_cast<int>(IF)
        << " ReqEn=0x" << static_cast<int>(requestedAndEnabled);
        LOG_DEBUG(ss.str());
}
    if (requestedAndEnabled == 0) {
        return 0; // No interrupts to handle
    }

    if (halted) {
        halted = false;
    }

    if (!interruptEnabled) {
        return 0; // Interrupts are disabled, do not handle them
    }

    BYTE interruptToService = 0;
    WORD interruptAddress = 0;

    if (requestedAndEnabled & 0x01) { // VBlank
        interruptToService = 0x01;
        interruptAddress = 0x0040;
    } else if (requestedAndEnabled & 0x02) { // LCD STAT
        interruptToService = 0x02;
        interruptAddress = 0x0048;
    } else if (requestedAndEnabled & 0x04) { // Timer
        interruptToService = 0x04;
        interruptAddress = 0x0050;
    } else if (requestedAndEnabled & 0x08) { // Serial
        interruptToService = 0x08;
        interruptAddress = 0x0058;
    } else if (requestedAndEnabled & 0x10) { // Joypad
        interruptToService = 0x10;
        interruptAddress = 0x0060;
    }

    if (interruptToService != 0) {
        interruptEnabled = false;
        writeMemory(IF_REGISTER, IF & ~interruptToService);
        pushStackWord(m_ProgramCounter);
        m_ProgramCounter = interruptAddress;
        LOG_DEBUG("Servicing Interrupt - Type: 0x" + std::to_string(interruptToService) + " Addr: 0x" + std::to_string(interruptAddress));
        LOG_DEBUG("Interrupt serviced successfully.");
    }
    return 20; // 5 M-cycles (20 T-cycles) for handling an interrupt
}


// --- Logging Helper ---
void CPU::logOpcodeExecution(BYTE opcode_val, bool is_prefixed, const OpcodeInfo& info, WORD current_pc_before_fetch) {
    std::stringstream ss;
    ss << "PC:0x" << std::hex << std::setw(4) << std::setfill('0') << current_pc_before_fetch
       << " | Op:0x" << std::setw(2) << std::setfill('0') << static_cast<int>(opcode_val)
       << (is_prefixed ? " (CB)" : "")
       << " | Mnem:" << info.mnemonic
       << " | AF:" << std::setw(4) << getAF()
       << " BC:" << std::setw(4) << getBC()
       << " DE:" << std::setw(4) << getDE()
       << " HL:" << std::setw(4) << getHL()
       << " SP:" << std::setw(4) << getSP()
       << " | Flags:" << (getFlagZ() ? "Z" : "-") << (getFlagN() ? "N" : "-")
       << (getFlagH() ? "H" : "-") << (getFlagC() ? "C" : "-");
    LOG_DEBUG(ss.str());
}


// --- Core Execution Logic ---
int CPU::ExecuteNextOpcode() {
    handleInterrupts(); // Check for and handle interrupts first

    if (pendingInterruptEnable) {
        interruptEnabled = true;
        pendingInterruptEnable = false;
    }

    if (halted) {
        return 4; // 1 M-cycle (4 T-cycles)
    }

    WORD pc_before_fetch = m_ProgramCounter;
    BYTE opcode = readBytePC(); // Fetch opcode, PC is now advanced

    // *** ADDED LOG 1: Log the fetched byte immediately ***
    LOG_DEBUG("ExecuteNextOpcode - Fetched byte at 0x" + std::to_string(pc_before_fetch) + " = 0x" + std::to_string(opcode));

    const OpcodeInfo& info = (opcode == 0xCB)
                             ? opcodeTables.getInfo(readBytePC(), true) // Fetch CB sub-opcode
                             : opcodeTables.getInfo(opcode, false);     // Get info for standard opcode

    // *** ADDED LOG 2: Log the opcode value *before* passing it to logOpcodeExecution ***
    LOG_DEBUG("ExecuteNextOpcode - Opcode value before logging/processing = 0x" + std::to_string(opcode));

    // Log before execution
    if (opcode == 0xCB) {
         // Use info.address which is the CB sub-opcode value
         logOpcodeExecution(info.address, true, info, pc_before_fetch);
    } else {
         // Use the original fetched opcode
         logOpcodeExecution(opcode, false, info, pc_before_fetch);
    }

    int cycles = processInstruction(info);

    return cycles;
}




// ... (processInstruction and handleUnknownOpcode remain the same) ...
int CPU::processInstruction(const OpcodeInfo& info) {
    // --- 8-bit Load Instructions ---
    if (info.mnemonic == "LD") {
        // Further differentiate LD instructions based on operands
        // This is where the mapping to specific LD_impl functions happens
        if (info.operand2 == Register::NONE && (info.operand1 >= Register::A && info.operand1 <= Register::L)) { // LD r, n8
             if (info.length == 2) return LD_reg_n8_impl(*this, info);
        }
        if (info.operand1 == Register::MEM_HL && info.length == 2) { // LD (HL), n8
            return LD_memHL_n8_impl(*this, info);
        }
        if (info.operand1 >= Register::A && info.operand1 <= Register::L &&
            info.operand2 >= Register::A && info.operand2 <= Register::L) { // LD r, r'
            return LD_reg_reg_impl(*this, info);
        }
        if (info.operand1 >= Register::A && info.operand1 <= Register::L && info.operand2 == Register::MEM_HL) { // LD r, (HL)
            return LD_reg_memHL_impl(*this, info);
        }
        if (info.operand1 == Register::MEM_HL && info.operand2 >= Register::A && info.operand2 <= Register::L) { // LD (HL), r
            return LD_memHL_reg_impl(*this, info);
        }
        if (info.operand1 == Register::A && info.operand2 == Register::MEM_BC) return LD_A_memBC_impl(*this, info);
        if (info.operand1 == Register::A && info.operand2 == Register::MEM_DE) return LD_A_memDE_impl(*this, info);
        if (info.operand1 == Register::A && info.operand2 == Register::MEM_A16) return LD_A_memA16_impl(*this, info);
        if (info.operand1 == Register::MEM_BC && info.operand2 == Register::A) return LD_memBC_A_impl(*this, info);
        if (info.operand1 == Register::MEM_DE && info.operand2 == Register::A) return LD_memDE_A_impl(*this, info);
        if (info.operand1 == Register::MEM_A16 && info.operand2 == Register::A) return LD_memA16_A_impl(*this, info);

        // LDH instructions (Check mnemonic again, might be redundant if group is used)
        if (info.mnemonic == "LDH") {
            if (info.operand1 == Register::MEM_A8 && info.operand2 == Register::A) return LDH_memA8_A_impl(*this, info);
            if (info.operand1 == Register::A && info.operand2 == Register::MEM_A8) return LDH_A_memA8_impl(*this, info);
            if (info.operand1 == Register::MEM_C && info.operand2 == Register::A) return LDH_memC_A_impl(*this, info); // LD (C), A
            if (info.operand1 == Register::A && info.operand2 == Register::MEM_C) return LDH_A_memC_impl(*this, info); // LD A, (C)
        }

        // LD A, (HLI/HLD) and LD (HLI/HLD), A
        if (info.operand1 == Register::A && info.operand2 == Register::MEM_HLI) return LD_A_memHLI_impl(*this, info);
        if (info.operand1 == Register::A && info.operand2 == Register::MEM_HLD) return LD_A_memHLD_impl(*this, info);
        if (info.operand1 == Register::MEM_HLI && info.operand2 == Register::A) return LD_memHLI_A_impl(*this, info);
        if (info.operand1 == Register::MEM_HLD && info.operand2 == Register::A) return LD_memHLD_A_impl(*this, info);
    }

    // --- 16-bit Load Instructions ---
    if (info.mnemonic == "LD" && info.group == InstructionGroup::X16_LSM) {
        if ((info.operand1 == Register::BC || info.operand1 == Register::DE || info.operand1 == Register::HL || info.operand1 == Register::SP) && info.length == 3) { // LD rr, n16
            return LD_rr_n16_impl(*this, info);
        }
        if (info.operand1 == Register::SP && info.operand2 == Register::HL) return LD_SP_HL_impl(*this, info);
        if (info.operand1 == Register::MEM_A16 && info.operand2 == Register::SP) return LD_memA16_SP_impl(*this, info);
        if (info.operand1 == Register::HL && info.operand2 == Register::SP ) { // LD HL, SP+e8 (0xF8)
             // This specific form has e8 as an immediate value after SP.
             return LD_HL_SP_e8_impl(*this, info);
        }
    }
    if (info.mnemonic == "PUSH") return PUSH_rr_impl(*this, info);
    if (info.mnemonic == "POP") return POP_rr_impl(*this, info);


    // --- 8-bit ALU Instructions ---
    if (info.mnemonic == "ADD" && info.group == InstructionGroup::X8_ALU) {
        if (info.operand2 >= Register::A && info.operand2 <= Register::L) return ADD_A_reg_impl(*this, info);
        if (info.operand2 == Register::MEM_HL) return ADD_A_memHL_impl(*this, info);
        if (info.operand2 == Register::NONE && info.length == 2) return ADD_A_n8_impl(*this, info); // ADD A, n8
    }
    if (info.mnemonic == "ADC" && info.group == InstructionGroup::X8_ALU) {
        if (info.operand2 >= Register::A && info.operand2 <= Register::L) return ADC_A_reg_impl(*this, info);
        if (info.operand2 == Register::MEM_HL) return ADC_A_memHL_impl(*this, info);
        if (info.operand2 == Register::NONE && info.length == 2) return ADC_A_n8_impl(*this, info);
    }
    if (info.mnemonic == "SUB" && info.group == InstructionGroup::X8_ALU) {
        if (info.operand2 >= Register::A && info.operand2 <= Register::L) return SUB_A_reg_impl(*this, info);
        if (info.operand2 == Register::MEM_HL) return SUB_A_memHL_impl(*this, info);
        if (info.operand2 == Register::NONE && info.length == 2) return SUB_A_n8_impl(*this, info);
    }
    if (info.mnemonic == "SBC" && info.group == InstructionGroup::X8_ALU) {
        if (info.operand2 >= Register::A && info.operand2 <= Register::L) return SBC_A_reg_impl(*this, info);
        if (info.operand2 == Register::MEM_HL) return SBC_A_memHL_impl(*this, info);
        if (info.operand2 == Register::NONE && info.length == 2) return SBC_A_n8_impl(*this, info);
    }
    if (info.mnemonic == "AND" && info.group == InstructionGroup::X8_ALU) {
        if (info.operand2 >= Register::A && info.operand2 <= Register::L) return AND_A_reg_impl(*this, info);
        if (info.operand2 == Register::MEM_HL) return AND_A_memHL_impl(*this, info);
        if (info.operand2 == Register::NONE && info.length == 2) return AND_A_n8_impl(*this, info);
    }
    if (info.mnemonic == "XOR" && info.group == InstructionGroup::X8_ALU) {
        if (info.operand2 >= Register::A && info.operand2 <= Register::L) return XOR_A_reg_impl(*this, info);
        if (info.operand2 == Register::MEM_HL) return XOR_A_memHL_impl(*this, info);
        if (info.operand2 == Register::NONE && info.length == 2) return XOR_A_n8_impl(*this, info);
    }
    if (info.mnemonic == "OR" && info.group == InstructionGroup::X8_ALU) {
        if (info.operand2 >= Register::A && info.operand2 <= Register::L) return OR_A_reg_impl(*this, info);
        if (info.operand2 == Register::MEM_HL) return OR_A_memHL_impl(*this, info);
        if (info.operand2 == Register::NONE && info.length == 2) return OR_A_n8_impl(*this, info);
    }
    if (info.mnemonic == "CP" && info.group == InstructionGroup::X8_ALU) {
        if (info.operand2 >= Register::A && info.operand2 <= Register::L) return CP_A_reg_impl(*this, info);
        if (info.operand2 == Register::MEM_HL) return CP_A_memHL_impl(*this, info);
        if (info.operand2 == Register::NONE && info.length == 2) return CP_A_n8_impl(*this, info);
    }
    if (info.mnemonic == "INC" && info.group == InstructionGroup::X8_ALU) {
        if (info.operand1 >= Register::A && info.operand1 <= Register::L) return INC_reg_impl(*this, info);
        if (info.operand1 == Register::MEM_HL) return INC_memHL_impl(*this, info);
    }
    if (info.mnemonic == "DEC" && info.group == InstructionGroup::X8_ALU) {
        if (info.operand1 >= Register::A && info.operand1 <= Register::L) return DEC_reg_impl(*this, info);
        if (info.operand1 == Register::MEM_HL) return DEC_memHL_impl(*this, info);
    }

    // --- 16-bit ALU Instructions ---
    if (info.mnemonic == "ADD" && info.group == InstructionGroup::X16_ALU) {
         if (info.operand1 == Register::HL && (info.operand2 == Register::BC || info.operand2 == Register::DE || info.operand2 == Register::HL || info.operand2 == Register::SP) ) {
            return ADD_HL_rr_impl(*this, info);
         }
         if (info.operand1 == Register::SP && info.length == 2) { // ADD SP, e8
            return ADD_SP_e8_impl(*this, info);
         }
    }
    if (info.mnemonic == "INC" && info.group == InstructionGroup::X16_ALU) return INC_rr_impl(*this, info);
    if (info.mnemonic == "DEC" && info.group == InstructionGroup::X16_ALU) return DEC_rr_impl(*this, info);


    // --- Rotate and Shift Instructions (Non-CB) ---
    if (info.mnemonic == "RLCA") return RLCA_impl(*this, info);
    if (info.mnemonic == "RLA") return RLA_impl(*this, info);
    if (info.mnemonic == "RRCA") return RRCA_impl(*this, info);
    if (info.mnemonic == "RRA") return RRA_impl(*this, info);

    // --- CB-Prefixed Instructions (dispatched from here if info.isPrefixed) ---
    if (info.isPrefixed) {
        if (info.mnemonic == "RLC") {
            if (info.operand1 != Register::MEM_HL) return RLC_reg_impl(*this, info);
            else return RLC_memHL_impl(*this, info);
        }
        if (info.mnemonic == "RRC") {
            if (info.operand1 != Register::MEM_HL) return RRC_reg_impl(*this, info);
            else return RRC_memHL_impl(*this, info);
        }
        if (info.mnemonic == "RL") {
            if (info.operand1 != Register::MEM_HL) return RL_reg_impl(*this, info);
            else return RL_memHL_impl(*this, info);
        }
        if (info.mnemonic == "RR") {
            if (info.operand1 != Register::MEM_HL) return RR_reg_impl(*this, info);
            else return RR_memHL_impl(*this, info);
        }
        if (info.mnemonic == "SLA") {
            if (info.operand1 != Register::MEM_HL) return SLA_reg_impl(*this, info);
            else return SLA_memHL_impl(*this, info);
        }
        if (info.mnemonic == "SRA") {
            if (info.operand1 != Register::MEM_HL) return SRA_reg_impl(*this, info);
            else return SRA_memHL_impl(*this, info);
        }
        if (info.mnemonic == "SWAP") {
            if (info.operand1 != Register::MEM_HL) return SWAP_reg_impl(*this, info);
            else return SWAP_memHL_impl(*this, info);
        }
        if (info.mnemonic == "SRL") {
            if (info.operand1 != Register::MEM_HL) return SRL_reg_impl(*this, info);
            else return SRL_memHL_impl(*this, info);
        }
        if (info.mnemonic == "BIT") {
            if (info.operand1 != Register::MEM_HL) return BIT_b_reg_impl(*this, info);
            else return BIT_b_memHL_impl(*this, info);
        }
        if (info.mnemonic == "RES") {
            if (info.operand1 != Register::MEM_HL) return RES_b_reg_impl(*this, info);
            else return RES_b_memHL_impl(*this, info);
        }
        if (info.mnemonic == "SET") {
            if (info.operand1 != Register::MEM_HL) return SET_b_reg_impl(*this, info);
            else return SET_b_memHL_impl(*this, info);
        }
    }

    // --- Control/Branch Instructions ---
    if (info.mnemonic == "JP") {
        if (info.operand1 == Register::HL) return JP_HL_impl(*this, info);
        if (info.condition != ConditionType::NONE && info.length == 3) return JP_cc_n16_impl(*this, info);
        if (info.condition == ConditionType::NONE && info.length == 3) return JP_n16_impl(*this, info);
    }
    if (info.mnemonic == "JR") {
        if (info.condition != ConditionType::NONE) return JR_cc_e8_impl(*this, info);
        return JR_e8_impl(*this, info);
    }
    if (info.mnemonic == "CALL") {
        if (info.condition != ConditionType::NONE) return CALL_cc_n16_impl(*this, info);
        return CALL_n16_impl(*this, info);
    }
    if (info.mnemonic == "RET") {
        if (info.condition != ConditionType::NONE) return RET_cc_impl(*this, info);
        return RET_impl(*this, info);
    }
    if (info.mnemonic == "RETI") return RETI_impl(*this, info);
    if (info.mnemonic == "RST") return RST_impl(*this, info);


    // --- Control/Miscellaneous Instructions ---
    if (info.mnemonic == "NOP") return NOP_impl(*this, info);
    if (info.mnemonic == "HALT") return HALT_impl(*this, info);
    if (info.mnemonic == "STOP") return STOP_impl(*this, info);
    if (info.mnemonic == "DI") return DI_impl(*this, info);
    if (info.mnemonic == "EI") return EI_impl(*this, info);
    if (info.mnemonic == "DAA") return DAA_impl(*this, info);
    if (info.mnemonic == "CPL") return CPL_impl(*this, info);
    if (info.mnemonic == "SCF") return SCF_impl(*this, info);
    if (info.mnemonic == "CCF") return CCF_impl(*this, info);


    // If no specific handler was found
    return handleUnknownOpcode(info.address, info.isPrefixed);
}


int CPU::handleUnknownOpcode(BYTE opcode, bool prefixed) {
    std::stringstream ss;
    ss << "Unknown or unimplemented opcode: " << (prefixed ? "CB " : "")
       << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(opcode)
       << " at PC=0x" << std::hex << std::setw(4) << std::setfill('0') << (m_ProgramCounter - (prefixed ? 2 : 1)); // PC is already advanced
    LOG_ERROR(ss.str());
    // You might want to throw an exception or stop emulation here
    // For now, return a default cycle count to avoid infinite loops if possible
    return 4;
}

} // namespace GB

