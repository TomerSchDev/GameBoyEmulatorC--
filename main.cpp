#include <iostream>
#include <emulator.h>
#include <logger.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main(int argc, char* argv[]) {
    auto logger = Logger::getInstance();
    logger->setLogLevel(LogLevel::DEBUG);
    
    // Create emulator instance
    Emulator emulator;
    
    // Initialize emulator
    LOG_INFO("Initializing emulator...");
    if (!emulator.init()) {
        LOG_ERROR("Failed to initialize emulator");
        return 1;
    }

    // Try to load ROM
    std::string gamePath = "roms\\Pokemon_Red.gb";
    if (argc > 1) {
        gamePath = argv[1];
    }

    LOG_INFO("Loading ROM: " + gamePath);
    if (!emulator.loadGame(gamePath)) {
        LOG_ERROR("Failed to load ROM");
        emulator.cleanup();
        return 1;
    }

    // Main emulation loop
    LOG_INFO("Starting emulation...");
    emulator.run();

    // Cleanup
    LOG_INFO("Shutting down...");
    if (!emulator.unloadGame()) {
        LOG_ERROR("Failed to unload ROM");
    }
    
    emulator.cleanup();
    return 0;
}