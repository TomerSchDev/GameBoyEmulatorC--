#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <cart.h>
#include <common.h>
#include <string>
#include <memory>
#include <memory_controller.h>
#include <ppu.h>
#include <timer.h>
#include "joypad.h"
#include <unordered_map>
#include <functional> // Added for std::function
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

// Hardware constants
constexpr int FRAME_DELAY_MS = 1000 / TARGET_FPS;  // ~16.67ms

// Keep necessary forward declarations
class Timer;
class Joypad;
class MemoryController;
class PPU;
namespace GB { class CPU; class Timer; } // Forward declare CPU in the GB namespace
class Emulator {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    bool loaded;
    
    // Thread management
    std::thread emulatorThread;
    std::atomic<bool> emulationActive{false};
    std::atomic<bool> running{false}; // Added this
    std::atomic<float> emulationSpeed{1.0f};
    std::mutex screenBufferMutex;
    std::mutex inputMutex; // Added this
    std::mutex pauseMutex;  // Add this dedicated mutex for pausing
    std::condition_variable pauseCondition;
    std::atomic<bool> debugMode{false};
    std::atomic<bool> paused{false};

    std::shared_ptr<MemoryController> memoryController;
    std::unique_ptr<GB::CPU> cpu;
    std::unique_ptr<PPU> ppu;
    std::unique_ptr<GB::Timer> timer;
    
    // Changed to use std::function
    std::unordered_map<SDL_Keycode, int> keyMap;
    std::unordered_map<int, std::function<void(int)>> keyFunction_array; // Changed from member function pointers
    
public:
    Emulator();
    ~Emulator();
    bool init();
    void run();
    void cleanup();
    bool loadGame(const std::string& gamePath);
    bool unloadGame();
    void RequestInterrupt(BYTE interruptBit);
    Joypad joypad;
    
private:
    void handleInput(const SDL_Event& event);
    void update();
    void render();
    int ExecuteNextOpcode();
    void pauseEmulation(bool pause);
    void toggleDebugMode();
    void emulationLoop();
    int handleInterrupts();
    BYTE GetJoypadState();
    void KeyReleased(int key);
    void KeyPressed(int key);
    void startEmulation();
    void stopEmulation();
    void setEmulationSpeed(float speed);
};