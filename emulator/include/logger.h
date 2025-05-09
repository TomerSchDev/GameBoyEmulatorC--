#pragma once
#include <string>
#include <fstream>
#include "common.h"  // Add this at the top

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
private:
    static Logger* instance;
    std::ofstream logFile;
    LogLevel currentLevel;
    std::string logFileName;
    Logger();
    Logger(const std::string& newLogFileName); // Constructor taking a filename

public:
    static Logger* getInstance();
    static Logger* getInstance(const std::string &logFileName);

    void setLogLevel(LogLevel level);
    void log(LogLevel level, const std::string& className, const std::string& message);
    void debug(const std::string& className, const std::string& message);
    void info(const std::string& className, const std::string& message);
    void warning(const std::string& className, const std::string& message);
    void error(const std::string& className, const std::string& message);
    ~Logger();

private:
    std::string getCurrentTimestamp();
    std::string logLevelToString(LogLevel level);
};

// Convenience macros with class name
#define LOG_DEBUG(message) Logger::getInstance()->debug(__FUNCTION__, message)
#define LOG_INFO(message) Logger::getInstance()->info(__FUNCTION__, message)
#define LOG_WARNING(message) Logger::getInstance()->warning(__FUNCTION__, message)
#define LOG_ERROR(message) Logger::getInstance()->error(__FUNCTION__, message)