#pragma once
#include <common.h>
#include <string>
#include <vector>
// ROM header structure
struct rom_header { // You can rename this back to rom_header if you replace your old one
    uint8_t entry_point[4];        // ROM Addresses: 0x0100 - 0x0103
                                   // Usually NOP followed by JP to main program

    uint8_t nintendo_logo[48];     // ROM Addresses: 0x0104 - 0x0133
                                   // Bitmap of the Nintendo logo, checked by boot ROM

    char    title[15];             // ROM Addresses: 0x0134 - 0x0142
                                   // Game title (15 characters max).
                                   // For CGB, first 11 chars (0x134-0x13E) are title,
                                   // next 4 (0x13F-0x142) are manufacturer code.
                                   // Padded with 0x00 if shorter.

    uint8_t cgb_flag;              // ROM Address:   0x0143
                                   // Game Boy Color flag:
                                   // 0x80 = CGB compatible (works on DMG & CGB)
                                   // 0xC0 = CGB only
                                   // 0x00 or other = DMG game

    uint8_t new_licensee_code[2];  // ROM Addresses: 0x0144 - 0x0145
                                   // Two ASCII characters for newer licensee code.
                                   // Used if old_licensee_code (0x14B) is 0x33.

    uint8_t sgb_flag;              // ROM Address:   0x0146
                                   // Super Game Boy flag:
                                   // 0x03 = SGB functions supported
                                   // 0x00 = No SGB functions

    uint8_t type;                  // ROM Address:   0x0147
                                   // Cartridge type (MBC type, RAM, battery, etc.)

    uint8_t rom_size;              // ROM Address:   0x0148
                                   // ROM size code (Value N means 32KB << N)

    uint8_t ram_size;              // ROM Address:   0x0149
                                   // External RAM size code (if any)

    uint8_t destination_code;      // ROM Address:   0x014A
                                   // 0x00 = Japanese market
                                   // 0x01 = Non-Japanese market

    uint8_t old_licensee_code;     // ROM Address:   0x014B
                                   // Older licensee code. If 0x33, use new_licensee_code.

    uint8_t version;               // ROM Address:   0x014C
                                   // Mask ROM version number (usually 0x00)

    uint8_t checksum;              // ROM Address:   0x014D
                                   // Header checksum. Calculated over ROM 0x0134-0x014C.
                                   // This field will now correctly point to this address.

    uint16_t global_checksum;      // ROM Addresses: 0x014E - 0x014F
                                   // 16-bit sum of all bytes in ROM (except these two).
                                   // Stored little-endian (LSB at 0x14E, MSB at 0x14F - *Correction*: Pan Docs says LSB is at 0x014E, MSB at 0x014F, but then some sources say global checksum is MSB then LSB in file. Let's stick to standard struct layout and little-endian for WORDs. The common representation is that 0x14E holds the MSB and 0x14F holds the LSB for the 16-bit value if read as two bytes, but if it's a uint16_t field in a struct on a little-endian system, the bytes would be LSB then MSB in memory. Given Game Boy is little-endian, a `uint16_t` read from `0x14E` would mean `0x14E` is LSB, `0x14F` is MSB. Let's assume it's read as a little-endian WORD from address `0x14E`.)
                                   // Ok, standard for GB is `0x014E`=MSB, `0x014F`=LSB. So if we define it as `uint16_t global_checksum`, to read it correctly you'd do:
                                   // `(rom_data[0x014E] << 8) | rom_data[0x014F]`.
                                   // If this struct field is used on a little-endian machine, it will read `rom_data[0x014E]` as LSB and `rom_data[0x014F]` as MSB.
                                   // To make the struct field itself directly usable as the combined value on a little-endian system, while acknowledging the GB storage convention, it might be better to keep it as uint16_t and ensure your loading mechanism or access handles the GB's specific MSB/LSB order if it differs from your system's native uint16_t memory layout.
                                   // Simpler to define as `uint8_t global_checksum_bytes[2];` and combine manually.
                                   // However, for direct casting, `uint16_t` is common and assumes the system matches GB's effective endianness for this field or that it's handled.
                                   // Let's keep uint16_t for simplicity, assuming it's typically read as a little-endian value from 0x14E.
                                   // If ROM stores MSB then LSB: `uint8_t global_checksum_msb; uint8_t global_checksum_lsb;` might be safer for struct overlay.
                                   // Pan Docs: "014E-014F - Global Checksum (UWORD)" - usually implies little-endian for UWORD.
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
    BYTE calculate_gameboy_header_checksum(const BYTE* cartridge_memory);
};