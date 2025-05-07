#pragma once
#include <memory>
#include "memory_controller.h"
#include "logger.h"


class PPU {
private:
    std::shared_ptr<MemoryController> memoryController;
    int scanlineCounter;
    bool lcdEnabled;
    BYTE currentMode;
    std::array<BYTE, 160 * 144> screenBuffer;


public:
    PPU(std::shared_ptr<MemoryController> memory);
    ~PPU() = default;
    void update(int cycles);

    const std::array<BYTE, 160 * 144>& getScreenBuffer() const { return screenBuffer; }


private:
    bool isLCDEnabled() const;
    void drawScanline();
    void requestVBlankInterrupt();
    void updateScanline();
    void updateLCDStatus();
    void setLCDStatus(BYTE mode);
    void checkLYCInterrupt();
    void renderTiles();
    void renderSprites();
    int getColorFromPalette(BYTE palette, int colorId);
};