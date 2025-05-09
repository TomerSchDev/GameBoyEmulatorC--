#pragma once
#include "common.h" // Assuming common.h defines BYTE, WORD etc.
#include <memory>
#include "memory_controller.h"
#include "logger.h"
#include <array>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
class MemoryController; // Forward declaration of MemoryController class
class PPU {
private:
    std::shared_ptr<MemoryController> memoryController;
    int scanlineCounter;
    bool lcdEnabled;
    BYTE currentMode;
    std::vector<Uint32> screenBuffer;
    std::mutex bufferMutex;
    BYTE prevLCDControl = 0;
    BYTE prevBGP = 0;
    bool frameRendered = false;
    int frameCount = 0;
    

public:
    explicit PPU(std::shared_ptr<MemoryController> memory);  // Add explicit keyword
    ~PPU() = default;
    void update(int cycles);
    bool isLCDEnabled() const;
    
    const std::vector<Uint32>& getScreenBuffer() const {
        return screenBuffer;
    }
    void debugFillTestPattern();
    void reset() ; // Reset the PPU state


private:
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
    uint32_t calculateBufferChecksum();
    void monitorRegisterChanges();
    Uint32 mapColorToSDL(int r, int g, int b, int a = 255);
};