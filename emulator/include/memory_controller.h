#pragma once

#include "logger.h"
#include <common.h>
#include <vector>
#include <memory_region.h>
#include <ram.h>
#include <cart.h>
#include <memory>
#include <emulator.h>
class Emulator; // Forward declaration of Emulator class


class MemoryController {
    private:
        std::unique_ptr<RAM> ram;
        std::unique_ptr<Cart> cart;
        Emulator* emulator; // Pointer to the Emulator instance

        // Banking related members
        bool m_EnableRAM;
        bool m_ROMBanking;
        bool m_MBC1;
        bool m_MBC2;
        BYTE m_CurrentROMBank;
        BYTE m_CurrentRAMBank;
        std::vector<BYTE> m_RAMBanks;

    public:
        MemoryController();
        ~MemoryController() = default;

        // Memory access
        BYTE read(WORD address) const;
        void write(WORD address, BYTE data);
        void doDMATransfer(BYTE data);
        
        // Direct VRAM access for PPU
        const BYTE getVRAM() const { 
            if (!ram) {
                LOG_ERROR("Attempting to access VRAM with null RAM");
                return 0;
            }
            return ram->read(0x8000); 
        }
        
        const BYTE getOAM() const { 
            if (!ram) {
                LOG_ERROR("Attempting to access OAM with null RAM");
                return 0;
            }
            return ram->read(0xFE00); 
        }
        
        // Cart management
        bool attachCart(std::unique_ptr<Cart> newCart);
        bool detachCart();
        bool hasCart() const { return cart != nullptr && cart->isLoaded(); }
        bool attachEmulator(Emulator* newEmulator) {
            if (!newEmulator) {
                LOG_ERROR("Attempting to attach null emulator instance");
                return false;
            }
            if (emulator) {
                LOG_WARNING("Attempting to attach emulator while another is present");
                return false;
            }
            emulator = newEmulator;
            return emulator != nullptr;
        }


    private:
        // Banking helpers
        void HandleBanking(WORD address, BYTE data);
        void DoRamBankEnable(WORD address, BYTE data);
        void DoChangeLoROMBank(BYTE data);
        void DoChangeHiRomBank(BYTE data);
        void DoRAMBankChange(BYTE data);
        void DoChangeROMRAMMode(BYTE data);
};