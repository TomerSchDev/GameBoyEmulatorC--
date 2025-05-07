#include "memory_controller.h"
#include <sstream>
#include <iomanip>

MemoryController::MemoryController()
    : m_EnableRAM(false)
    , m_ROMBanking(true)
    , m_MBC1(false)
    , m_MBC2(false)
    , m_CurrentROMBank(1)
    , m_CurrentRAMBank(0)
{
    ram = std::make_unique<RAM>();
    m_RAMBanks.resize(0x8000);  // 32KB of RAM banks
    LOG_INFO("Memory Controller initialized");
}

BYTE MemoryController::read(WORD address) const {
    auto region = getMemoryRegion(address);
    BYTE value = 0xFF;

    switch (region) {
        case MemoryRegion::JOYPAD_REGISTER:
        {
            // Handle joypad register read
            BYTE joypadRequest = read(JOYPAD_REGISTER);
            return (joypadRequest & 0xF0) | emulator->joypad.GetState(joypadRequest);
        }
        case MemoryRegion::ROM_BANK_0:
            if (cart && cart->isLoaded()) {
                return cart->read(address);
            }
            break;

        case MemoryRegion::ROM_BANK_N:
            if (cart && cart->isLoaded()) {
                WORD newAddress = address - 0x4000;
                return cart->read(newAddress + (m_CurrentROMBank * 0x4000));
            }
            break;

        case MemoryRegion::EXTERNAL_RAM:
            if (m_EnableRAM) {
                WORD newAddress = address - 0xA000;
                return m_RAMBanks[newAddress + (m_CurrentRAMBank * 0x2000)];
            }
            break;

        case MemoryRegion::ECHO_RAM:
            return ram->read(address - 0x2000);

        case MemoryRegion::RESTRICTED:
            LOG_WARNING("Read attempt from restricted memory area: 0x" + 
                       std::to_string(address));
            break;

        default:
            return ram->read(address);
    }

    std::stringstream ss;
    ss << "Memory Read - Region: " << getMemoryRegionName(region)
       << " Address: 0x" << std::hex << std::setw(4) << std::setfill('0') << address
       << " Value: 0x" << std::setw(2) << static_cast<int>(value);
    LOG_DEBUG(ss.str());

    return value;
}
void MemoryController::DoRamBankEnable(WORD address, BYTE data) {
    // Check MBC2 specific condition
    if (m_MBC2) {
        if ((address & 0x10) != 0) {  // Test bit 4 of address
            return;
        }
    }

    // Check lower nibble for both MBC1 and MBC2
    BYTE testData = data & 0x0F;
    if (testData == 0x0A) {
        m_EnableRAM = true;
    } else if (testData == 0x00) {
        m_EnableRAM = false;
    }

    LOG_DEBUG("RAM bank " + std::string(m_EnableRAM ? "enabled" : "disabled") + 
              " (MBC" + std::string(m_MBC2 ? "2" : "1") + ")");
}

void MemoryController::DoChangeLoROMBank(BYTE data) {
    // MBC2 handling - only uses lower 4 bits
    if (m_MBC2) {
        m_CurrentROMBank = data & 0x0F;
        if (m_CurrentROMBank == 0) {
            m_CurrentROMBank++;  // Bank 0 is fixed at 0x0000-0x3FFF
        }
        LOG_DEBUG("MBC2 ROM bank changed to " + std::to_string(m_CurrentROMBank));
        return;
    }

    // MBC1 handling - uses lower 5 bits
    BYTE lower5 = data & 0x1F;           // 0x1F = 31 = 0b00011111
    m_CurrentROMBank &= 0xE0;           // 0xE0 = 224 = 0b11100000
    m_CurrentROMBank |= lower5;

    if (m_CurrentROMBank == 0) {
        m_CurrentROMBank++;  // Bank 0 is fixed at 0x0000-0x3FFF
    }
    
    LOG_DEBUG("MBC1 ROM bank lower bits changed to " + std::to_string(m_CurrentROMBank));
}
void MemoryController::doDMATransfer(BYTE data) {
    WORD sourceAddress = data << 8;  // Multiply by 0x100 (256)
    
    // Copy 160 bytes from source to OAM
    for (BYTE i = 0; i < DMA_LENGTH; i++) {
        BYTE value = read(sourceAddress + i);
        write(OAM_START + i, value);
    }
    
    LOG_DEBUG("DMA Transfer from 0x" + 
              std::to_string(sourceAddress) + 
              " to OAM complete");
}
void MemoryController::DoChangeHiRomBank(BYTE data) {
    // Clear upper 3 bits of current ROM bank (keep lower 5)
    m_CurrentROMBank &= 0x1F;           // 0x1F = 31 = 0b00011111
    
    // Set upper bits from data (bits 5-6)
    m_CurrentROMBank |= ((data & 0x03) << 5);  // Shift to bits 5-6 position
    
    if (m_CurrentROMBank == 0) {
        m_CurrentROMBank++;  // Bank 0 is fixed at 0x0000-0x3FFF
    }
    
    LOG_DEBUG("MBC1 ROM bank high bits changed to " + std::to_string(m_CurrentROMBank));
}

void MemoryController::DoRAMBankChange(BYTE data) {
    if (m_MBC2) {
        LOG_WARNING("Attempted RAM bank change in MBC2 mode");
        return;  // MBC2 doesn't support RAM banking
    }
    
    m_CurrentRAMBank = data & 0x03;  // Only lower 2 bits used
    LOG_DEBUG("RAM bank changed to " + std::to_string(m_CurrentRAMBank));
}

void MemoryController::DoChangeROMRAMMode(BYTE data) {
    bool newMode = (data & 0x01) == 0;
    
    // Only change mode if it's different
    if (m_ROMBanking != newMode) {
        m_ROMBanking = newMode;
        
        // Reset RAM bank when switching to ROM banking mode
        if (m_ROMBanking) {
            m_CurrentRAMBank = 0;
            LOG_DEBUG("Switched to ROM banking mode, RAM bank reset to 0");
        } else {
            LOG_DEBUG("Switched to RAM banking mode");
        }
    }
}


void MemoryController::write(WORD address, BYTE data) {
    auto region = getMemoryRegion(address);

    std::stringstream ss;
    ss << "Memory Write - Region: " << getMemoryRegionName(region)
       << " Address: 0x" << std::hex << std::setw(4) << std::setfill('0') << address
       << " Data: 0x" << std::setw(2) << static_cast<int>(data);
    LOG_DEBUG(ss.str());

    switch (region) {
        case MemoryRegion::JOYPAD_REGISTER:
        {
            // Handle joypad register write
            BYTE joypadRequest = read(JOYPAD_REGISTER);
            ram->write(JOYPAD_REGISTER, joypadRequest & 0xF0);
            break;
        }
            
        case MemoryRegion::ROM_BANK_0:
        case MemoryRegion::ROM_BANK_N:
            HandleBanking(address, data);
            break;
        case MemoryRegion::DMA_REGISTER:
            doDMATransfer(data);
            break;
        case MemoryRegion::EXTERNAL_RAM:
            if (m_EnableRAM) {
                WORD newAddress = address - 0xA000;
                m_RAMBanks[newAddress + (m_CurrentRAMBank * 0x2000)] = data;
            }
            break;

        case MemoryRegion::ECHO_RAM:
            ram->write(address - 0x2000, data);
            ram->write(address, data);
            break;

        case MemoryRegion::RESTRICTED:
            LOG_WARNING("Write attempt to restricted memory area: 0x" + 
                       std::to_string(address));
            break;

        default:
            ram->write(address, data);
            break;
    }
}

void MemoryController::HandleBanking(WORD address, BYTE data) {
    if (address < 0x2000) {
        if (m_MBC1 || m_MBC2) {
            DoRamBankEnable(address, data);
        }
    }
    else if (address < 0x4000) {
        if (m_MBC1 || m_MBC2) {
            DoChangeLoROMBank(data);
        }
    }
    else if (address < 0x6000) {
        if (m_MBC1) {
            if (m_ROMBanking) {
                DoChangeHiRomBank(data);
            } else {
                DoRAMBankChange(data);
            }
        }
    }
    else if (address < 0x8000) {
        if (m_MBC1) {
            DoChangeROMRAMMode(data);
        }
    }
}

bool MemoryController::attachCart(std::unique_ptr<Cart> newCart) {
    if (cart) {
        LOG_WARNING("Attempting to attach cart while another is present");
        return false;
    }
    
    cart = std::move(newCart);
    
    // Set MBC type based on cartridge type
    // TODO: Implement cartridge type detection
    
    LOG_INFO("Cartridge attached to Memory Controller");
    return true;
}

bool MemoryController::detachCart() {
    if (!cart) {
        LOG_WARNING("Attempting to detach cart when none is present");
        return false;
    }
    
    cart.reset();
    m_EnableRAM = false;
    m_CurrentROMBank = 1;
    m_CurrentRAMBank = 0;
    
    LOG_INFO("Cartridge detached from Memory Controller");
    return true;
}