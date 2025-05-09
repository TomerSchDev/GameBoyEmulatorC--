#include "timer.h"
#include "logger.h"
#include <stdexcept> // For std::runtime_error

// Define IF register address if not defined elsewhere
#ifndef IF_REGISTER
#define IF_REGISTER 0xFF0F
#endif
// Define Timer interrupt bit if not defined elsewhere
#ifndef TIMER_INTERRUPT_BIT
#define TIMER_INTERRUPT_BIT  0x04
#endif


namespace GB {

// Define clock speed if not available elsewhere (Ensure this matches your CPU clock)
const int CPU_CLOCK_SPEED = 4194304; // Hz (T-cycles per second)

Timer::Timer(std::shared_ptr<MemoryController> memory)
    : memoryController(memory),
      m_Counter(0),          // TIMA (0xFF05)
      m_Modulo(0),           // TMA (0xFF06)
      m_Control(0),          // TAC (0xFF07)
      m_IsEnabled(false),
      m_TimerCounter(0),     // Internal counter for TIMA increments
      m_DividerCounter(0),   // Internal counter for DIV increments
      m_DividerRegister(0)   // DIV (0xFF04) - Actual register value
{
    // Reset internal counters based on initial TAC state (usually 0)
    setClockFreq(); // Initialize m_TimerCounter based on TAC=0
    LOG_INFO("Timer initialized");
}

bool Timer::isEnabled() const {
    // Read TAC register directly to get the most up-to-date enable status
    return (memoryController->read(TMC) & TIMER_ENABLE_BIT) != 0;
}

// Updates the DIV register (0xFF04) based on T-cycles passed
void Timer::updateDividerRegister(int cycles) {
    LOG_DEBUG("Updating divider register with cycles: " + std::to_string(cycles));
    m_DividerCounter += cycles; // Accumulate T-cycles
    LOG_DEBUG("Divider Counter after increment: " + std::to_string(m_DividerCounter));
    // DIV increments every 256 T-cycles (16384 Hz)
    while (m_DividerCounter >= 256) {
        LOG_DEBUG("Divider Counter before decrement: " + std::to_string(m_DividerCounter));
        m_DividerCounter -= 256;
        m_DividerRegister++; // Let it wrap around naturally (BYTE)
        LOG_DEBUG("Divider Register incremented: " + std::to_string(m_DividerRegister));
        // LOG_DEBUG("Divider Register incremented to: " + std::to_string(m_DividerRegister)); // Can be noisy
    }
}

// Resets the DIV register (0xFF04) when written to
void Timer::resetDividerRegister() {
    m_DividerRegister = 0;
    m_DividerCounter = 0; // Also reset the internal counter
    LOG_DEBUG("Divider Register reset to 0 by write");
}

// Main update function called with T-cycles from CPU execution
void Timer::update(int cycles) {
        // Update divider register unconditionally
    LOG_DEBUG("Timer update called with cycles: " + std::to_string(cycles));
    updateDividerRegister(cycles);

    // Check if timer is enabled via TAC register
    if (!isEnabled()) {
        LOG_DEBUG("Timer is disabled, skipping update.");
        return;
    }

    // Decrement the counter towards the next TIMA increment
    m_TimerCounter -= cycles;
    LOG_DEBUG("Timer Counter after decrement: " + std::to_string(m_TimerCounter));
    // Check if enough cycles have passed for a TIMA increment
    while (m_TimerCounter <= 0) {
        LOG_DEBUG("Timer Counter reached 0, incrementing TIMA.");
        // Reload the counter with the period for the current frequency
        m_TimerCounter += getFrequencyPeriod(); // Add the period back
        
        LOG_DEBUG("Timer Counter after incremnt from getFrequencyPeriod : " + std::to_string(m_TimerCounter));
        // Increment TIMA (0xFF05)
        m_Counter++;

        // Check for TIMA overflow
        if (m_Counter == 0) { // Overflow occurred (0xFF -> 0x00)
            // Reload TIMA with TMA (0xFF06)
            LOG_DEBUG("Timer overflow - Reloading TIMA with TMA: " + std::to_string(m_Modulo));
            m_Counter = m_Modulo;

            // Request Timer Interrupt (Set bit 2 in IF register 0xFF0F)
            memoryController->write(IF_REGISTER,
                memoryController->read(IF_REGISTER) | TIMER_INTERRUPT_BIT);
            LOG_DEBUG("Timer Interrupt Requested (IF bit 2 set)"); // *** ADD THIS LOG ***

            LOG_DEBUG("Timer overflow - Interrupt requested. TIMA reloaded with TMA=" + std::to_string(m_Modulo));
        }
    }
}


BYTE Timer::read(WORD address) const {
    switch (address) {
        case TIMA: return m_Counter;
        case TMA:  return m_Modulo;
        case TMC:  return m_Control; // Return the internally tracked control value
        case DIV_REGISTER: return m_DividerRegister; // Divider register read
        default:
            LOG_WARNING("Attempted to read from invalid timer address: 0x" +
                        std::to_string(address));
            return 0xFF; // Invalid read
    }
}

void Timer::write(WORD address, BYTE value) {
    switch (address) {
        case TIMA:
            m_Counter = value;
            // LOG_DEBUG("TIMA (0xFF05) written with value: " + std::to_string(value));
            break;
        case TMA:
            m_Modulo = value;
            // LOG_DEBUG("TMA (0xFF06) written with value: " + std::to_string(value));
            break;
        case DIV_REGISTER:
            resetDividerRegister();  // Any write resets the register and internal counter
            break;
        case TMC:
            {
                // Store the new control value internally
                m_Control = value;
                // Update the enabled state based on the new value
                m_IsEnabled = (value & TIMER_ENABLE_BIT) != 0;
                // Update the internal frequency counter immediately
                setClockFreq();
                LOG_DEBUG("Timer Control (TMC/0xFF07) written: 0x" + std::to_string(value) +
                          ", Enabled: " + (m_IsEnabled ? "Yes" : "No") +
                          ", Freq: " + std::to_string(getFrequency()) + " Hz");
            }
            break;
        default:
             LOG_WARNING("Attempted to write to invalid timer address: 0x" +
                        std::to_string(address) + " with value: " + std::to_string(value));
            break;
    }
}

bool Timer::isInterruptRequested() const
{
    // Check if the timer interrupt flag is set in the IF register (0xFF0F)
    BYTE ifRegister = memoryController->read(IF_REGISTER);
    return (ifRegister & TIMER_INTERRUPT_BIT) != 0;
}

void Timer::resetInterruptRequest()
{
    // Clear the timer interrupt request by resetting the corresponding bit in IF register
    BYTE ifRegister = memoryController->read(IF_REGISTER);
    memoryController->write(IF_REGISTER, ifRegister & ~TIMER_INTERRUPT_BIT);
    LOG_DEBUG("Timer interrupt request cleared in IF register (0xFF0F)");
}

void Timer::reset() {
    // Reset all timer registers to their initial values
    m_Counter = 0;         // Reset TIMA
    m_Modulo = 0;          // Reset TMA
    m_Control = 0;         // Reset TMC
    m_TimerCounter = 0;    // Reset internal counter
    m_IsEnabled = false;   // Disable timer
    m_DividerCounter = 0;  // Reset divider counter
    m_DividerRegister = 0; // Reset divider register
    
    LOG_DEBUG("Timer reset to initial state");
}
// Sets the internal timer counter reload value based on TAC frequency bits
void Timer::setClockFreq() {
    // We don't reset m_TimerCounter here, just calculate the period for the *next* reload
    // The current countdown continues until it hits <= 0.
    // The getFrequencyPeriod() function will return the correct value based on m_Control.
}

// Returns the frequency in Hz based on TAC bits
int Timer::getFrequency() const {
    switch (m_Control & CLOCK_SELECT_MASK) {
        case 0: return 4096;    // 4.096 KHz
        case 1: return 262144;  // 262.144 KHz
        case 2: return 65536;   // 65.536 KHz
        case 3: return 16384;   // 16.384 KHz
        default: return 4096;   // Should not happen
    }
}

// Returns the number of T-cycles per TIMA increment for the current frequency
int Timer::getFrequencyPeriod() const {
     switch (m_Control & CLOCK_SELECT_MASK) {
        case 0: return 1024; // CPU_CLOCK_SPEED / 4096
        case 1: return 16;   // CPU_CLOCK_SPEED / 262144
        case 2: return 64;   // CPU_CLOCK_SPEED / 65536
        case 3: return 256;  // CPU_CLOCK_SPEED / 16384
        default: return 1024; // Should not happen
    }
}

// Returns the current value of the DIV register
BYTE Timer::getDividerRegister() const {
    return m_DividerRegister;
}


} // namespace GB
