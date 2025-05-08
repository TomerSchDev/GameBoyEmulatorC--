#include "ppu.h"
PPU::PPU(std::shared_ptr<MemoryController> memory)
    : memoryController(memory)
    , scanlineCounter(SCANLINE_CYCLES)
    , lcdEnabled(true)
    , currentMode(MODE_HBLANK)  // Start in HBlank mode
{
    LOG_INFO("PPU initialized");
        // Initialize screen buffer to white
    screenBuffer.fill(0xFFFFFFFF);

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


// First, fix the buffer index comparison issue
void PPU::setPixel(int x, int y, Uint32 color) {
    size_t bufferIndex = y * SCREEN_PIXELS_WIDTH + x;
    if (bufferIndex < screenBuffer.size()) {  // Remove redundant >= 0 check since size_t is unsigned
        screenBuffer[bufferIndex] = color;
    }
}

// Now fix the renderTiles function with proper window handling
void PPU::renderTiles() {
    BYTE lcdControl = memoryController->read(LCD_CONTROL);
    BYTE scrollY = memoryController->read(SCY_REGISTER);
    BYTE scrollX = memoryController->read(SCX_REGISTER);
    
    // Get current scanline first
    BYTE currentLine = memoryController->read(LY_REGISTER);
    
    // Window position registers
    BYTE windowY = memoryController->read(WY_REGISTER);
    BYTE windowX = memoryController->read(WX_REGISTER) - 7;  // WX is offset by 7
    bool windowEnabled = (lcdControl & 0x20) && windowY <= currentLine;

    // Determine which tile data area to use
    WORD tileDataArea = (lcdControl & 0x10) ? TILE_DATA_1 : TILE_DATA_2;
    bool unsignedIndexing = (lcdControl & 0x10);

    // Background tile map selection
    WORD bgTileMap = (lcdControl & 0x08) ? BG_TILE_MAP_2 : BG_TILE_MAP_1;
    
    // Window tile map selection
    WORD windowTileMap = (lcdControl & 0x40) ? WINDOW_TILE_MAP_2 : WINDOW_TILE_MAP_1;

    // Draw the 160 pixels for this scanline
    for (int pixel = 0; pixel < SCREEN_PIXELS_WIDTH; pixel++) {
        bool useWindow = windowEnabled && pixel >= windowX;
        
        // Calculate which tile we're currently drawing
        BYTE x = useWindow ? pixel - windowX : (pixel + scrollX);
        BYTE y = useWindow ? currentLine - windowY : (currentLine + scrollY);
        
        // Get the tile map we're using
        WORD tileMap = useWindow ? windowTileMap : bgTileMap;
        
        // Calculate tile coordinates
        BYTE tileX = x / 8;
        BYTE tileY = y / 8;
        
        // Get the tile index from the tile map
        WORD tileMapAddress = tileMap + (tileY * 32) + tileX;
        BYTE tileIndex = memoryController->read(tileMapAddress);

        // Calculate tile data address
        WORD tileDataAddress;
        if (unsignedIndexing) {
            tileDataAddress = tileDataArea + (tileIndex * 16);
        } else {
            tileDataAddress = tileDataArea + ((static_cast<signed char>(tileIndex) + 128) * 16);
        }

        // Get the specific line of the tile we need
        BYTE tileLine = y % 8;
        BYTE tileData1 = memoryController->read(tileDataAddress + (tileLine * 2));
        BYTE tileData2 = memoryController->read(tileDataAddress + (tileLine * 2) + 1);

        // Get the specific pixel
        BYTE pixelBit = 7 - (x % 8);
        BYTE colorNum = ((tileData2 >> pixelBit) & 1) << 1;
        colorNum |= (tileData1 >> pixelBit) & 1;

        // Get the color from the background palette
        BYTE bgPalette = memoryController->read(BGP_REGISTER);
        BYTE color = (bgPalette >> (colorNum * 2)) & 0x03;

        // Convert color to RGBA (adjust these values as needed)
        Uint32 pixelColor;
        switch(color) {
            case 0: pixelColor = 0xFFFFFFFF; break; // White
            case 1: pixelColor = 0xAAAAAA; break;   // Light gray
            case 2: pixelColor = 0x555555; break;   // Dark gray
            case 3: pixelColor = 0x000000; break;   // Black
            default: pixelColor = 0xFF00FF; break;  // Error color (pink)
        }

        setPixel(pixel, currentLine, pixelColor);
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
    return 0;
}
void PPU::renderSprites() {
    BYTE lcdControl = memoryController->read(LCD_CONTROL);
    bool use8x16 = (lcdControl & 0x04) != 0;  // Bit 2: OBJ (Sprite) Size (0=8x8, 1=8x16)
    int spriteSize = use8x16 ? 16 : 8;

    BYTE currentLine = memoryController->read(LY_REGISTER);

    for (int spriteIndex = 0; spriteIndex < MAX_SPRITES; spriteIndex++) {
        // Calculate the address of the sprite's attributes
        WORD spriteAddress = OAM_START + (spriteIndex * SPRITE_ATTRIBUTE_SIZE);

        // Read the sprite's attributes
        BYTE yPos = memoryController->read(spriteAddress + SPRITE_Y_POS) - 16;
        BYTE xPos = memoryController->read(spriteAddress + SPRITE_X_POS) - 8;
        BYTE tileIndex = memoryController->read(spriteAddress + SPRITE_TILE_INDEX);
        BYTE attributes = memoryController->read(spriteAddress + SPRITE_ATTRIBUTES);

        // Check if the sprite is visible on the current scanline
        if (currentLine >= yPos && currentLine < (yPos + spriteSize)) {
            bool yFlip = (attributes & SPRITE_Y_FLIP) != 0;
            bool xFlip = (attributes & SPRITE_X_FLIP) != 0;
            bool paletteNumber = (attributes & SPRITE_PALETTE) != 0;

            // Determine the palette address
            WORD paletteAddress = paletteNumber ? 0xFF49 : 0xFF48;

            // Calculate the line within the sprite
            int line = currentLine - yPos;
            if (yFlip) {
                line = spriteSize - 1 - line;
            }

            // Calculate the address of the tile data
            WORD tileDataAddress = 0x8000 + (tileIndex * 16) + (line * 2);

            // Read the tile data
            BYTE data1 = memoryController->read(tileDataAddress);
            BYTE data2 = memoryController->read(tileDataAddress + 1);

            // Render the sprite pixels
            for (int tilePixel = 0; tilePixel < 8; tilePixel++) {
                int colorBit = tilePixel;
                if (xFlip) {
                    colorBit = 7 - tilePixel;
                }

                // Extract the color number from the tile data
                int colorNum = ((data2 >> (7 - colorBit)) & 0x01) << 1;
                colorNum |= ((data1 >> (7 - colorBit)) & 0x01);

                // Get the color from the palette
                int color = getColorFromPalette(memoryController->read(paletteAddress), colorNum);

                // White is transparent for sprites
                if (color == 255) {
                    continue;
                }

                // Calculate the x position of the pixel
                int xPix = xPos + tilePixel;

                // Check if the pixel is within the screen bounds
                if (currentLine >= 0 && currentLine < 144 && xPix >= 0 && xPix < 160) {
                    // Convert grayscale color to RGBA8888
                    Uint32 rgbaColor = (color << 24) | (color << 16) | (color << 8) | BYTE_MASK;

                    // Write the pixel to the screen buffer
                    screenBuffer[currentLine * 160 + xPix] = rgbaColor;
                }
            }
        }
    }
}