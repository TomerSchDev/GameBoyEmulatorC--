#include <emulator.h>
#include <logger.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

Emulator::Emulator()
    : window(nullptr)
    , renderer(nullptr)
    , running(false)
    , loaded(false)
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
}

Emulator::~Emulator() {
    cleanup();
}
bool Emulator::init() {
    LOG_INFO("Initializing emulator...");

    if (!SDL_Init(SDL_INIT_VIDEO)){
        LOG_ERROR("SDL could not initialize! SDL_Error: " + std::string(SDL_GetError()));
        return false;
    }

    if (!TTF_Init()) {
        LOG_ERROR("SDL_ttf could not initialize! TTF_Error: " + std::string(SDL_GetError()));
        return false;
    }

    window = SDL_CreateWindow("GameBoy Emulator", WIDNOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window) {
        LOG_ERROR("Window could not be created! SDL_Error: " + std::string(SDL_GetError()));
        return false;
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        LOG_ERROR("Renderer could not be created! SDL_Error: " + std::string(SDL_GetError()));
        return false;
    }

    // Initialize core components
    memoryController = std::make_shared<MemoryController>();
    cpu = std::make_unique<CPU>(memoryController);
    ppu = std::make_unique<PPU>(memoryController);
    timer = std::make_unique<Timer>(memoryController);
    if (!memoryController || !cpu || !ppu || !timer) {
        LOG_ERROR("Failed to initialize core components");
        return false;
    }
    memoryController->attachEmulator(this);
    // Set initial state
    running = false;
    loaded = false;

    LOG_INFO("Emulator initialized successfully");
    return true;
}

bool Emulator::loadGame(const std::string& gamePath) {
    LOG_INFO("Loading game: " + gamePath);
    
    if (running) {
        LOG_ERROR("Cannot load game while emulator is running");
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

    if (!memoryController->attachCart(std::move(cart))) {
        LOG_ERROR("Failed to attach cart to memory controller");
        return false;
    }

    loaded = true;
    LOG_INFO("Game loaded successfully");
    return true;
}

void Emulator::run() {
    if (!loaded) {
        LOG_ERROR("Cannot run emulator - no game loaded");
        return;
    }

    running = true;
    LOG_INFO("Starting emulation loop");

    while (running) {
        Uint32 frameStart = SDL_GetTicks();

        handleEvents();
        update();
        render();

        // Cap framerate
        int frameTime = SDL_GetTicks() - frameStart;
        if (FRAME_DELAY_MS > frameTime) {
            LOG_DEBUG("Frame time: " + std::to_string(frameTime) + "ms");
            // Delay to maintain target frame rate
            LOG_DEBUG("Delaying for: " + std::to_string(FRAME_DELAY_MS - frameTime) + "ms");
            SDL_Delay(FRAME_DELAY_MS - frameTime);
        }
    }
}


void Emulator::cleanup() {
    LOG_INFO("Cleaning up emulator resources");

    if (loaded) {
        unloadGame();
    }

    cpu.reset();
    ppu.reset();
    memoryController.reset();

    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }

    TTF_Quit();
    SDL_Quit();

    LOG_INFO("Cleanup complete");
}

void Emulator::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            running = false;
        } else if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
            SDL_Keycode keyCode = event.key.key;
            LOG_DEBUG("Key event: " + std::to_string(keyCode));
            if (!keyMap.count(keyCode)) {
                LOG_WARNING("Key not mapped: " + std::to_string(keyCode));
                continue;
            }
            // Check if the key is mapped
            int key = keyMap[keyCode];
            if (key == -2)
            {
                //spaciel key handling TODO
                running = false;
            }
            // Handle key press/release
            if (event.type == SDL_EVENT_KEY_DOWN) {
                KeyPressed(key);
            }else
            {
                KeyReleased(key);
            }
        }
    }
}
void Emulator::update()
{
    int cyclesThisUpdate = 0;

    while (cyclesThisUpdate < CYCLES_PER_FRAME) {
        if (!cpu) {
            LOG_ERROR("CPU not initialized");
            return;
        }

        int cycles = cpu->ExecuteNextOpcode();
        cyclesThisUpdate += cycles;

        timer->update(cycles);
        ppu->update(cycles);
        handleInterrupts();
    }
}

void Emulator::handleInterrupts() {
    if (!cpu) {
        return;
    }

    BYTE interruptFlags = memoryController->read(IF_REGISTER);
    BYTE interruptEnable = memoryController->read(IE_REGISTER);
    BYTE pendingInterrupts = interruptFlags & interruptEnable;

    if (pendingInterrupts) {
        cpu->handleInterrupts(pendingInterrupts);
    }
}

void Emulator::render() {
    SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 255);
    SDL_RenderClear(this->renderer);

    const std::array<Uint32, 160 * 144>& screenBuffer = ppu->getScreenBuffer();

    // Pitch is the width of the texture in bytes
    int pitch = 160 * sizeof(Uint32);

    // Update the texture with the new data
    SDL_UpdateTexture(this->texture, nullptr, screenBuffer.data(), pitch);

    // Copy the texture to the renderer
    SDL_RenderTexture(this->renderer, this->texture, nullptr, nullptr);

    // Update the screen
    SDL_RenderPresent(this->renderer);

}

BYTE Emulator::GetJoypadState() {
    BYTE joypadRequest = memoryController->read(JOYPAD_REGISTER);
    BYTE joypadOutput = joypadRequest | 0x0F; // Initialize with all buttons unpressed (1)

    // flip all the bits
    joypadOutput ^= 0xFF;

    // are we interested in the standard buttons?
    if (!(joypadRequest & JOYPAD_SELECT_DIRECTIONS)) {
        BYTE topJoypad = joypad.GetJoypadState() >> 4;
        topJoypad |= 0xF0; // turn the top 4 bits on
        joypadOutput &= topJoypad; // show what buttons are pressed
    } else if (!(joypadRequest & JOYPAD_SELECT_BUTTONS)) { // directional buttons
        BYTE bottomJoypad = joypad.GetJoypadState() & 0xF;
        bottomJoypad |= 0xF0;
        joypadOutput &= bottomJoypad;
    }
    return joypadOutput;
}
void Emulator::KeyPressed(int key) {
    bool previouslyUnset = (joypad.GetJoypadState() & (1 << key)) != 0;

    // Set the bit in joypadState to 0 (pressed)
    joypad.KeyPressed(key);

    BYTE joypadRequest = memoryController->read(JOYPAD_REGISTER);
    bool button = key >= JOYPAD_A;
    bool requestInterrupt = false;

    // Check if the button press should request an interrupt
    if (button && !(joypadRequest & JOYPAD_SELECT_BUTTONS)) {
        requestInterrupt = true;
    } else if (!button && !(joypadRequest & JOYPAD_SELECT_DIRECTIONS)) {
        requestInterrupt = true;
    }

    // Request interrupt if necessary and the button was previously unpressed
    if (requestInterrupt && previouslyUnset) {
        BYTE flags = memoryController->read(IF_REGISTER);
        flags |= JOYPAD_INTERRUPT_BIT;
        memoryController->write(IF_REGISTER, flags);
    }
}
void Emulator::KeyReleased(int key) {
    joypad.KeyReleased(key);
}
bool Emulator::unloadGame() {
    if (!loaded) {
        LOG_WARNING("No game loaded to unload");
        return true;
    }

    if (memoryController) {
        memoryController->detachCart();
    }

    loaded = false;
    LOG_INFO("Game unloaded successfully");
    return true;
}