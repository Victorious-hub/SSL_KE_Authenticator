#include <ctime>
#include <iostream>
#include <sstream>

#include "logger.h"

Logger::Logger(const std::string& filename)
{
    logFile.open(filename, std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Error opening log file." << std::endl;
    }
}

Logger::~Logger()
{
    logFile.close();
}

void Logger::log(LogLevel level, const std::string& message)
{
    time_t now = time(0);
    tm* timeinfo = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

    std::ostringstream logEntry;
    logEntry << "[" << timestamp << "] " << levelToString(level) << ": " << message << std::endl;

    std::cout << logEntry.str();
    if (logFile.is_open()) {
        logFile << logEntry.str();
        logFile.flush();
    }
}

std::string Logger::levelToString(LogLevel level)
{
    switch (level) {
    case DEBUG:
        return "DEBUG";
    case INFO:
        return "INFO";
    case WARNING:
        return "WARNING";
    case ERROR:
        return "ERROR";
    case CRITICAL:
        return "CRITICAL";
    default:
        return "UNKNOWN";
    }
}
