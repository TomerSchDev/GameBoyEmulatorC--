#pragma once
#include <common.h>
#include <cstring>
#include <string>

// Cart memory constants
constexpr WORD HEADER_OFFSET = 0x100;
constexpr WORD HEADER_START = 0x0134;
constexpr WORD HEADER_END = 0x014C;
constexpr BYTE CHECKSUM_MASK = 0xFF;

#pragma pack(push, 1)  // Ensure proper struct alignment
typedef struct {
    BYTE entry[4];
    BYTE logo[0x30];
    char title[16];
    WORD new_lic_code;
    BYTE sgb_flag;
    BYTE type;
    BYTE rom_size;
    BYTE ram_size;
    BYTE dest_code;
    BYTE lic_code;
    BYTE version;
    BYTE checksum;
    WORD global_checksum;
} rom_header;
#pragma pack(pop)

class Cart {
    public:
        Cart();
        ~Cart();
        bool load(const std::string& cart);  
        bool unload();
        bool isLoaded();
        BYTE read(WORD address);
        void write(WORD address,BYTE data);
        /*
        // ...existing commented methods...
        */
    private:
        //rom_header header;
        bool loaded;
        BYTE m_CartridgeMemory[0x200000];
        //cart_context ctx;
    };