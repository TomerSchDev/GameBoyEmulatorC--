#include <cart.h>
#include <logger.h>
#include <fstream>
#include <string>
#include <ios>
#include <vector>
#include <iomanip>
/*
static const char *ROM_TYPES[] = {
    "ROM ONLY",
    "MBC1",
    "MBC1+RAM",
    "MBC1+RAM+BATTERY",
    "0x04 ???",
    "MBC2",
    "MBC2+BATTERY",
    "0x07 ???",
    "ROM+RAM 1",
    "ROM+RAM+BATTERY 1",
    "0x0A ???",
    "MMM01",
    "MMM01+RAM",
    "MMM01+RAM+BATTERY",
    "0x0E ???",
    "MBC3+TIMER+BATTERY",
    "MBC3+TIMER+RAM+BATTERY 2",
    "MBC3",
    "MBC3+RAM 2",
    "MBC3+RAM+BATTERY 2",
    "0x14 ???",
    "0x15 ???",
    "0x16 ???",
    "0x17 ???",
    "0x18 ???",
    "MBC5",
    "MBC5+RAM",
    "MBC5+RAM+BATTERY",
    "MBC5+RUMBLE",
    "MBC5+RUMBLE+RAM",
    "MBC5+RUMBLE+RAM+BATTERY",
    "0x1F ???",
    "MBC6",
    "0x21 ???",
    "MBC7+SENSOR+RUMBLE+RAM+BATTERY",
}

static const char *LIC_CODE[0xA5] = {
    [0x00] = "None",
    [0x01] = "Nintendo R&D1",
    [0x08] = "Capcom",
    [0x13] = "Electronic Arts",
    [0x18] = "Hudson Soft",
    [0x19] = "b-ai",
    [0x20] = "kss",
    [0x22] = "pow",
    [0x24] = "PCM Complete",
    [0x25] = "san-x",
    [0x28] = "Kemco Japan",
    [0x29] = "seta",
    [0x30] = "Viacom",
    [0x31] = "Nintendo",
    [0x32] = "Bandai",
    [0x33] = "Ocean/Acclaim",
    [0x34] = "Konami",
    [0x35] = "Hector",
    [0x37] = "Taito",
    [0x38] = "Hudson",
    [0x39] = "Banpresto",
    [0x41] = "Ubi Soft",
    [0x42] = "Atlus",
    [0x44] = "Malibu",
    [0x46] = "angel",
    [0x47] = "Bullet-Proof",
    [0x49] = "irem",
    [0x50] = "Absolute",
    [0x51] = "Acclaim",
    [0x52] = "Activision",
    [0x53] = "American sammy",
    [0x54] = "Konami",
    [0x55] = "Hi tech entertainment",
    [0x56] = "LJN",
    [0x57] = "Matchbox",
    [0x58] = "Mattel",
    [0x59] = "Milton Bradley",
    [0x60] = "Titus",
    [0x61] = "Virgin",
    [0x64] = "LucasArts",
    [0x67] = "Ocean",
    [0x69] = "Electronic Arts",
    [0x70] = "Infogrames",
    [0x71] = "Interplay",
    [0x72] = "Broderbund",
    [0x73] = "sculptured",
    [0x75] = "sci",
    [0x78] = "THQ",
    [0x79] = "Accolade",
    [0x80] = "misawa",
    [0x83] = "lozc",
    [0x86] = "Tokuma Shoten Intermedia",
    [0x87] = "Tsukuda Original",
    [0x91] = "Chunsoft",
    [0x92] = "Video system",
    [0x93] = "Ocean/Acclaim",
    [0x95] = "Varie",
    [0x96] = "Yonezawa/s’pal",
    [0x97] = "Kaneko",
    [0x99] = "Pack in soft",
    [0xA4] = "Konami (Yu-Gi-Oh!)"
};

const char *cart_lic_name() {
    if (ctx.header->new_lic_code <= 0xA4) {
        return LIC_CODE[ctx.header->lic_code];
    }

    return "UNKNOWN";
}

const char *cart_type_name() {
    if (ctx.header->type <= 0x22) {
        return ROM_TYPES[ctx.header->type];
    }

    return "UNKNOWN";
}

bool cart_load(char *cart) {
    snprintf(ctx.filename, sizeof(ctx.filename), "%s", cart);

    FILE *fp = fopen(cart, "r");

    if (!fp) {
        printf("Failed to open: %s\n", cart);
        return false;
    }

    printf("Opened: %s\n", ctx.filename);

    fseek(fp, 0, SEEK_END);
    ctx.rom_size = ftell(fp);

    rewind(fp);

    ctx.rom_data = malloc(ctx.rom_size);
    fread(ctx.rom_data, ctx.rom_size, 1, fp);
    fclose(fp);

    ctx.header = (rom_header *)(ctx.rom_data + 0x100);
    ctx.header->title[15] = 0;

    printf("Cartridge Loaded:\n");
    printf("\t Title    : %s\n", ctx.header->title);
    printf("\t Type     : %2.2X (%s)\n", ctx.header->type, cart_type_name());
    printf("\t ROM Size : %d KB\n", 32 << ctx.header->rom_size);
    printf("\t RAM Size : %2.2X\n", ctx.header->ram_size);
    printf("\t LIC Code : %2.2X (%s)\n", ctx.header->lic_code, cart_lic_name());
    printf("\t ROM Vers : %2.2X\n", ctx.header->version);

    u16 x = 0;
    for (u16 i=0x0134; i<=0x014C; i++) {
        x = x - ctx.rom_data[i] - 1;
    }

    printf("\t Checksum : %2.2X (%s)\n", ctx.header->checksum, (x & BYTE_MASK) ? "PASSED" : "FAILED");

    return true;
}
*/

Cart::Cart()
{
    this->loaded=false;
}

Cart::~Cart()
{
    this->unload();
    this->m_CartridgeMemory.clear();
}

bool Cart::load(const std::string &filename)
{
    LOG_INFO("Attempting to load ROM: " + filename);

    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        LOG_ERROR("Failed to open ROM file: " + filename);
        return false;
    }

    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    if (fileSize <= 0) {
        LOG_ERROR("Invalid ROM file size: " + std::to_string(fileSize));
        loaded = false;
        return false;
    }
    LOG_INFO("Expected ROM file size: " + std::to_string(fileSize) + " bytes"); // Log expected size
    // Check if ROM size is valid
    m_CartridgeMemory.resize(fileSize);
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(m_CartridgeMemory.data()), fileSize);

    std::streamsize bytesRead = file.gcount(); // Get actual bytes read
    LOG_INFO("Bytes actually read from ROM: " + std::to_string(bytesRead)); // Log actual bytes read

    if (!file || bytesRead != fileSize) { // Check if read was successful and complete
        LOG_ERROR("Failed to read ROM file completely: " + filename + " (Read " + std::to_string(bytesRead) + " bytes)");
        // Optionally clear memory or handle error more robustly
        m_CartridgeMemory.assign(m_CartridgeMemory.size(), 0xFF); // Fill with a known value on error?
        loaded = false;
        return false;
    }
     LOG_INFO("ROM file read successfully.");
    // Get ROM header information (starting at 0x100)
    const rom_header* header = reinterpret_cast<const rom_header*>(m_CartridgeMemory.data() + 0x100);
    
    // Log cartridge info
    std::string title(header->title, 16);
    LOG_INFO("Cartridge loaded successfully:");
    LOG_INFO("Title: " + title);
    LOG_INFO("ROM Size: " + std::to_string(32 << header->rom_size) + "KB");
    LOG_INFO("ROM Version: " + std::to_string(header->version));

    // Check for checksum
    BYTE checksum = calculate_gameboy_header_checksum(m_CartridgeMemory);
    if (checksum != header->checksum) {
        LOG_WARNING("Checksum mismatch: expected " + std::to_string(header->checksum) + ", calculated " + std::to_string(checksum));
    } else {
        LOG_INFO("Checksum verified successfully");
    }

    loaded = true;
    LOG_INFO("Cartridge loaded successfully");
    LOG_DEBUG("Writing rom data to file for debugging");
    std::ofstream debugFile("rom_dump.txt", std::ios::binary);
    //translate to hex
    for (size_t i = 0; i < m_CartridgeMemory.size(); ++i) {
        debugFile << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(m_CartridgeMemory[i]) << " ";
        if ((i + 1) % 16 == 0) {
            debugFile << "\n";
        }
    }
    debugFile.close();
    LOG_INFO("ROM data written to rom_dump.txt for debugging purposes.");
    return true;
}
BYTE Cart::calculate_gameboy_header_checksum(const std::vector<unsigned char>& cartridge_memory)const {
    BYTE checksum = 0; // BYTE is unsigned char, 8-bit unsigned arithmetic
    for (size_t address = 0x0134; address <= 0x014C; ++address) {
        checksum = checksum - cartridge_memory[address] - 1;
    }
    return checksum;
}

bool Cart::unload() {
    if (!loaded) {
        LOG_WARNING("Attempting to unload a cart that isn't loaded");
        return true;
    }

    // Clear cartridge memory
    m_CartridgeMemory.clear();
    loaded = false;
    
    LOG_INFO("Cartridge unloaded successfully");
    return true;
}

void Cart::initBanking() {
    if (!loaded) {
        LOG_ERROR("Cannot initialize banking: ROM not loaded");
        return;
    }
    // Initialize banking registers
    BYTE* cartridgeMemory = m_CartridgeMemory.data();
    rom_header* header = reinterpret_cast<rom_header*>(cartridgeMemory + 0x100);
    cartridgeType = header->type;
    
    // Initialize banking registers
    currentROMBank = 1;
    currentRAMBank = 0;
    ramEnabled = false;
    romBankingMode = true;

    // Determine cartridge features
    switch(cartridgeType) {
        case 0x00: // ROM ONLY
            hasRAM = false;
            hasBattery = false;
            break;
        case 0x01: // MBC1
            hasRAM = false;
            hasBattery = false;
            break;
        case 0x02: // MBC1+RAM
        case 0x03: // MBC1+RAM+BATTERY
            hasRAM = true;
            hasBattery = (cartridgeType == 0x03);
            break;
        // ... add more types as needed
    }
}

BYTE Cart::read(WORD address) {
    if (!loaded || address >= m_CartridgeMemory.size()) {
        LOG_ERROR("Cartridge read error: Address 0x" + std::to_string(address) + " out of bounds or ROM not loaded.");
        return 0xFF; // Return a default value on error
    }
    BYTE value;
    switch(cartridgeType) {
        case 0x00: // ROM ONLY
            value = readROMOnly(address);
            LOG_DEBUG("Read from ROM ONLY: Address 0x" + std::to_string(address) + " Value: 0x" + std::to_string(value));
            return value;
        case 0x01: // MBC1
        case 0x02: // MBC1+RAM
        case 0x03: // MBC1+RAM+BATTERY
            value = readMBC1(address);
            LOG_DEBUG("Read from MBC1: Address 0x" + std::to_string(address) + " Value: 0x" + std::to_string(value));
            return value;
        default:
            LOG_WARNING("Unsupported cartridge type: " + std::to_string(cartridgeType));
            return 0xFF;
    }
}

void Cart::write(WORD address, BYTE data) {
    if (!loaded) return;

    switch(cartridgeType) {
        case 0x00: // ROM ONLY
            writeROMOnly(address, data);
            break;
        case 0x01: // MBC1
        case 0x02: // MBC1+RAM
        case 0x03: // MBC1+RAM+BATTERY
            writeMBC1(address, data);
            break;
        default:
            LOG_WARNING("Unsupported cartridge type: " + std::to_string(cartridgeType));
            break;
    }
}

BYTE Cart::readROMOnly(WORD address) {
    if (address < 0x8000) {
        return m_CartridgeMemory[address];
    }
    return 0xFF;
}

void Cart::writeROMOnly(WORD address, BYTE data) {
    // ROM only cartridges can't be written to
    return;
}

BYTE Cart::readMBC1(WORD address) {
    if (address < 0x4000) {
        // ROM Bank 0
        return m_CartridgeMemory[address];
    }
    else if (address < 0x8000) {
        // ROM Bank 1-127
        WORD bankAddress = (address - 0x4000) + (currentROMBank * 0x4000);
        return m_CartridgeMemory[bankAddress];
    }
    else if (address >= 0xA000 && address < 0xC000) {
        // RAM Banks
        if (ramEnabled && hasRAM) {
            WORD ramAddress = (address - 0xA000) + (currentRAMBank * 0x2000);
            return m_CartridgeRAM[ramAddress];
        }
    }
    return 0xFF;
}

void Cart::writeMBC1(WORD address, BYTE data) {
    if (address < 0x2000) {
        // RAM Enable
        ramEnabled = ((data & 0x0F) == 0x0A);
    }
    else if (address < 0x4000) {
        // ROM Bank Number
        BYTE bank = data & 0x1F;
        if (bank == 0) bank = 1;
        currentROMBank = (currentROMBank & 0x60) | bank;
    }
    else if (address < 0x6000) {
        // RAM Bank Number or Upper ROM Bank Number
        if (romBankingMode) {
            currentROMBank = (currentROMBank & 0x1F) | ((data & 0x03) << 5);
        } else {
            currentRAMBank = data & 0x03;
        }
    }
    else if (address < 0x8000) {
        // ROM/RAM Mode Select
        romBankingMode = !(data & 0x01);
    }
    else if (address >= 0xA000 && address < 0xC000) {
        // RAM Banks
        if (ramEnabled && hasRAM) {
            WORD ramAddress = (address - 0xA000) + (currentRAMBank * 0x2000);
            m_CartridgeRAM[ramAddress] = data;
        }
    }
}
// ...existing code...

BYTE Cart::readMBC2(WORD address) {
    if (address < 0x4000) {
        // ROM Bank 0
        return m_CartridgeMemory[address];
    }
    else if (address < 0x8000) {
        // ROM Bank 1-15
        WORD bankAddress = (address - 0x4000) + (currentROMBank * 0x4000);
        return m_CartridgeMemory[bankAddress];
    }
    else if (address >= 0xA000 && address < 0xA200) {
        // MBC2 has built-in RAM of 512 x 4 bits
        if (ramEnabled) {
            return m_CartridgeRAM[address - 0xA000] & 0x0F;
        }
    }
    return 0xFF;
}

void Cart::writeMBC2(WORD address, BYTE data) {
    if (address < 0x2000) {
        // RAM Enable (lower bit of upper address byte must be 0)
        if (!(address & 0x0100)) {
            ramEnabled = ((data & 0x0F) == 0x0A);
        }
    }
    else if (address < 0x4000) {
        // ROM Bank Number (lower bit of upper address byte must be 1)
        if (address & 0x0100) {
            currentROMBank = data & 0x0F;
            if (currentROMBank == 0) currentROMBank = 1;
        }
    }
    else if (address >= 0xA000 && address < 0xA200) {
        // RAM Banks
        if (ramEnabled) {
            m_CartridgeRAM[address - 0xA000] = data & 0x0F;
        }
    }
}

bool Cart::verifyChecksum() const {
    // Use const_cast to maintain const correctness
    BYTE *romData = const_cast<BYTE*>(m_CartridgeMemory.data());
    const rom_header* header = reinterpret_cast<const rom_header*>( romData+ 0x100);
    WORD checksum = 0;
    
    for (WORD addr = HEADER_START; addr <= HEADER_END; addr++) {
        checksum = checksum - m_CartridgeMemory[addr] - 1;
    }

    return (checksum & CHECKSUM_MASK) == header->checksum;
}

void Cart::saveRAM() const {
    if (!hasBattery || !hasRAM || !loaded) {
        return;
    }

    // Create save filename from ROM filename
    std::string saveFile = "save.ram";  // You might want to derive this from the cartridge title
    
    std::ofstream file(saveFile, std::ios::binary);
    if (!file) {
        LOG_ERROR("Failed to create save file: " + saveFile);
        return;
    }
    // Save RAM data
    
    file.write(reinterpret_cast<const char*>(m_CartridgeRAM.data()), m_CartridgeRAM.size());
    if (file.fail()) {
        LOG_ERROR("Failed writing save data");
    } else {
        LOG_INFO("Successfully saved RAM to: " + saveFile);
    }
}

void Cart::loadRAM() {
    if (!hasBattery || !hasRAM || !loaded) {
        return;
    }

    // Load from save filename
    std::string saveFile = "save.ram";  // Should match saveRAM()
    
    std::ifstream file(saveFile, std::ios::binary);
    if (!file) {
        LOG_WARNING("No save file found: " + saveFile);
        return;
    }

    file.read(reinterpret_cast<char*>(m_CartridgeRAM.data()), m_CartridgeRAM.size());
    if (file.fail()) {
        LOG_ERROR("Failed reading save data");
    } else {
        LOG_INFO("Successfully loaded RAM from: " + saveFile);
    }
}