#pragma once 
#include <common.h>

#define RAM_SIZE 0x10000  // 64KB of RAM
class RAM{
    private:
        BYTE m_memory [RAM_SIZE];
    public:
        RAM();
        ~RAM();
        BYTE read(WORD address) const;
        void write(WORD address,BYTE data);
};