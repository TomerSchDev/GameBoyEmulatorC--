#include <iostream>
#include <emulator.h>
#include <logger.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main(int argc, char* argv[]) {
    bool debugMode = false;
    
    auto logger = Logger::getInstance();
    logger->setLogLevel(LogLevel::INFO);
    if (argc > 1) {
        debugMode = true;
    }
    if (debugMode) {
        std::string logFileName = "emulator_" + std::string(argv[1]) + ".log";
        auto new_log = Logger::getInstance(logFileName);
        new_log->setLogLevel(LogLevel::DEBUG);
    }    
    // Create emulator instance
    Emulator emulator;
    
    // Initialize emulator
    LOG_INFO("Initializing emulator...");
    if (!emulator.init()) {
        LOG_ERROR("Failed to initialize emulator");
        return 1;
    }

    // Try to load ROM
    std::string gamePath = "roms\\Tetris.gb"; // Change this to your ROM path
    

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