#pragma once
#include <string>

enum class MemoryRegion {
    ROM_BANK_0,          // 0x0000 - 0x3FFF
    ROM_BANK_N,          // 0x4000 - 0x7FFF
    VRAM,               // 0x8000 - 0x9FFF
    EXTERNAL_RAM,       // 0xA000 - 0xBFFF
    WORK_RAM,           // 0xC000 - 0xDFFF
    ECHO_RAM,           // 0xE000 - 0xFDFF
    SPRITE_TABLE,       // 0xFE00 - 0xFE9F
    RESTRICTED,         // 0xFEA0 - 0xFEFF
    IO_PORTS,           // 0xFF00 - 0xFF7F
    HRAM,              // 0xFF80 - 0xFFFE
    DMA_REGISTER,       // 0xFF46
    INTERRUPT_ENABLE    // 0xFFFF
};

inline MemoryRegion getMemoryRegion(WORD address) {
    if (address <= 0x3FFF) return MemoryRegion::ROM_BANK_0;
    if (address <= 0x7FFF) return MemoryRegion::ROM_BANK_N;
    if (address <= 0x9FFF) return MemoryRegion::VRAM;
    if (address <= 0xBFFF) return MemoryRegion::EXTERNAL_RAM;
    if (address <= 0xDFFF) return MemoryRegion::WORK_RAM;
    if (address <= 0xFDFF) return MemoryRegion::ECHO_RAM;
    if (address <= 0xFE9F) return MemoryRegion::SPRITE_TABLE;
    if (address <= 0xFEFF) return MemoryRegion::RESTRICTED;
    if (address <= 0xFF7F) return MemoryRegion::IO_PORTS;
    if (address <= 0xFFFE) return MemoryRegion::HRAM;
    if (address == DMA_REGISTER) return MemoryRegion::DMA_REGISTER;
    return MemoryRegion::INTERRUPT_ENABLE;
}

inline std::string getMemoryRegionName(MemoryRegion region) {
    switch (region) {
        case MemoryRegion::ROM_BANK_0: return "ROM_BANK_0";
        case MemoryRegion::ROM_BANK_N: return "ROM_BANK_N";
        case MemoryRegion::VRAM: return "VRAM";
        case MemoryRegion::EXTERNAL_RAM: return "EXTERNAL_RAM";
        case MemoryRegion::WORK_RAM: return "WORK_RAM";
        case MemoryRegion::ECHO_RAM: return "ECHO_RAM";
        case MemoryRegion::SPRITE_TABLE: return "SPRITE_TABLE";
        case MemoryRegion::RESTRICTED: return "RESTRICTED";
        case MemoryRegion::IO_PORTS: return "IO_PORTS";
        case MemoryRegion::HRAM: return "HRAM";
        case MemoryRegion::INTERRUPT_ENABLE: return "INTERRUPT_ENABLE";
        case MemoryRegion::DMA_REGISTER: return "DMA_REGISTER";
        default: return "UNKNOWN";
    }
}