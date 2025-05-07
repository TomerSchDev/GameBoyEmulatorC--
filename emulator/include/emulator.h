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

class CPU;
class PPU;
class MemoryController;


//Hardware constants
constexpr int FRAME_DELAY_MS = 1000 / TARGET_FPS;  // ~16.67ms


class Emulator {
    private:
        SDL_Window* window;
        SDL_Renderer* renderer;
        bool running;
        bool loaded;
        
        std::shared_ptr<MemoryController> memoryController;
        std::unique_ptr<CPU> cpu;
        std::unique_ptr<PPU> ppu;
        std::unique_ptr<Timer> timer;
public:
    Emulator();
    ~Emulator();
    bool init();
    void run();
    void cleanup();
    bool loadGame(const std::string& gamePath);
    bool unloadGame();

private:
    void handleEvents();
    void update();
    void render();
    int ExecuteNextOpcode();
    void handleInterrupts();

};