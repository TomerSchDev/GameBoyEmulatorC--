#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//screen dims
#define SCREEN_PIXELS_WIDTH 160
#define SCREEN_PIXELS_HEIGHT 144

//global constants
#define SCREEN_SCALE 4

#define WIDNOW_WIDTH SCREEN_PIXELS_WIDTH * SCREEN_SCALE
#define WINDOW_HEIGHT SCREEN_PIXELS_HEIGHT * SCREEN_SCALE


// GameBoy Hardware Constants
constexpr int CPU_CLOCK_SPEED = 4194304;  // 4.194304 MHz
constexpr int TARGET_FPS = 60;
constexpr int CYCLES_PER_FRAME = CPU_CLOCK_SPEED / TARGET_FPS;


//types
typedef unsigned char BYTE ;
typedef char SIGNED_BYTE ;
typedef unsigned short WORD ;
typedef signed short SIGNED_WORD ;

// Interrupt registers and flags
constexpr WORD IF_REGISTER = 0xFF0F;    // Interrupt Flag Register
constexpr WORD IE_REGISTER = 0xFFFF;    // Interrupt Enable Register
constexpr WORD DIV_REGISTER = 0xFF04;  // Divider Register


// Interrupt service routine addresses
constexpr WORD VBLANK_ISR_ADDR = 0x0040;
constexpr WORD LCD_ISR_ADDR    = 0x0048;
constexpr WORD TIMER_ISR_ADDR  = 0x0050;
constexpr WORD JOYPAD_ISR_ADDR = 0x0060;


// Interrupt bits
constexpr BYTE VBLANK_INTERRUPT_BIT = 0x01;   // Bit 0
constexpr BYTE LCD_INTERRUPT_BIT    = 0x02;   // Bit 1
constexpr BYTE TIMER_INTERRUPT_BIT  = 0x04;   // Bit 2
constexpr BYTE JOYPAD_INTERRUPT_BIT = 0x10;   // Bit 4
constexpr BYTE LCD_ENABLE_BIT = 0x80;    // Bit 7 of LCD Control

// LCD/PPU Constants
constexpr WORD LY_REGISTER = 0xFF44;     // Current scanline
constexpr WORD LCD_CONTROL = 0xFF40;     // LCD Control register
constexpr int SCANLINE_CYCLES = 456;     // Cycles per scanline
constexpr int VISIBLE_SCANLINES = 144;   // Number of visible scanlines
constexpr int TOTAL_SCANLINES = 154;     // Total scanlines including VBlank

// LCD Status Register constants
constexpr WORD STAT_REGISTER = 0xFF41;    // LCD Status Register
constexpr WORD LYC_REGISTER = 0xFF45;     // LY Compare Register

// LCD Mode bits in STAT register
constexpr BYTE MODE_HBLANK = 0x00;
constexpr BYTE MODE_VBLANK = 0x01;
constexpr BYTE MODE_OAM = 0x02;
constexpr BYTE MODE_TRANSFER = 0x03;

// STAT register interrupt bits
constexpr BYTE STAT_HBLANK_INT = 0x08;    // Mode 0 interrupt (bit 3)
constexpr BYTE STAT_VBLANK_INT = 0x10;    // Mode 1 interrupt (bit 4)
constexpr BYTE STAT_OAM_INT = 0x20;       // Mode 2 interrupt (bit 5)
constexpr BYTE STAT_LYC_INT = 0x40;       // LYC interrupt (bit 6)

// Mode durations in cycles
constexpr int MODE_2_CYCLES = 80;         // OAM scan
constexpr int MODE_3_CYCLES = 172;        // Pixel transfer
constexpr int MODE_0_CYCLES = 204;        // HBlanks

// DMA Constants
constexpr WORD DMA_REGISTER = 0xFF46;    // DMA Transfer and Control
constexpr WORD OAM_START = 0xFE00;       // Start of Sprite Attribute Table
constexpr WORD OAM_END = 0xFE9F;         // End of Sprite Attribute Table
constexpr BYTE DMA_LENGTH = 0xA0;        // Length of DMA transfer (160 bytes)

// LCD viewport and scrolling registers
constexpr WORD SCY_REGISTER = 0xFF42;     // Scroll Y register
constexpr WORD SCX_REGISTER = 0xFF43;     // Scroll X register
constexpr WORD WY_REGISTER = 0xFF4A;      // Window Y register
constexpr WORD WX_REGISTER = 0xFF4B;      // Window X register
constexpr WORD BGP_REGISTER = 0xFF47;    // Background palette register
// Tile map regions
constexpr WORD BG_TILE_MAP_1 = 0x9800;    // BG tile map region 1
constexpr WORD BG_TILE_MAP_2 = 0x9C00;    // BG tile map region 2
constexpr WORD WINDOW_TILE_MAP_1 = 0x9800; // Window tile map region 1
constexpr WORD WINDOW_TILE_MAP_2 = 0x9C00; // Window tile map region 2

// Tile data regions
constexpr WORD TILE_DATA_1 = 0x8000;      // Tile data region 1 (unsigned)
constexpr WORD TILE_DATA_2 = 0x8800;      // Tile data region 2 (signed)

constexpr int TILE_SIZE = 16;             // Size of each tile in bytes
constexpr int TILE_WIDTH = 8;              // Width of a tile in pixels
constexpr int TILE_HEIGHT = 8;             // Height of a tile in pixels

// Sprite-related constants
constexpr int MAX_SPRITES = 40;           // Maximum number of sprites
constexpr int SPRITE_ATTRIBUTE_SIZE = 4;  // Size of each sprite attribute in bytes

// Sprite attribute offsets
constexpr int SPRITE_Y_POS = 0;           // Y position offset
constexpr int SPRITE_X_POS = 1;           // X position offset
constexpr int SPRITE_TILE_INDEX = 2;      // Tile index offset
constexpr int SPRITE_ATTRIBUTES = 3;      // Attributes offset

// Sprite attribute flags
constexpr BYTE SPRITE_PRIORITY = 0x80;     // Bit 7: Sprite to Background Priority
constexpr BYTE SPRITE_Y_FLIP = 0x40;        // Bit 6: Y flip
constexpr BYTE SPRITE_X_FLIP = 0x20;        // Bit 5: X flip
constexpr BYTE SPRITE_PALETTE = 0x10;       // Bit 4: Palette number

// Joypad register
constexpr WORD JOYPAD_REGISTER = 0xFF00;

// Joypad bit assignments
constexpr BYTE JOYPAD_RIGHT = 0;
constexpr BYTE JOYPAD_LEFT = 1;
constexpr BYTE JOYPAD_UP = 2;
constexpr BYTE JOYPAD_DOWN = 3;
constexpr BYTE JOYPAD_A = 4;
constexpr BYTE JOYPAD_B = 5;
constexpr BYTE JOYPAD_SELECT = 6;
constexpr BYTE JOYPAD_START = 7;

// Joypad select bits
constexpr BYTE JOYPAD_SELECT_BUTTONS = 0x20; // Bit 5
constexpr BYTE JOYPAD_SELECT_DIRECTIONS = 0x10; // Bit 4
// Interrupt priority (lower index = higher priority)
enum class InterruptType {
    VBlank = 0,
    LCD    = 1,
    Timer  = 2,
    Joypad = 4
};





