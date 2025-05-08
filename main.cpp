#include <iostream>
#include <emulator.h>
#include <logger.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main(int argc, char* argv[]) {
    bool debugMode = false;
    if (argc > 1) {
        //check if the first argument is "debug" to enable debug mode
        if (std::string(argv[1]) == "debug") {
            // Enable debug mode
            std::cout << "Debug mode enabled" << std::endl;
            debugMode = true;
        } 
    }
    auto logger = Logger::getInstance();
    Logger::getInstance()->setLogLevel(LogLevel::INFO);;
    if (debugMode) {
        // Set the log level to DEBUG if debug mode is enabled
        logger = Logger::getInstance("emulator_debug.log");
        Logger::getInstance()->setLogLevel(LogLevel::DEBUG);
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