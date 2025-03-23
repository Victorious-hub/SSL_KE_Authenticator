#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>

enum LogLevel { DEBUG, INFO, WARNING, ERROR, CRITICAL };

class Logger {
public:
    Logger() {}
    Logger(const std::string& filename);
    ~Logger();

    void log(LogLevel level, const std::string& message);

private:
    std::ofstream logFile;
    std::string levelToString(LogLevel level);
};

#endif // LOGGER_H