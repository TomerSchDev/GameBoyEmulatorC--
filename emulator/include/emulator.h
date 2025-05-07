#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <cart.h>
#include <cpu.h>
#include <string>
#include <memory>
#include <memory_controller.h>
#include <ppu.h>
#include <timer.h>
#include "joypad.h" // Include Joypad header
#include <unordered_map>           // For std::map (or <unordered_map> for std::unordered_map)


class CPU;
class PPU;
class MemoryController;


//Hardware constants
constexpr int FRAME_DELAY_MS = 1000 / TARGET_FPS;  // ~16.67ms

class Timer;
class Joypad; // Forward declaration of Joypad class
class MemoryController; // Forward declaration of MemoryController class
class PPU; // Forward declaration of PPU class
class CPU; // Forward declaration of CPU class
class Emulator {
    private:
        SDL_Window* window;
        SDL_Renderer* renderer;
        SDL_Texture* texture;
        bool running;
        bool loaded;
        
        std::shared_ptr<MemoryController> memoryController;
        std::unique_ptr<CPU> cpu;
        std::unique_ptr<PPU> ppu;
        std::unique_ptr<Timer> timer;
        std::unordered_map<SDL_Keycode, int> keyMap; // Use std::map
public:
    Emulator();
    ~Emulator();
    bool init();
    void run();
    void cleanup();
    bool loadGame(const std::string& gamePath);
    bool unloadGame();

    Joypad joypad; // Instance of Joypad class
private:
    void handleEvents();
    void update();
    void render();
    int ExecuteNextOpcode();
    void handleInterrupts();
    BYTE GetJoypadState();
    void KeyReleased(int key);
    void KeyPressed(int key);
    

};