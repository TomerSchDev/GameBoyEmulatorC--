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
void Emulator::handleEvents()
{
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
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    const std::array<Uint32, 160 * 144>& screenBuffer = ppu->getScreenBuffer();

    // Pitch is the width of the texture in bytes
    int pitch = 160 * sizeof(Uint32);

    // Update the texture with the new data
    SDL_UpdateTexture(texture, nullptr, screenBuffer.data(), pitch);

    // Copy the texture to the renderer
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);

    // Update the screen
    SDL_RenderPresent(renderer);

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