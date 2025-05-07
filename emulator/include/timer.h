#pragma once
#include <common.h>
#include <memory_controller.h>
#include <memory>

// Timer registers
constexpr WORD TIMA = 0xFF05;  // Timer counter
constexpr WORD TMA  = 0xFF06;  // Timer modulo
constexpr WORD TMC  = 0xFF07;  // Timer control

// Timer control bits
constexpr BYTE TIMER_ENABLE_BIT = 0x04;     // Bit 2 controls timer enable/disable
constexpr BYTE CLOCK_SELECT_MASK = 0x03;    // Bits 0-1 select clock frequency
constexpr BYTE DIVIDER_MAX = 255;

class MemoryController; // Forward declaration of MemoryController class
class Timer {
private:
    std::shared_ptr<MemoryController> memoryController;
    BYTE m_Counter;     // TIMA - Timer counter
    BYTE m_Modulo;      // TMA - Timer modulo
    BYTE m_Control;     // TMC - Timer control
    int m_TimerCounter; // Internal counter for frequency tracking
    bool m_IsEnabled;   // Timer enabled flag
    int m_DividerCounter;    // Counter for divider register
    BYTE m_DividerRegister;  // Current value of divider register

public:
    Timer(std::shared_ptr<MemoryController> memory);
    bool isEnabled() const;
    void update(int cycles);
    BYTE read(WORD address) const;
    void write(WORD address, BYTE value);
    bool isInterruptRequested() const;
    void resetInterruptRequest();
    void updateDividerRegister(int cycles);
    BYTE getDividerRegister() const;
    void resetDividerRegister();

private:
    int getFrequency() const;
    BYTE getClockFreq() const;
    void setClockFreq();
};