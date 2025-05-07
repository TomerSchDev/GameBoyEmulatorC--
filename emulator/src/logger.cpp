#include <logger.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>

Logger* Logger::instance = nullptr;

Logger::Logger() : currentLevel(LogLevel::INFO) {
    logFile.open("emulator.log", std::ios::out | std::ios::app);
}

// ...existing destructor and getInstance...

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

// ...existing timestamp and level conversion methods...
Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

Logger* Logger::getInstance() {
    if (instance == nullptr) {
        instance = new Logger();
    }
    return instance;
}

void Logger::setLogLevel(LogLevel level) {
    currentLevel = level;
}

std::string Logger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
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


