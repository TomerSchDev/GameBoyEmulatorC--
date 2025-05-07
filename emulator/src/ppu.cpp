#include "ppu.h"
PPU::PPU(std::shared_ptr<MemoryController> memory)
    : memoryController(memory)
    , scanlineCounter(SCANLINE_CYCLES)
    , lcdEnabled(true)
    , currentMode(MODE_HBLANK)  // Start in HBlank mode
{
    LOG_INFO("PPU initialized");
}

void PPU::update(int cycles) {
    if (!isLCDEnabled()) {
        setLCDStatus(MODE_VBLANK);  // Set to mode 1 when LCD disabled
        scanlineCounter = SCANLINE_CYCLES;
        memoryController->write(LY_REGISTER, 0);
        return;
    }

    scanlineCounter -= cycles;

    // Update LCD status before potentially changing scanline
    updateLCDStatus();

    if (scanlineCounter <= 0) {
        scanlineCounter = SCANLINE_CYCLES;
        updateScanline();
    }
}
void PPU::updateLCDStatus() {
    BYTE currentLine = memoryController->read(LY_REGISTER);

    if (currentLine >= VISIBLE_SCANLINES) {
        // In VBlank
        setLCDStatus(MODE_VBLANK);
        return;
    }

    // Determine mode based on remaining scanline cycles
    if (scanlineCounter >= SCANLINE_CYCLES - MODE_2_CYCLES) {
        // First 80 cycles - OAM scan
        setLCDStatus(MODE_OAM);
    }
    else if (scanlineCounter >= SCANLINE_CYCLES - (MODE_2_CYCLES + MODE_3_CYCLES)) {
        // Next 172 cycles - Pixel transfer
        setLCDStatus(MODE_TRANSFER);
    }
    else {
        // Remaining 204 cycles - HBlank
        setLCDStatus(MODE_HBLANK);
    }

    // Check LYC interrupt
    checkLYCInterrupt();
}
void PPU::checkLYCInterrupt() {
    BYTE currentLine = memoryController->read(LY_REGISTER);
    BYTE compareValue = memoryController->read(LYC_REGISTER);
    BYTE status = memoryController->read(STAT_REGISTER);

    // Set/clear coincidence flag (bit 2)
    if (currentLine == compareValue) {
        status |= 0x04;  // Set coincidence flag
        if (status & STAT_LYC_INT) {  // If LYC interrupt enabled
            BYTE flags = memoryController->read(IF_REGISTER);
            flags |= LCD_INTERRUPT_BIT;
            memoryController->write(IF_REGISTER, flags);
            LOG_DEBUG("LYC interrupt requested");
        }
    } else {
        status &= ~0x04;  // Clear coincidence flag
    }

    memoryController->write(STAT_REGISTER, status);
}

bool PPU::isLCDEnabled() const {
    return memoryController->read(LCD_CONTROL) & LCD_ENABLE_BIT;
}
void PPU::setLCDStatus(BYTE mode) {
    if (mode == currentMode) {
        return;  // No mode change
    }

    BYTE oldStatus = memoryController->read(STAT_REGISTER);
    BYTE newStatus = (oldStatus & 0xFC) | mode;  // Update mode bits
    memoryController->write(STAT_REGISTER, newStatus);
    currentMode = mode;

    // Check if we should request LCD STAT interrupt
    bool requestInterrupt = false;
    
    switch (mode) {
        case MODE_HBLANK:
            requestInterrupt = oldStatus & STAT_HBLANK_INT;
            break;
        case MODE_VBLANK:
            requestInterrupt = oldStatus & STAT_VBLANK_INT;
            break;
        case MODE_OAM:
            requestInterrupt = oldStatus & STAT_OAM_INT;
            break;
    }

    if (requestInterrupt) {
        BYTE flags = memoryController->read(IF_REGISTER);
        flags |= LCD_INTERRUPT_BIT;
        memoryController->write(IF_REGISTER, flags);
        LOG_DEBUG("LCD STAT interrupt requested for mode: " + std::to_string(mode));
    }
}
void PPU::updateScanline() {
    // Get current scanline and increment
    BYTE currentLine = memoryController->read(LY_REGISTER);
    currentLine++;
    memoryController->write(LY_REGISTER, currentLine);

    // Handle different scanline regions
    if (currentLine == VISIBLE_SCANLINES) {
        requestVBlankInterrupt();
    }
    else if (currentLine >= TOTAL_SCANLINES) {
        memoryController->write(LY_REGISTER, 0);
        LOG_DEBUG("Reset to scanline 0");
    }
    else if (currentLine < VISIBLE_SCANLINES) {
        drawScanline();
    }
}

void PPU::requestVBlankInterrupt() {
    BYTE flags = memoryController->read(IF_REGISTER);
    flags |= VBLANK_INTERRUPT_BIT;
    memoryController->write(IF_REGISTER, flags);
    LOG_DEBUG("VBlank interrupt requested");
}

void PPU::drawScanline() {
    BYTE control = memoryController->read(LCD_CONTROL);

    if (control & 0x01) {  // Bit 0 - BG Display
        renderTiles();
    }

    if (control & 0x02) {  // Bit 1 - OBJ (Sprite) Display Enable
        renderSprites();
    }
}
void PPU::renderTiles() {
    BYTE lcdControl = memoryController->read(LCD_CONTROL);

    // Get scroll and window positions
    BYTE scrollY = memoryController->read(SCY_REGISTER);
    BYTE scrollX = memoryController->read(SCX_REGISTER);
    BYTE windowY = memoryController->read(WY_REGISTER);
    BYTE windowX = memoryController->read(WX_REGISTER);

    // Get tile map and data addresses
    WORD bgTileMapAddress = (lcdControl & 0x08) ? BG_TILE_MAP_2 : BG_TILE_MAP_1;
    WORD tileDataAddress = (lcdControl & 0x10) ? TILE_DATA_1 : TILE_DATA_2;
    bool signedTileData = !(lcdControl & 0x10);

    // Get current scanline
    BYTE currentLine = memoryController->read(LY_REGISTER);

    // Get background palette
    BYTE bgPalette = memoryController->read(BGP_REGISTER);

    // Iterate through pixels for the current scanline
    for (int x = 0; x < 160; x++) {
        // Calculate tile coordinates
        WORD tileX = (x + scrollX) / 8;
        WORD tileY = (currentLine + scrollY) / 8;
        WORD tileOffset = tileY * 32 + tileX;

        // Read tile identifier from tile map
        BYTE tileIdentifier = memoryController->read(bgTileMapAddress + tileOffset);

        // Calculate tile data address
        WORD tileData;
        if (signedTileData) {
            // Signed tile identifier
            SIGNED_BYTE signedId = (SIGNED_BYTE)tileIdentifier;
            tileData = TILE_DATA_2 + ((signedId + 128) * TILE_SIZE);
        } else {
            // Unsigned tile identifier
            tileData = TILE_DATA_1 + (tileIdentifier * TILE_SIZE);
        }

        // Calculate pixel offset within the tile
        int pixelX = (x + scrollX) % 8;
        int pixelY = (currentLine + scrollY) % 8;
        WORD pixelOffset = pixelY * 2;

        // Read tile data (2 bytes per line)
        BYTE byte1 = memoryController->read(tileData + pixelOffset);
        BYTE byte2 = memoryController->read(tileData + pixelOffset + 1);

        // Extract color ID for the pixel
        int colorBit = 7 - pixelX;
        int colorId = ((byte2 >> colorBit) & 0x01) << 1 | ((byte1 >> colorBit) & 0x01);

        // Map color ID to color using the palette
        int color = getColorFromPalette(bgPalette, colorId);

        // Placeholder for writing pixel to screen buffer
        LOG_DEBUG("Pixel color: " + std::to_string(color));
        // Convert grayscale color to RGBA8888
        Uint32 rgbaColor = (color << 24) | (color << 16) | (color << 8) | 0xFF;

        // Write pixel to screen buffer
        int bufferIndex = currentLine * 160 + x;
        if (bufferIndex >= 0 && bufferIndex < screenBuffer.size()) {
            screenBuffer[bufferIndex] = rgbaColor;
        }
    
    }
}
int PPU::getColorFromPalette(BYTE palette, int colorId) {
    int color = 0;
    switch (colorId) {
        case 0: color = (palette & 0x03); break;          // Bits 1-0
        case 1: color = (palette >> 2) & 0x03; break;   // Bits 3-2
        case 2: color = (palette >> 4) & 0x03; break;   // Bits 5-4
        case 3: color = (palette >> 6) & 0x03; break;   // Bits 7-6
    }

    // Map color values to grayscale
    switch (color) {
        case 0: return 255;   // White
        case 1: return 192;   // Light gray
        case 2: return 96;    // Dark gray
        case 3: return 0;     // Black
    }
    return 0;s
}
void PPU::renderSprites() {
    // Placeholder for sprite rendering logic
    BYTE currentLine = memoryController->read(LY_REGISTER);
    LOG_DEBUG("Rendering sprites for scanline: " + std::to_string(currentLine));
}