#include <emulator.h>
#include <logger.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <functional>
#include <iostream>
#include <cpu.h>  // Full include here, not in header

// Define IF and IE register addresses if not defined elsewhere
#ifndef IF_REGISTER
#define IF_REGISTER 0xFF0F
#endif
#ifndef IE_REGISTER
#define IE_REGISTER 0xFFFF
#endif
// Define interrupt bits if not defined elsewhere
#ifndef VBLANK_INTERRUPT_BIT
#define VBLANK_INTERRUPT_BIT 0x01
#endif
#ifndef LCD_INTERRUPT_BIT
#define LCD_INTERRUPT_BIT    0x02
#endif
#ifndef TIMER_INTERRUPT_BIT
#define TIMER_INTERRUPT_BIT  0x04
#endif
#ifndef SERIAL_INTERRUPT_BIT
#define SERIAL_INTERRUPT_BIT 0x08
#endif
#ifndef JOYPAD_INTERRUPT_BIT
#define JOYPAD_INTERRUPT_BIT 0x10
#endif


Emulator::Emulator()
    : window(nullptr)
    , renderer(nullptr)
    , texture(nullptr)
    , loaded(false)
    , emulationActive(false)
    , running(false)
    , emulationSpeed(1.0f)  // Initialize first (matches declaration order)
    , debugMode(false)      // Initialize second
    , paused(false)         // Initialize third
    // Continue with other members in declaration order
    , keyFunction_array()
    , joypad()
{
    LOG_INFO("Emulator constructor called");
    // Initialize key map
    keyMap[SDLK_A] = JOYPAD_A;
    keyMap[SDLK_S] = JOYPAD_B;
    keyMap[SDLK_RETURN] = JOYPAD_START;
    keyMap[SDLK_SPACE] = JOYPAD_SELECT;
    keyMap[SDLK_RIGHT] = JOYPAD_RIGHT;
    keyMap[SDLK_LEFT] = JOYPAD_LEFT;
    keyMap[SDLK_UP] = JOYPAD_UP;
    keyMap[SDLK_DOWN] = JOYPAD_DOWN;
    keyMap[SDLK_LSHIFT] = -2; // Special key handling
    keyFunction_array[0] = [this](int key) { this->KeyReleased(key); }; // Released (0)
    keyFunction_array[1] = [this](int key) { this->KeyPressed(key); }; // Pressed (1)
}

Emulator::~Emulator() {
    cleanup();
}
bool Emulator::init() {
    LOG_INFO("Initializing emulator...");

    if (!SDL_Init(SDL_INIT_VIDEO)){ // Check return value of SDL_Init
        LOG_ERROR("SDL could not initialize! SDL_Error: " + std::string(SDL_GetError()));
        return false;
    }

    if (!TTF_Init()) { // Check return value of TTF_Init
        LOG_ERROR("SDL_ttf could not initialize! TTF_Error: " + std::string(SDL_GetError()));
        SDL_Quit(); // Quit SDL if TTF fails
        return false;
    }

    window = SDL_CreateWindow("GameBoy Emulator", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window) {
        LOG_ERROR("Window could not be created! SDL_Error: " + std::string(SDL_GetError()));
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, NULL); // No specific driver needed usually
    if (!renderer) {
        LOG_ERROR("Renderer could not be created! SDL_Error: " + std::string(SDL_GetError()));
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
        SCREEN_PIXELS_WIDTH, SCREEN_PIXELS_HEIGHT);
    if (!texture) {
        LOG_ERROR("Texture could not be created! SDL_Error: " + std::string(SDL_GetError()));
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }


    // Initialize core components
    memoryController = std::make_shared<MemoryController>();
    if (!memoryController) {
         LOG_ERROR("Failed to initialize Memory Controller");
         // Cleanup SDL resources before returning
         SDL_DestroyTexture(texture);
         SDL_DestroyRenderer(renderer);
         SDL_DestroyWindow(window);
         TTF_Quit();
         SDL_Quit();
         return false;
    }
    // Pass 'this' emulator instance to MemoryController AFTER it's constructed
    memoryController->attachEmulator(this);

    cpu = std::make_unique<GB::CPU>(memoryController);
    ppu = std::make_unique<PPU>(memoryController);
    timer = std::make_unique<GB::Timer>(memoryController); // Pass MemoryController to Timer

    if (!cpu || !ppu || !timer) {
        LOG_ERROR("Failed to initialize core components (CPU, PPU, or Timer)");
        // Cleanup SDL and MemoryController
        memoryController.reset(); // Release MemoryController
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    // Set initial state
    running = false; // Should be set true by run()
    loaded = false;

    LOG_INFO("Emulator initialized successfully");
    return true;
}
void Emulator::pauseEmulation(bool pause) {
    if (pause) {
        std::unique_lock<std::mutex> lock(pauseMutex); // Lock the mutex for pausing
        paused = true; // Set the paused state
        LOG_INFO("Emulator paused.");
    } else {
        // Implementation for resuming the emulation (if needed)
        // This could involve resetting a flag in the CPU or Emulator class
        // to indicate that the emulation should resume.
        LOG_INFO("Emulator resumed.");
        std::unique_lock<std::mutex> lock(pauseMutex);
        paused = false; // Reset the paused state
        pauseCondition.notify_all(); // Notify any waiting threads
    }
}
bool Emulator::loadGame(const std::string& gamePath) {
    LOG_INFO("Loading game: " + gamePath);

    if (emulationActive.load()) { // Check emulationActive instead of running
        LOG_ERROR("Cannot load game while emulation is active");
        return false;
    }

    if (loaded) {
        LOG_WARNING("Another game is loaded, unloading first");
        if (!unloadGame()) {
            return false;
        }
    }

    std::unique_ptr<Cart> cart = std::make_unique<Cart>();
    if (!cart->load(gamePath)) {
        LOG_ERROR("Failed to load ROM file");
        return false;
    }

    // Ensure MemoryController exists before attaching cart
    if (!memoryController) {
         LOG_ERROR("Memory Controller not initialized before attaching cart");
         return false;
    }

    if (!memoryController->attachCart(std::move(cart))) {
        LOG_ERROR("Failed to attach cart to memory controller");
        return false;
    }

    // Reset CPU after loading a new game
    if (cpu) {
        cpu->Reset();
    } else {
        LOG_ERROR("CPU not initialized, cannot reset after loading game.");
        unloadGame(); // Unload cart if CPU isn't ready
        return false;
    }
    // Optionally reset PPU and Timer as well if needed

    loaded = true;
    LOG_INFO("Game loaded successfully");
    return true;
}

bool Emulator::unloadGame()
{
    LOG_INFO("Unloading game");

    if (emulationActive.load()) {
        LOG_ERROR("Cannot unload game while emulation is active");
        return false;
    }
    if (!loaded) {
        LOG_WARNING("No game loaded, nothing to unload");
        return true; // Nothing to unload
    }
    if (memoryController) {
        if (!memoryController->detachCart()) {
            LOG_ERROR("Failed to detach cart from memory controller");
            return false;
        }
    } else {
        LOG_ERROR("Memory Controller not initialized, cannot detach cart");
        return false;
    }
    if (cpu) {
        cpu->Reset(); // Reset CPU state
    } else {
        LOG_ERROR("CPU not initialized, cannot reset after unloading game.");
        return false;
    }
    if (ppu) {
        ppu->reset(); // Reset PPU state
    } else {
        LOG_ERROR("PPU not initialized, cannot reset after unloading game.");
        return false;
    }
    if (timer) {
        timer->reset(); // Reset Timer state
    } else {
        LOG_ERROR("Timer not initialized, cannot reset after unloading game.");
        return false;
    }
    loaded = false; // Set loaded to false after unloading
    LOG_INFO("Game unloaded successfully");
    return true;
}

void Emulator::run() {
    if (!loaded) {
        LOG_ERROR("No game loaded. Cannot run emulation.");
        return;
    }
    if (emulationActive.load()) {
        LOG_WARNING("Emulation is already running.");
        return;
    }

    // Start emulation in separate thread
    startEmulation();

    // Main thread handles SDL events and rendering
    running = true; // Keep the main loop running
    while (running) {
        // Process SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false; // Signal main loop and emulation thread to stop
                emulationActive.store(false); // Ensure emulation thread knows to exit
                pauseCondition.notify_all(); // Wake up thread if paused
            }
            // Handle input
            handleInput(event);
        }

        // Render the current frame
        render();

        // Control rendering frame rate (smoother than the emulation thread)
        SDL_Delay(16); // ~60fps
    }

    // Clean up the emulation thread AFTER the main loop exits
    stopEmulation();
}

void Emulator::cleanup() {
    LOG_INFO("Cleaning up emulator resources");

    // Ensure emulation is stopped first
    stopEmulation();

    if (loaded) {
        unloadGame();
    }

    cpu.reset();
    ppu.reset();
    timer.reset(); // Reset timer pointer
    memoryController.reset(); // Reset memory controller pointer

    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    TTF_Quit();
    SDL_Quit();

    LOG_INFO("Cleanup complete");
}

// This function seems like a duplicate or debug version of emulationLoop.
// Keeping it for now, but likely should be removed or merged with emulationLoop.
void Emulator::update()
{
    int cyclesThisUpdate = 0;
    const int CYCLES_PER_UPDATE = CYCLES_PER_FRAME; // Or some other value if this is for debugging

    while (cyclesThisUpdate < CYCLES_PER_UPDATE) {
        if (!cpu) {
            LOG_ERROR("CPU not initialized in update()");
            return;
        }
        // LOG_DEBUG("Debug mode value: " + std::to_string(debugMode.load())); // Can be noisy
        static int totalCycles = 0; // Static variable here might be problematic if update() is called repeatedly

        int cycles = cpu->ExecuteNextOpcode();
        if (cycles < 0) {
             LOG_ERROR("CPU execution error in update()");
             running = false; // Stop emulation on error
             emulationActive.store(false);
             return;
        }
        cyclesThisUpdate += cycles;
        totalCycles += cycles;

        // *** Update Timer and PPU here as well if this function is used ***
        if (timer) timer->update(cycles);
        if (ppu) ppu->update(cycles);
        // cyclesThisUpdate += handleInterrupts(); // handleInterrupts doesn't return cycles

        // Log less frequently
        // if (totalCycles % 10000 < cycles) {
        //     LOG_DEBUG("Update() CPU cycles: " + std::to_string(totalCycles) +
        //              ", PPU enabled: " + (ppu ? std::to_string(ppu->isLCDEnabled()) : "N/A"));
        // }
        if (cyclesThisUpdate >= CYCLES_PER_UPDATE) {
            break;
        }
    }
    // LOG_INFO("Update() completed with cycles: " + std::to_string(cyclesThisUpdate)); // Can be noisy
}

int Emulator::handleInterrupts() { // Should probably return void now
    if (!cpu) {
        return 0; // Return 0 cycles if no CPU
    }
    cpu->handleInterrupts(); // Call the CPU's handler
    // Interrupt handling cycles (20 T-cycles) are implicitly handled
    // by the fact that the CPU doesn't execute another instruction
    // during the interrupt service routine fetch/setup.
    // If precise cycle accounting is needed *outside* the CPU,
    // cpu->handleInterrupts could return the cycle cost.
    return 0; // Currently returns void, so return 0 cycles.
}
void Emulator::RequestInterrupt(BYTE interruptBit) {
    if (!cpu) {
        LOG_ERROR("CPU not initialized, cannot request interrupt");
        return;
    }
    cpu->RequestInterrupt(interruptBit);
}
void Emulator::startEmulation() {
    if (emulationActive.load()) {
        LOG_WARNING("Emulation already running");
        return;
    }
    if (!loaded) {
        LOG_ERROR("No game loaded, cannot start emulation.");
        return;
    }

    // running = true; // Set by run()
    paused = false;
    emulationActive.store(true); // Use store for atomic bool

    // Start emulation in a separate thread
    emulatorThread = std::thread(&Emulator::emulationLoop, this);
    LOG_INFO("Emulation thread started");
}

void Emulator::stopEmulation() {
    if (!emulationActive.load()) { // Check if active
        return;
    }

    emulationActive.store(false); // Signal thread to stop

    // Wake up the thread if it's paused
    {
        // std::lock_guard<std::mutex> lock(screenBufferMutex); // No need to lock screen buffer mutex here
        pauseCondition.notify_all();
    }

    // Wait for thread to finish
    if (emulatorThread.joinable()) {
        emulatorThread.join();
    }

    // running should be controlled by the main loop (SDL events)
    // running = false;

    LOG_INFO("Emulation thread stopped");
}

void Emulator::setEmulationSpeed(float speed) {
    if (speed <= 0.0f) {
        LOG_WARNING("Attempted to set invalid emulation speed: " + std::to_string(speed));
        return;
    }
    emulationSpeed.store(speed);
    LOG_INFO("Emulation speed set to " + std::to_string(speed) + "x");
}
void Emulator::handleInput(const SDL_Event& event) {
    std::lock_guard<std::mutex> lock(inputMutex); // Lock the joypad state
    // LOG_INFO("Event received: " + std::to_string(event.type)); // Can be very noisy

    if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
        bool pressed = (event.type == SDL_EVENT_KEY_DOWN);

        SDL_Keycode keyCode = event.key.key;

        // Handle Emulator controls first
        if (keyCode == SDLK_P) {
            if (pressed) pauseEmulation(!paused); // Toggle pause on press
            return;
        }
        if (keyCode == SDLK_O) {
            if (pressed) toggleDebugMode(); // Toggle debug on press
            return;
        }
        // Speed controls (example)
        if (keyCode == SDLK_EQUALS || keyCode == SDLK_PLUS) { // Increase speed
             if (pressed) setEmulationSpeed(emulationSpeed.load() * 1.1f);
             return;
        }
        if (keyCode == SDLK_MINUS) { // Decrease speed
             if (pressed) setEmulationSpeed(emulationSpeed.load() / 1.1f);
             return;
        }


        // Handle Game Boy controls
        if (!keyMap.count(keyCode)) {
            // LOG_WARNING("Key not mapped: " + std::to_string(keyCode));
            return;
        }

        int key = keyMap[keyCode];
        if (key >= 0) { // Ensure it's a valid Game Boy key index
            keyFunction_array[pressed ? 1 : 0](key); // Call KeyPressed or KeyReleased
        }
    }
}

void Emulator::toggleDebugMode()
{
    debugMode.store(!debugMode.load());
    if (debugMode.load()) {
        LOG_INFO("Debug mode enabled");
    } else {
        LOG_INFO("Debug mode disabled");
    }
}

void Emulator::emulationLoop()
{
    // Timing variables
    auto lastTime = std::chrono::high_resolution_clock::now();
    double accumulatedCycles = 0.0; // Use double for better precision

    while (emulationActive.load()) { // Use atomic bool for loop condition
        // Handle paused state
        { // Scope for unique_lock
            std::unique_lock<std::mutex> lock(pauseMutex);  // Use the pause mutex here
            pauseCondition.wait(lock, [this]() { return !paused || !emulationActive; });
        } // Lock released here

        if (!emulationActive.load()) break; // Exit if stopped while paused

        // Calculate target cycles for this frame based on speed
        double targetCycles = static_cast<double>(CYCLES_PER_FRAME) * emulationSpeed.load();
        accumulatedCycles = 0.0; // Reset accumulated cycles for the frame

        // Run CPU cycles for one frame's worth of time
        while (accumulatedCycles < targetCycles && emulationActive.load()) {
            if (!cpu) {
                LOG_ERROR("CPU is null in emulation loop!");
                emulationActive.store(false); // Stop emulation
                break;
            }
             if (!timer) {
                LOG_ERROR("Timer is null in emulation loop!");
                emulationActive.store(false); // Stop emulation
                break;
            }
             if (!ppu) {
                LOG_ERROR("PPU is null in emulation loop!");
                emulationActive.store(false); // Stop emulation
                break;
            }

            int cycles = cpu->ExecuteNextOpcode();
            if (cycles < 0) {
                LOG_ERROR("CPU execution error in emulation loop");
                emulationActive.store(false); // Stop emulation on error
                break;
            }

            accumulatedCycles += cycles;

            // *** Update Timer and PPU with the cycles executed ***
            timer->update(cycles);
            ppu->update(cycles);

            // Interrupt handling cycles are implicitly handled by CPU execution flow
            // No need to add extra cycles here unless handleInterrupts returns a cost
            // handleInterrupts(); // Called at the start of ExecuteNextOpcode now
        }

        // Frame timing (optional, can rely on VSync in render or SDL_Delay)
        // This simple sleep might not be the most accurate timing method.
        // Consider more advanced timing techniques if needed.
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastTime);
        int64_t targetFrameDuration = static_cast<int64_t>(1000000.0 / TARGET_FPS); // Target based on 60fps

        if (frameDuration.count() < targetFrameDuration) {
            std::this_thread::sleep_for(std::chrono::microseconds(targetFrameDuration - frameDuration.count()));
        }
        lastTime = std::chrono::high_resolution_clock::now(); // Update time for next frame calculation
    }
     LOG_INFO("Exiting emulation loop.");
}
void Emulator::render() {
    if (!renderer || !texture || !ppu) {
        // Log error only if components are unexpectedly null *after* successful init
        if (running) { // Avoid logging if called during cleanup
             LOG_ERROR("Render components not initialized!");
        }
        return;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(renderer);

    // Use atomic load for debugMode check
    if (debugMode.load()) {
        // Optionally render debug info or a specific pattern
        // For now, just render the normal buffer even in debug
        // ppu->debugFillTestPattern(); // Call this if you want a test pattern in debug
        // LOG_DEBUG("Rendering in debug mode");
    }

    // Lock before accessing the screen buffer from PPU
    {
        std::lock_guard<std::mutex> lock(screenBufferMutex); // Use the screen buffer mutex
        void* texturePixels = nullptr;
        int pitch = 0;

        // *** Add check before locking ***
        if (!texture) {
             LOG_ERROR("Texture pointer is null before SDL_LockTexture!");
             return;
        }

        // Lock the texture for writing
        if (SDL_LockTexture(texture, nullptr, &texturePixels, &pitch) != 0) {
            // *** Improved Error Logging ***
            const char* sdlError = SDL_GetError(); // Get error *immediately*
            LOG_ERROR("Failed to lock texture: " + (sdlError ? std::string(sdlError) : "Unknown SDL Error"));
            return; // Skip rendering if lock fails
        }

        // Copy the PPU buffer to the texture
        // Check if texturePixels is valid before memcpy
        if (texturePixels) {
             // Ensure the PPU buffer size matches the expected texture size
             const auto& ppuBuffer = ppu->getScreenBuffer();
             size_t expectedSize = static_cast<size_t>(SCREEN_PIXELS_WIDTH) * SCREEN_PIXELS_HEIGHT * sizeof(Uint32);
             if (ppuBuffer.size() * sizeof(Uint32) == expectedSize) {
                 memcpy(texturePixels, ppuBuffer.data(), expectedSize);
             } else {
                  LOG_ERROR("PPU buffer size mismatch! Expected: " + std::to_string(expectedSize) + ", Got: " + std::to_string(ppuBuffer.size() * sizeof(Uint32)));
                  // Optionally fill texture with a solid color to indicate error
                  memset(texturePixels, 0xFF, pitch * SCREEN_PIXELS_HEIGHT); // Fill with white on error
             }
        } else {
             LOG_ERROR("SDL_LockTexture succeeded but returned null pixels pointer!");
        }


        // Unlock the texture
        SDL_UnlockTexture(texture);
    } // Mutex lock released here

    // Render the texture to the screen
    // *** Add check before rendering ***
    if (!renderer || !texture) {
         LOG_ERROR("Renderer or Texture became null before SDL_RenderTexture!");
         return;
    }
    SDL_RenderTexture(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

BYTE Emulator::GetJoypadState() {
    // This logic seems okay, but ensure memoryController->read/write are thread-safe
    // if accessed from multiple threads (input thread vs emulation thread).
    // Using a mutex for joypad state itself is good.
    std::lock_guard<std::mutex> lock(inputMutex); // Lock joypad state access
    BYTE joypadRequest = memoryController->read(JOYPAD_REGISTER);
    // Start with lower nibble high (unpressed), upper nibble from register write
    BYTE joypadOutput = (joypadRequest & 0xF0) | 0x0F;

    if (!(joypadRequest & JOYPAD_SELECT_DIRECTIONS)) { // Bit 4 low: Directions selected
        joypadOutput &= (joypad.GetJoypadState() & 0x0F); // AND with lower nibble state
    } else if (!(joypadRequest & JOYPAD_SELECT_BUTTONS)) { // Bit 5 low: Buttons selected
        joypadOutput &= (joypad.GetJoypadState() >> 4);   // AND with upper nibble state (shifted down)
    } else {
        // If neither is selected, all buttons/directions read as unpressed (high)
        joypadOutput |= 0x0F;
    }
    return joypadOutput;
}
void Emulator::KeyPressed(int key) {
     std::lock_guard<std::mutex> lock(inputMutex); // Lock joypad state access
    bool previouslyUnset = (joypad.GetJoypadState() & (1 << key)) != 0;

    // Set the bit in joypadState to 0 (pressed)
    joypad.KeyPressed(key);

    // Determine if the pressed key belongs to the currently selected group
    BYTE joypadRequest = memoryController->read(JOYPAD_REGISTER);
    bool buttonColumnSelected = !(joypadRequest & JOYPAD_SELECT_BUTTONS);
    bool directionColumnSelected = !(joypadRequest & JOYPAD_SELECT_DIRECTIONS);

    bool isButton = (key >= JOYPAD_A); // A, B, Select, Start
    bool isDirection = !isButton;      // Right, Left, Up, Down

    bool requestInterrupt = false;
    if ((isButton && buttonColumnSelected) || (isDirection && directionColumnSelected)) {
        requestInterrupt = true;
    }


    // Request interrupt only if the selected group had a key pressed
    // and that specific key was previously unpressed.
    if (requestInterrupt && previouslyUnset) {
        // Request Joypad Interrupt (Bit 4 of IF register)
        if (cpu) { // Check if CPU exists
             cpu->RequestInterrupt(JOYPAD_INTERRUPT_BIT);
             LOG_DEBUG("Joypad interrupt requested for key: " + std::to_string(key));
        }
    }
}
void Emulator::KeyReleased(int key) {
     std::lock_guard<std::mutex> lock(inputMutex); // Lock joypad state access
    joypad.KeyReleased(key);
}



