#include "joypad.h"

BYTE Joypad::GetState(BYTE joypadRequest) {
    BYTE joypadOutput = 0x0F; // Initialize with all buttons unpressed (1)

    if (!(joypadRequest & JOYPAD_SELECT_DIRECTIONS)) {
        // Direction buttons are selected
        if (!(joypadState & (1 << JOYPAD_RIGHT))) joypadOutput &= ~(1 << JOYPAD_RIGHT);
        if (!(joypadState & (1 << JOYPAD_LEFT))) joypadOutput &= ~(1 << JOYPAD_LEFT);
        if (!(joypadState & (1 << JOYPAD_UP))) joypadOutput &= ~(1 << JOYPAD_UP);
        if (!(joypadState & (1 << JOYPAD_DOWN))) joypadOutput &= ~(1 << JOYPAD_DOWN);
    } else if (!(joypadRequest & JOYPAD_SELECT_BUTTONS)) {
        // Button keys are selected
        if (!(joypadState & (1 << JOYPAD_A))) joypadOutput &= ~(1 << JOYPAD_A);
        if (!(joypadState & (1 << JOYPAD_B))) joypadOutput &= ~(1 << JOYPAD_B);
        if (!(joypadState & (1 << JOYPAD_SELECT))) joypadOutput &= ~(1 << JOYPAD_SELECT);
        if (!(joypadState & (1 << JOYPAD_START))) joypadOutput &= ~(1 << JOYPAD_START);
    }

    return joypadOutput;
}

void Joypad::KeyPressed(int key) {
    //bool previouslyUnset = (joypadState & (1 << key)) != 0;

    // Set the bit in joypadState to 0 (pressed)
    joypadState &= ~(1 << key);
}
void Joypad::KeyReleased(int key) {
    // Set the bit in joypadState to 1 (released)
    joypadState |= (1 << key);
}