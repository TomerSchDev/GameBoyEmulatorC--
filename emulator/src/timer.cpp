#include "timer.h"
#include "logger.h"

Timer::Timer(std::shared_ptr<MemoryController> memory)
    : memoryController(memory)
    , m_Counter(0)
    , m_Modulo(0)
    , m_Control(0)
    , m_TimerCounter(1024)
    , m_IsEnabled(false)
    , m_DividerCounter(0)
    , m_DividerRegister(0)
{
    LOG_INFO("Timer initialized");
}
bool Timer::isEnabled() const {
    return (m_Control & TIMER_ENABLE_BIT) != 0;
}
void Timer::updateDividerRegister(int cycles) {
    m_DividerCounter += cycles;
    
    if (m_DividerCounter >= DIVIDER_MAX) {
        m_DividerCounter = 0;
        m_DividerRegister++;
        LOG_DEBUG("Divider Register incremented to: " + std::to_string(m_DividerRegister));
    }
}
void Timer::resetDividerRegister() {
    m_DividerRegister = 0;
    m_DividerCounter = 0;
    LOG_DEBUG("Divider Register reset to 0");
}
void Timer::update(int cycles) {
    if (!isEnabled()) {
        return;
    }
    // Update divider register first
    updateDividerRegister(cycles);

    m_TimerCounter -= cycles;

    if (m_TimerCounter <= 0) {
        m_TimerCounter = CPU_CLOCK_SPEED / getFrequency();
        m_Counter++;
        
        if (m_Counter == 0) {
            m_Counter = m_Modulo;
            
            // Set timer interrupt flag
            BYTE interruptFlags = memoryController->read(IF_REGISTER);
            interruptFlags |= TIMER_INTERRUPT_BIT;
            memoryController->write(IF_REGISTER, interruptFlags);
            
            LOG_DEBUG("Timer overflow - Interrupt requested");
        }
    }
}

bool Timer::isInterruptRequested() const {
    BYTE interruptFlags = memoryController->read(IF_REGISTER);
    return (interruptFlags & TIMER_INTERRUPT_BIT) != 0;
}

void Timer::resetInterruptRequest() {
    BYTE interruptFlags = memoryController->read(IF_REGISTER);
    interruptFlags &= ~TIMER_INTERRUPT_BIT;
    memoryController->write(IF_REGISTER, interruptFlags);
    LOG_DEBUG("Timer interrupt cleared");
}

BYTE Timer::read(WORD address) const {
    switch (address) {
        case TIMA: return m_Counter;
        case TMA:  return m_Modulo;
        case TMC:  return m_Control;
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
            break;
        case TMA:
            m_Modulo = value;
            break;
        case DIV_REGISTER:
            resetDividerRegister();  // Any write resets the register
            break;
        case TMC:
            {
                BYTE currentFreq = getClockFreq();
                m_Control = value;
                m_IsEnabled = (value & TIMER_ENABLE_BIT) != 0;
                
                // Check if frequency changed
                if (currentFreq != getClockFreq()) {
                    setClockFreq();
                    LOG_DEBUG("Timer frequency changed to: " + 
                             std::to_string(getFrequency()) + " Hz");
                }
            }
            break;
    }
}

BYTE Timer::getClockFreq() const {
    return m_Control & CLOCK_SELECT_MASK;
}

void Timer::setClockFreq() {
    BYTE freq = getClockFreq();
    switch (freq) {
        case 0: m_TimerCounter = 1024; break; // 4096 Hz
        case 1: m_TimerCounter = 16;   break; // 262144 Hz
        case 2: m_TimerCounter = 64;   break; // 65536 Hz
        case 3: m_TimerCounter = 256;  break; // 16384 Hz
    }
}

int Timer::getFrequency() const {
    switch (m_Control & CLOCK_SELECT_MASK) {
        case 0: return 4096;    // 4096 Hz
        case 1: return 262144;  // 262144 Hz
        case 2: return 65536;   // 65536 Hz
        case 3: return 16384;   // 16384 Hz
        default: return 4096;   // Default frequency
    }
}