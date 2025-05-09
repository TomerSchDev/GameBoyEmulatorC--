#include "ppu.h"
#include <cstring> // For memset
#include <sstream> // For logging
#include <iomanip> // For std::hex

// Define register addresses if not in common.h or ppu.h
#define LCD_CONTROL 0xFF40
#define STAT_REGISTER 0xFF41
#define SCY_REGISTER 0xFF42
#define SCX_REGISTER 0xFF43
#define LY_REGISTER 0xFF44
#define LYC_REGISTER 0xFF45
#define BGP_REGISTER 0xFF47
#define OBP0_REGISTER 0xFF48
#define OBP1_REGISTER 0xFF49
#define WY_REGISTER 0xFF4A
#define WX_REGISTER 0xFF4B
#define IF_REGISTER 0xFF0F // Interrupt Flag

// Define interrupt bits if not in common.h or ppu.h
#define VBLANK_INTERRUPT_BIT 0x01
#define LCD_INTERRUPT_BIT    0x02

// Define STAT register bits
#define STAT_LYC_EQ_LY  0x04 // Bit 2: LYC=LY Flag
#define STAT_HBLANK_INT 0x08 // Bit 3: Mode 0 HBlank Interrupt Enable
#define STAT_VBLANK_INT 0x10 // Bit 4: Mode 1 VBlank Interrupt Enable
#define STAT_OAM_INT    0x20 // Bit 5: Mode 2 OAM Interrupt Enable
#define STAT_LYC_INT    0x40 // Bit 6: LYC=LY Coincidence Interrupt Enable

// Define LCDC register bits
#define LCD_ENABLE_BIT 0x80 // Bit 7: LCD Display Enable


PPU::PPU(std::shared_ptr<MemoryController> memory)
    : memoryController(memory),
      scanlineCounter(SCANLINE_CYCLES), // Start with full cycles for a scanline
      currentMode(MODE_OAM), // PPU starts in Mode 2 (OAM Scan) after power on
      screenBuffer(SCREEN_PIXELS_WIDTH * SCREEN_PIXELS_HEIGHT, 0xFFFFFFFF), // Initialize buffer
      frameRendered(false),
      frameCount(0),
      prevLCDControl(0), // Initialize previous states
      prevBGP(0)
{
    LOG_INFO("PPU initialized");
    // Initial PPU state often involves setting LY=0 and starting in Mode 2
    memoryController->write(LY_REGISTER, 0);
    setLCDStatus(MODE_OAM); // Explicitly set initial mode and check interrupts
}

void PPU::reset() {
    scanlineCounter = SCANLINE_CYCLES; // Reset counter
    currentMode = MODE_OAM; // Reset to initial mode
    screenBuffer.assign(SCREEN_PIXELS_WIDTH * SCREEN_PIXELS_HEIGHT, 0xFFFFFFFF); // Reset to white
    frameRendered = false;
    frameCount = 0;
    prevLCDControl = 0;
    prevBGP = 0;
    memoryController->write(LY_REGISTER, 0); // Reset scanline register
    // Reset STAT register (except maybe LYC coincidence bit if LY=LYC=0 initially)
    BYTE initialStat = memoryController->read(STAT_REGISTER) & 0x80; // Keep upper bit if needed
    initialStat |= MODE_OAM; // Set initial mode
    if(memoryController->read(LYC_REGISTER) == 0) {
        initialStat |= STAT_LYC_EQ_LY; // Set coincidence if LYC=0
    }
    memoryController->write(STAT_REGISTER, initialStat);

    LOG_INFO("PPU reset to initial state");
}

void PPU::update(int cycles) {
    // monitorRegisterChanges(); // Can be noisy, enable if needed

    if (!isLCDEnabled()) {
        // If LCD was just disabled, reset state
        if (currentMode != MODE_VBLANK || memoryController->read(LY_REGISTER) != 0) {
             LOG_INFO("LCD Disabled - Resetting PPU state (LY=0, Mode=VBLANK)");
             scanlineCounter = SCANLINE_CYCLES;
             memoryController->write(LY_REGISTER, 0);
             // Update STAT register: Mode bits should be 1 (VBLANK) when LCD is off? Check manuals.
             // Pandocs suggests mode is 0 when LCD disabled, but LY is 0. Let's try mode 0.
             setLCDStatus(MODE_HBLANK); // Set mode 0 (HBLANK) when LCD disabled? Or VBLANK? Test.
        }
        return; // Do nothing further if LCD is off
    }

    scanlineCounter -= cycles;

    // Determine current PPU mode based on scanline counter and LY register
    BYTE currentLine = memoryController->read(LY_REGISTER);

    // VBLANK Period (Lines 144-153)
    if (currentLine >= VISIBLE_SCANLINES) {
        if (currentMode != MODE_VBLANK) {
            setLCDStatus(MODE_VBLANK);
            // VBlank interrupt is requested only ONCE when transitioning to line 144
            // It's handled in updateScanline when LY becomes 144.
        }
    }
    // Visible Scanline Period (Lines 0-143)
    else {
        if (scanlineCounter <= 0) { // End of HBLANK (Mode 0) -> Start of next line or VBLANK
            // Move to next scanline
            scanlineCounter += SCANLINE_CYCLES; // Reset counter for the new line
            updateScanline(); // Increments LY, handles VBlank transition
            // The new mode (OAM Scan) will be set at the start of the next update cycle check
            // or by updateScanline if it transitions to VBlank.
            // Check LYC again after LY increment
             checkLYCInterrupt();
             setLCDStatus(MODE_OAM); // Start of new line is OAM scan

        } else if (scanlineCounter <= MODE_0_CYCLES) { // HBLANK (Mode 0)
             if (currentMode != MODE_HBLANK) {
                 setLCDStatus(MODE_HBLANK);
                 // Render scanline data to buffer during HBlank? Or during Mode 3?
                 // Usually Mode 3, but some do it here. Let's assume Mode 3 for now.
             }
        } else if (scanlineCounter <= MODE_0_CYCLES + MODE_3_CYCLES) { // Drawing (Mode 3)
             if (currentMode != MODE_TRANSFER) {
                 setLCDStatus(MODE_TRANSFER);
                 // Perform the actual drawing for the current scanline
                 drawScanline();
             }
        } else { // OAM Scan (Mode 2)
             if (currentMode != MODE_OAM) {
                 setLCDStatus(MODE_OAM);
             }
        }
    }

    // Check LYC=LY coincidence again (might have changed due to LY increment)
    // This check is now done within setLCDStatus and after updateScanline
    // checkLYCInterrupt(); // Moved
}

// Add to PPU.cpp - Initialize once
Uint32 PPU::mapColorToSDL(int r, int g, int b, int a) {
    // Assuming SDL_PIXELFORMAT_RGBA8888 based on texture creation
    return (static_cast<Uint32>(r) << 24) | (static_cast<Uint32>(g) << 16) | (static_cast<Uint32>(b) << 8) | static_cast<Uint32>(a);
}

void PPU::debugFillTestPattern() {
    // Create a test pattern to verify the rendering pipeline
    for (int y = 0; y < SCREEN_PIXELS_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_PIXELS_WIDTH; x++) {
            Uint32 color;
            if ((x / 16 + y / 16) % 2 == 0)
                color = mapColorToSDL(255, 0, 0, 255); // Red
            else
                color = mapColorToSDL(255, 255, 255, 255); // White

            size_t index = static_cast<size_t>(y) * SCREEN_PIXELS_WIDTH + x;
            if (index < screenBuffer.size()) {
                screenBuffer[index] = color;
            }
        }
    }
    LOG_INFO("Test pattern generated - checksum: " + std::to_string(calculateBufferChecksum()));
}

void PPU::setLCDStatus(BYTE mode) {
    // Get current status
    BYTE status = memoryController->read(STAT_REGISTER);
    BYTE currentLine = memoryController->read(LY_REGISTER); // Get current line

    // Update mode bits (lower 2 bits)
    status = (status & 0xFC) | (mode & 0x03);

    // Update LYC=LY flag (bit 2)
    bool coincidence = (currentLine == memoryController->read(LYC_REGISTER));
    if (coincidence) {
        status |= STAT_LYC_EQ_LY;
    } else {
        status &= ~STAT_LYC_EQ_LY;
    }

    // Write updated status back
    memoryController->write(STAT_REGISTER, status);

    // Check for STAT interrupt conditions ONLY if mode actually changed or coincidence happened
    if (mode != currentMode || (coincidence && !(currentMode == MODE_VBLANK && currentLine == 0))) { // Avoid double interrupt on LY=LYC=0 at frame start
        bool requestInterrupt = false;
        // LYC=LY interrupt check
        if (coincidence && (status & STAT_LYC_INT)) {
            requestInterrupt = true;
            LOG_DEBUG("STAT Interrupt check: LYC=LY (" + std::to_string(currentLine) + ") and STAT bit 6 enabled.");
        }
        // Mode interrupt check
        switch (mode) {
            case MODE_HBLANK: // Mode 0
                if (status & STAT_HBLANK_INT) {
                    requestInterrupt = true;
                    LOG_DEBUG("STAT Interrupt check: Mode 0 (HBLANK) and STAT bit 3 enabled.");
                }
                break;
            case MODE_VBLANK: // Mode 1
                if (status & STAT_VBLANK_INT) {
                    requestInterrupt = true;
                    LOG_DEBUG("STAT Interrupt check: Mode 1 (VBLANK) and STAT bit 4 enabled.");
                }
                // Note: VBlank interrupt (IF bit 0) is separate from STAT Mode 1 interrupt
                break;
            case MODE_OAM: // Mode 2
                if (status & STAT_OAM_INT) {
                    requestInterrupt = true;
                    LOG_DEBUG("STAT Interrupt check: Mode 2 (OAM) and STAT bit 5 enabled.");
                }
                break;
            case MODE_TRANSFER: // Mode 3 - No interrupt for this mode
                break;
        }

        if (requestInterrupt) {
            memoryController->RequestInterrupt(LCD_INTERRUPT_BIT); // Use helper function
            LOG_DEBUG("LCD STAT Interrupt Requested.");
        }
    }

    // Log mode change if it happened
    if (mode != currentMode) {
        LOG_DEBUG("PPU Mode changed: " + std::to_string(currentMode) +
                  " -> " + std::to_string(mode) +
                  " [Scanline: " + std::to_string(currentLine) + "]");
        currentMode = mode;
    }
}


// LYC check should be done separately or integrated into setLCDStatus
void PPU::checkLYCInterrupt() {
     BYTE currentLine = memoryController->read(LY_REGISTER);
     BYTE compareValue = memoryController->read(LYC_REGISTER);
     BYTE status = memoryController->read(STAT_REGISTER);
     bool coincidence = (currentLine == compareValue);

     // Update coincidence flag in STAT
     if (coincidence) {
         status |= STAT_LYC_EQ_LY;
     } else {
         status &= ~STAT_LYC_EQ_LY;
     }
     memoryController->write(STAT_REGISTER, status); // Write updated status

     // Request interrupt ONLY if coincidence is true AND LYC interrupt is enabled
     if (coincidence && (status & STAT_LYC_INT)) {
         memoryController->RequestInterrupt(LCD_INTERRUPT_BIT);
         LOG_DEBUG("LYC=LY Interrupt Requested at LY=" + std::to_string(currentLine));
     }
}


bool PPU::isLCDEnabled() const {
    // Read LCDC directly for the most accurate state
    return (memoryController->read(LCD_CONTROL) & LCD_ENABLE_BIT) != 0;
}

void PPU::updateScanline() {
    BYTE currentLine = memoryController->read(LY_REGISTER);
    currentLine++;
    memoryController->write(LY_REGISTER, currentLine);

    // Check LYC=LY coincidence immediately after incrementing LY
    checkLYCInterrupt();

    if (currentLine == VISIBLE_SCANLINES) {
        // Transition to VBlank
        setLCDStatus(MODE_VBLANK); // Set mode 1
        requestVBlankInterrupt(); // Request VBlank interrupt (IF bit 0)
        frameRendered = true; // Mark frame as ready for presentation
        frameCount++;
        // *** ADD THIS LOG ***
        LOG_INFO("--- VBLANK STARTED (LY=" + std::to_string(currentLine) + ") ---");
    } else if (currentLine >= TOTAL_SCANLINES) {
        // Wrap around LY
        memoryController->write(LY_REGISTER, 0);
        // LYC check for LY=0
        checkLYCInterrupt();
        // Transition back to Mode 2 (OAM Scan) for the new frame
        setLCDStatus(MODE_OAM);
        LOG_DEBUG("--- New Frame Start (LY=0) ---");
    }
    // No else needed, if line < 144, the mode transition happens in update() based on cycles
}

void PPU::requestVBlankInterrupt() {
    memoryController->RequestInterrupt(VBLANK_INTERRUPT_BIT); // Use helper function
    LOG_DEBUG("VBlank Interrupt Requested (IF bit 0 set)");
}

void PPU::drawScanline() {
    BYTE control = memoryController->read(LCD_CONTROL);

    // Fill scanline with white initially or based on BG color 0?
    // Let's assume white for now if BG disabled.
    // std::fill_n(screenBuffer.begin() + currentLine * SCREEN_PIXELS_WIDTH, SCREEN_PIXELS_WIDTH, mapColorToSDL(255, 255, 255, 255));

    if (control & 0x01) {  // Bit 0 - BG & Window Enable/Priority
        renderTiles();
    } else {
         // If BG is disabled, the screen area is usually white
         BYTE currentLine = memoryController->read(LY_REGISTER);
         if (currentLine < VISIBLE_SCANLINES) {
            Uint32 white = mapColorToSDL(255, 255, 255, 255);
            size_t startIndex = static_cast<size_t>(currentLine) * SCREEN_PIXELS_WIDTH;
            std::fill_n(screenBuffer.begin() + startIndex, SCREEN_PIXELS_WIDTH, white);
         }
    }

    if (control & 0x02) {  // Bit 1 - OBJ (Sprite) Display Enable
        renderSprites();
    }
}


// First, fix the buffer index comparison issue
void PPU::setPixel(int x, int y, Uint32 color) {
     // Add boundary checks for safety
     if (x < 0 || x >= SCREEN_PIXELS_WIDTH || y < 0 || y >= SCREEN_PIXELS_HEIGHT) {
         // LOG_WARNING("setPixel out of bounds: (" + std::to_string(x) + ", " + std::to_string(y) + ")");
         return;
     }
    size_t bufferIndex = static_cast<size_t>(y) * SCREEN_PIXELS_WIDTH + x;
    // Check against buffer size (already done in caller, but safe to keep)
    if (bufferIndex < screenBuffer.size()) {
        screenBuffer[bufferIndex] = color;
    } else {
         // This should ideally not happen if coordinates are checked
         LOG_ERROR("setPixel buffer index out of range: " + std::to_string(bufferIndex));
    }
}

// Now fix the renderTiles function with proper window handling
void PPU::renderTiles() {
    // static int pixels_drawn = 0; // Remove static counter
    // pixels_drawn++;
    // if (pixels_drawn % 1000 == 0) {
    //     LOG_DEBUG("Drawn " + std::to_string(pixels_drawn) + " pixels");
    // }

    // Add in Emulator::render():
    // LOG_DEBUG("Updating texture with screen buffer - Checksum: " +
    //         std::to_string(calculateBufferChecksum())); // Moved logging to render()
    BYTE lcdControl = memoryController->read(LCD_CONTROL);
    // if (lcdControl != prevLCDControl) { // Moved to monitorRegisterChanges
    //     LOG_INFO("LCD Control changed to: 0x" + std::to_string(lcdControl));
    // }

    BYTE scrollY = memoryController->read(SCY_REGISTER);
    BYTE scrollX = memoryController->read(SCX_REGISTER);

    // Get current scanline first
    BYTE currentLine = memoryController->read(LY_REGISTER);
    if (currentLine >= VISIBLE_SCANLINES) return; // Should not draw outside visible area

    // Window position registers
    BYTE windowY = memoryController->read(WY_REGISTER);
    BYTE windowX = memoryController->read(WX_REGISTER) - 7;  // WX is offset by 7
    bool windowEnabledThisLine = (lcdControl & 0x20) && (lcdControl & 0x01) && windowY <= currentLine; // Window Enable + BG/Win Enable

    // Determine which tile data area to use
    WORD tileDataArea = (lcdControl & 0x10) ? TILE_DATA_1 : TILE_DATA_2;
    bool unsignedIndexing = (lcdControl & 0x10); // Tile Data Select (1=8000-8FFF, 0=8800-97FF)

    // Background tile map selection
    WORD bgTileMap = (lcdControl & 0x08) ? BG_TILE_MAP_2 : BG_TILE_MAP_1;

    // Window tile map selection
    WORD windowTileMap = (lcdControl & 0x40) ? WINDOW_TILE_MAP_2 : WINDOW_TILE_MAP_1;

    BYTE bgPalette = memoryController->read(BGP_REGISTER);

    // Draw the 160 pixels for this scanline
    for (int pixel = 0; pixel < SCREEN_PIXELS_WIDTH; pixel++) {
        bool useWindow = windowEnabledThisLine && pixel >= windowX;

        BYTE mapX, mapY;
        WORD tileMapAddrBase;
        BYTE tileLineInTile;

        if (useWindow) {
            mapX = pixel - windowX;
            mapY = currentLine - windowY; // Y relative to window start
            tileMapAddrBase = windowTileMap;
            tileLineInTile = mapY % 8;
        } else {
            mapX = pixel + scrollX;
            mapY = currentLine + scrollY;
            tileMapAddrBase = bgTileMap;
            tileLineInTile = mapY % 8;
        }

        // Calculate tile coordinates (32x32 map)
        BYTE tileX = mapX / 8;
        BYTE tileY = mapY / 8;

        // Get the tile index from the appropriate tile map
        WORD tileMapAddress = tileMapAddrBase + (static_cast<WORD>(tileY) * 32) + tileX;
        BYTE tileIndex = memoryController->read(tileMapAddress);

        // Calculate tile data address based on indexing mode
        WORD tileDataAddress;
        if (unsignedIndexing) { // Use $8000-$8FFF
            tileDataAddress = TILE_DATA_1 + (static_cast<WORD>(tileIndex) * 16);
        } else { // Use $8800-$97FF (signed index relative to $9000)
            tileDataAddress = TILE_DATA_2 + (static_cast<int8_t>(tileIndex) * 16);
        }

        // Get the specific line of the tile we need (2 bytes per line)
        WORD tileLineAddress = tileDataAddress + (static_cast<WORD>(tileLineInTile) * 2);
        BYTE tileData1 = memoryController->read(tileLineAddress);
        BYTE tileData2 = memoryController->read(tileLineAddress + 1);

        // Get the specific pixel's color index within the tile line
        BYTE pixelBit = 7 - (mapX % 8); // Bit position (7=leftmost, 0=rightmost)
        BYTE colorNum = ((tileData2 >> pixelBit) & 1) << 1; // Bit from second byte
        colorNum |= ((tileData1 >> pixelBit) & 1);      // Bit from first byte

        // Get the actual color from the background palette
        BYTE color = (bgPalette >> (colorNum * 2)) & 0x03;

        // Convert palette index to RGBA color
        Uint32 pixelColor;
        switch(color) {
            case 0: pixelColor = mapColorToSDL(255, 255, 255, 255); break; // White
            case 1: pixelColor = mapColorToSDL(170, 170, 170, 255); break; // Light gray
            case 2: pixelColor = mapColorToSDL(85, 85, 85, 255);    break; // Dark gray
            case 3: pixelColor = mapColorToSDL(0, 0, 0, 255);       break; // Black
            default: pixelColor = mapColorToSDL(255, 0, 255, 255); break;  // Error color (pink)
        }

        // Set the pixel in the buffer
        setPixel(pixel, currentLine, pixelColor);
    }
}
uint32_t PPU::calculateBufferChecksum() {
    uint32_t checksum = 0;
    for (const auto& pixel : screenBuffer) {
        // A simple checksum algorithm (e.g., Fletcher's checksum or just sum)
        checksum = (checksum + pixel) & 0xFFFFFFFF; // Basic sum, wrap around
    }
    return checksum;
}
int PPU::getColorFromPalette(BYTE palette, int colorId) {
    // This function seems to map palette index (0-3) to grayscale (0, 96, 192, 255)
    // Let's keep it as is, assuming this is the desired grayscale mapping.
    // If you want actual colors later, this needs changing.
    int colorIndex = 0;
    switch (colorId) {
        case 0: colorIndex = (palette >> 0) & 0x03; break; // Bits 1-0
        case 1: colorIndex = (palette >> 2) & 0x03; break; // Bits 3-2
        case 2: colorIndex = (palette >> 4) & 0x03; break; // Bits 5-4
        case 3: colorIndex = (palette >> 6) & 0x03; break; // Bits 7-6
    }

    // Map palette index to grayscale intensity (0=white, 3=black)
    switch (colorIndex) {
        case 0: return 255;   // White
        case 1: return 170;   // Light gray (~192)
        case 2: return 85;    // Dark gray (~96)
        case 3: return 0;     // Black
        default: return 0;    // Should not happen
    }
}
// Add this method to ppu.cpp
void PPU::monitorRegisterChanges() {
    // Monitor LCD Control changes
    BYTE lcdControl = memoryController->read(LCD_CONTROL);
    if (lcdControl != prevLCDControl) {
        std::stringstream ss;
        ss << "LCD Control changed: 0x" << std::hex << static_cast<int>(prevLCDControl)
           << " -> 0x" << static_cast<int>(lcdControl)
           << " [LCD:" << ((lcdControl & 0x80) ? "ON" : "OFF")
           << " WIN:" << ((lcdControl & 0x20) ? "ON" : "OFF")
           << " BG:" << ((lcdControl & 0x01) ? "ON" : "OFF") << "]";
        LOG_INFO(ss.str());
        prevLCDControl = lcdControl;
    }

    // Monitor background palette changes
    BYTE bgp = memoryController->read(BGP_REGISTER);
    if (bgp != prevBGP) {
         std::stringstream ss;
         ss << "BGP changed: 0x" << std::hex << static_cast<int>(prevBGP)
            << " -> 0x" << static_cast<int>(bgp)
            << " [Colors: " << ((bgp >> 0) & 0x03)
            << "," << ((bgp >> 2) & 0x03)
            << "," << ((bgp >> 4) & 0x03)
            << "," << ((bgp >> 6) & 0x03) << "]";
        LOG_INFO(ss.str());
        prevBGP = bgp;
    }
}
void PPU::renderSprites() {
    // static int pixels_drawn = 0; // Remove static counter
    // pixels_drawn++;
    // if (pixels_drawn % 1000 == 0) {
    //     LOG_DEBUG("Drawn " + std::to_string(pixels_drawn) + " pixels");
    // }

    // Add in Emulator::render():
    // LOG_DEBUG("Updating texture with screen buffer - Checksum: " +
    //         std::to_string(calculateBufferChecksum())); // Moved logging to render()
    BYTE lcdControl = memoryController->read(LCD_CONTROL);
    // if (lcdControl != prevLCDControl) { // Moved to monitorRegisterChanges
    //     LOG_INFO("LCD Control changed to: 0x" + std::to_string(lcdControl));
    // }
    bool use8x16 = (lcdControl & 0x04) != 0;  // Bit 2: OBJ (Sprite) Size (0=8x8, 1=8x16)
    int spriteHeight = use8x16 ? 16 : 8;

    BYTE currentLine = memoryController->read(LY_REGISTER);
    if (currentLine >= VISIBLE_SCANLINES) return; // Don't render sprites outside visible area

    int spritesRenderedThisLine = 0; // DMG PPU can only render 10 sprites per scanline

    // Iterate through OAM (Object Attribute Memory)
    for (int spriteIndex = 0; spriteIndex < MAX_SPRITES; spriteIndex++) {
        if (spritesRenderedThisLine >= 10) break; // Stop after 10 sprites

        // Calculate the address of the sprite's attributes
        WORD spriteAddress = OAM_START + (static_cast<WORD>(spriteIndex) * SPRITE_ATTRIBUTE_SIZE);

        // Read the sprite's attributes
        BYTE yPos = memoryController->read(spriteAddress + SPRITE_Y_POS); // Screen Y = yPos - 16
        BYTE xPos = memoryController->read(spriteAddress + SPRITE_X_POS); // Screen X = xPos - 8
        BYTE tileIndex = memoryController->read(spriteAddress + SPRITE_TILE_INDEX);
        BYTE attributes = memoryController->read(spriteAddress + SPRITE_ATTRIBUTES);

        // Adjust for screen coordinates
        int screenY = yPos - 16;
        int screenX = xPos - 8;

        // Is the sprite visible on this scanline?
        if (currentLine >= screenY && currentLine < (screenY + spriteHeight)) {
            spritesRenderedThisLine++; // Count this sprite

            bool yFlip = (attributes & SPRITE_Y_FLIP) != 0;
            bool xFlip = (attributes & SPRITE_X_FLIP) != 0;
            bool bgPriority = (attributes & SPRITE_PRIORITY) != 0; // Bit 7: BG and Window over OBJ
            bool paletteNumber = (attributes & SPRITE_PALETTE) != 0;  // Bit 4: Palette Number (0=OBP0, 1=OBP1)

            // Determine the palette address
            WORD paletteAddress = paletteNumber ? OBP1_REGISTER : OBP0_REGISTER;
            BYTE obp = memoryController->read(paletteAddress);

            // Calculate the line within the sprite tile(s)
            int lineInSprite = currentLine - screenY;
            if (yFlip) {
                lineInSprite = spriteHeight - 1 - lineInSprite;
            }

            // Adjust tile index for 8x16 sprites
            if (use8x16) {
                tileIndex &= 0xFE; // Mask LSB for 8x16 sprites
                if (lineInSprite >= 8) {
                    tileIndex |= 0x01; // Use the bottom tile if needed
                    lineInSprite -= 8;
                }
            }

            // Calculate the address of the tile data in VRAM ($8000-$8FFF)
            WORD tileDataAddress = TILE_DATA_1 + (static_cast<WORD>(tileIndex) * 16) + (static_cast<WORD>(lineInSprite) * 2);

            // Read the two bytes for the tile line
            BYTE data1 = memoryController->read(tileDataAddress);
            BYTE data2 = memoryController->read(tileDataAddress + 1);

            // Render the 8 horizontal pixels for this sprite line
            for (int tilePixelX = 0; tilePixelX < 8; tilePixelX++) {
                // Calculate the final screen X coordinate for this pixel
                int pixelX = screenX + tilePixelX;

                // Skip if pixel is off-screen horizontally
                if (pixelX < 0 || pixelX >= SCREEN_PIXELS_WIDTH) {
                    continue;
                }

                // Determine the bit position within the tile data bytes (7=left, 0=right)
                int colorBit = xFlip ? tilePixelX : (7 - tilePixelX);

                // Extract the 2-bit color number (0-3)
                BYTE colorNum = ((data2 >> colorBit) & 1) << 1;
                colorNum |= ((data1 >> colorBit) & 1);

                // Color number 0 is transparent for sprites
                if (colorNum == 0) {
                    continue;
                }

                // Check BG Priority attribute
                if (bgPriority) {
                    // Check if the underlying BG/Window pixel is non-zero (not color 0)
                    size_t bgBufferIndex = static_cast<size_t>(currentLine) * SCREEN_PIXELS_WIDTH + pixelX;
                    // Need to convert the existing pixel color back to a palette index/number
                    // This requires knowing the BG palette used for that pixel.
                    // For simplicity, often emulators check if the BG color is != white (color 0)
                    // This isn't perfectly accurate but often works.
                    // A more accurate way involves storing the BG color index per pixel.
                    // Let's assume for now: if BG pixel is not white, sprite is hidden.
                    if (bgBufferIndex < screenBuffer.size() && screenBuffer[bgBufferIndex] != mapColorToSDL(255, 255, 255, 255)) {
                         continue; // BG pixel has priority, skip drawing sprite pixel
                    }
                }


                // Get the actual color index (0-3) from the selected object palette
                BYTE colorIndex = (obp >> (colorNum * 2)) & 0x03;

                // Map the color index to a display color (using grayscale for now)
                int grayColor = 0;
                 switch (colorIndex) {
                     case 0: grayColor = 255; break; // White (Should be transparent, handled above)
                     case 1: grayColor = 170; break; // Light gray
                     case 2: grayColor = 85; break;  // Dark gray
                     case 3: grayColor = 0; break;   // Black
                 }
                 Uint32 rgbaColor = mapColorToSDL(grayColor, grayColor, grayColor, 255);

                // Set the pixel in the buffer
                setPixel(pixelX, currentLine, rgbaColor);
            }
        }
    }
}
