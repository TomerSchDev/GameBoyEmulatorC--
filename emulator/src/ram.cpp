#include <logger.h>
#include "ram.h"
#include <string>
RAM::RAM()
{
    memset(this->m_memory,0,sizeof(m_memory));
}

RAM::~RAM()
{
}

BYTE RAM::read(WORD address) const {
    // Check for invalid addresses
    if (address >= RAM_SIZE) {
        LOG_ERROR("Attempted read from invalid address: 0x" + 
                 std::to_string(address));
        return 0xFF;
    }

    // Special handling for Echo RAM (0xE000-0xFDFF mirrors 0xC000-0xDDFF)
    if (address >= 0xE000 && address < 0xFE00) {
        WORD mirrorAddr = address - 0x2000;
        LOG_DEBUG("Echo RAM read - Mirror address 0x" + 
                 std::to_string(mirrorAddr) + " -> 0x" + 
                 std::to_string(address));
        return m_memory[mirrorAddr];
    }

    // Restricted area (0xFEA0-0xFEFF)
    if (address >= 0xFEA0 && address <= 0xFEFF) {
        LOG_WARNING("Read attempt from restricted area: 0x" + 
                   std::to_string(address));
        return 0xFF;
    }

    return m_memory[address];
}

void RAM::write(WORD address, BYTE data) {
    // Check for invalid addresses
    if (address >= RAM_SIZE) {
        LOG_ERROR("Attempted write to invalid address: 0x" + 
                 std::to_string(address) + " with data: 0x" + 
                 std::to_string(data));
        return;
    }

    // ROM area (0x0000-0x7FFF)
    if (address < 0x8000) {
        LOG_WARNING("Attempted write to ROM area: 0x" + 
                   std::to_string(address) + " with data: 0x" + 
                   std::to_string(data));
        return;
    }

    // Echo RAM (0xE000-0xFDFF mirrors 0xC000-0xDDFF)
    if (address >= 0xE000 && address < 0xFE00) {
        WORD mirrorAddr = address - 0x2000;
        m_memory[mirrorAddr] = data;
        LOG_DEBUG("Echo RAM write - Mirror address 0x" + 
                 std::to_string(mirrorAddr) + " <- 0x" + 
                 std::to_string(data));
    }

    // Restricted area (0xFEA0-0xFEFF)
    if (address >= 0xFEA0 && address <= 0xFEFF) {
        LOG_WARNING("Write attempt to restricted area: 0x" + 
                   std::to_string(address) + " with data: 0x" + 
                   std::to_string(data));
        return;
    }

    m_memory[address] = data;
}