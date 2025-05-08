#include <logger.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>

Logger* Logger::instance = nullptr;

// Default constructor
Logger::Logger() : currentLevel(LogLevel::INFO), logFileName("emulator.log") { // Initialize member logFileName
    logFile.open(this->logFileName, std::ios::out | std::ios::app); // Use this->logFileName
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file: " << this->logFileName << std::endl;
    }
}

// Constructor taking a filename
// Changed parameter to const std::string& and initialize the member logFileName.
Logger::Logger(const std::string& newLogFileName) : currentLevel(LogLevel::INFO), logFileName(newLogFileName) {
    logFile.open(this->logFileName, std::ios::out | std::ios::app); // Use this->logFileName
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file: " << this->logFileName << std::endl;
    }
}

// ...existing code...
void Logger::log(LogLevel level, const std::string& className, const std::string& message) {
    if (level >= currentLevel) {
        std::string output = getCurrentTimestamp() + " [" + 
                           logLevelToString(level) + "] [" +
                           className + "] " + 
                           message + "\n";
        
        std::cout << output;
        if (logFile.is_open()) {
            logFile << output;
            logFile.flush();
        }
    }
}

void Logger::debug(const std::string& className, const std::string& message) {
    log(LogLevel::DEBUG, className, message);
}

void Logger::info(const std::string& className, const std::string& message) {
    log(LogLevel::INFO, className, message);
}

void Logger::warning(const std::string& className, const std::string& message) {
    log(LogLevel::WARNING, className, message);
}

void Logger::error(const std::string& className, const std::string& message) {
    log(LogLevel::ERROR, className, message);
}

Logger* Logger::getInstance(const std::string &fileNameParam) // Renamed parameter for clarity
{
    if (instance)
    {
        //check if the log file name is the same as the current one
        if (instance->logFileName == fileNameParam) { // Compare with the parameter
            return instance; // Return the existing instance if the log file name is the same
        }
        // If the log file name is different, delete the existing instance
        if(instance->logFile.is_open()){ // Check if file is open before closing
            instance->logFile.close(); // Close the existing log file if it's open
        }
        delete instance; // Delete the existing instance
        instance = nullptr; // Set the instance pointer to null
    }
    instance = new Logger(fileNameParam); // Create a new instance with the new log file name
    return instance;
}

// ...existing timestamp and level conversion methods...
Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

Logger* Logger::getInstance() {
    if (instance == nullptr) {
        instance = new Logger(); // Calls the default constructor
    }
    return instance;
}

void Logger::setLogLevel(LogLevel level) {
    currentLevel = level;
}

std::string Logger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm_snapshot;
    #ifdef _WIN32
    localtime_s(&tm_snapshot, &time);
    #else
    localtime_r(&time, &tm_snapshot); // POSIX thread-safe version
    #endif
    std::stringstream ss;
    ss << std::put_time(&tm_snapshot, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string Logger::logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR:   return "ERROR";
        default:               return "UNKNOWN";
    }
}


