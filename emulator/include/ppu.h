#pragma once
#include <memory>
#include "memory_controller.h"
#include "logger.h"
#include <array>
class MemoryController; // Forward declaration of MemoryController class
class PPU {
private:
    std::shared_ptr<MemoryController> memoryController;
    int scanlineCounter;
    bool lcdEnabled;
    BYTE currentMode;
    std::array<Uint32, SCREEN_PIXELS_WIDTH * SCREEN_PIXELS_HEIGHT> screenBuffer;

public:
    explicit PPU(std::shared_ptr<MemoryController> memory);  // Add explicit keyword
    ~PPU() = default;
    void update(int cycles);

    const std::array<Uint32, SCREEN_PIXELS_WIDTH * SCREEN_PIXELS_HEIGHT>& getScreenBuffer() const { return screenBuffer; }


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
    void setPixel(int x, int y, Uint32 color);
    int getColorFromPalette(BYTE palette, int colorId);
};