#pragma once
#include "common.h"

class Joypad {
private:
    BYTE joypadState = 0xFF; // Initialize all buttons as unpressed (1)

public:
    BYTE GetState(BYTE joypadRequest);
    void KeyPressed(int key);
    BYTE GetJoypadState() const { return joypadState; }
    void KeyReleased(int key);
};