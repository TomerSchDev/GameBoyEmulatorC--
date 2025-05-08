#pragma once
#include <common.h>
#include <string>
// ROM header structure
struct rom_header {
    BYTE entry_point[4];     // 0x100 - 0x103
    BYTE nintendo_logo[48];  // 0x104 - 0x133
    char title[16];          // 0x134 - 0x143
    BYTE cgb_flag;          // 0x143
    WORD new_licensee_code; // 0x144 - 0x145
    BYTE sgb_flag;          // 0x146
    BYTE type;              // 0x147
    BYTE rom_size;          // 0x148
    BYTE ram_size;          // 0x149
    BYTE destination_code;  // 0x14A
    BYTE old_licensee_code; // 0x14B
    BYTE version;           // 0x14C
    BYTE checksum;          // 0x14D
    WORD global_checksum;   // 0x14E - 0x14F
};
class Cart {
private:
    static constexpr size_t MAX_ROM_SIZE = 0x200000;  // 2MB
    static constexpr size_t MAX_RAM_SIZE = 0x20000;   // 128KB
    static constexpr WORD HEADER_START = 0x0134;
    static constexpr WORD HEADER_END = 0x014C;
    static constexpr WORD CHECKSUM_MASK = 0xFF;

    // Memory bank controller state
    BYTE currentROMBank;
    BYTE currentRAMBank;
    bool ramEnabled;
    bool romBankingMode;
    
    // Cartridge memory
    BYTE m_CartridgeMemory[MAX_ROM_SIZE];
    BYTE m_CartridgeRAM[MAX_RAM_SIZE];
    bool loaded;

    // Cartridge type info
    BYTE cartridgeType;
    bool hasRAM;
    bool hasBattery;

public:
    Cart();
    ~Cart();

    // Cart management
    bool load(const std::string& filename);
    bool unload();
    bool isLoaded() const { return loaded; }
    bool hasBatteryBackup() const { return hasBattery; }
    
    // Memory access
    BYTE getCartridgeType() const { return cartridgeType; }
    BYTE read(WORD address);
    void write(WORD address, BYTE data);

private:
    void initBanking();
    BYTE readMBC1(WORD address);
    void writeMBC1(WORD address, BYTE data);
    BYTE readMBC2(WORD address);
    void writeMBC2(WORD address, BYTE data);
    BYTE readROMOnly(WORD address);
    void writeROMOnly(WORD address, BYTE data);
    bool verifyChecksum() const;
    void saveRAM() const;
    void loadRAM();
};