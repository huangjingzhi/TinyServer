
#include <fstream>
#include <iostream>
#include <string>
#include <mutex>

#ifndef LOGGER_H
#define LOGGER_H

typedef enum  LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO  = 2,
    WARN  = 3,
    ERROR = 4,
    FATAL = 5
} LogLevel;

class Logger {
public:
    Logger(const std::string& fileName);
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void SetLogLevel(LogLevel level);
    void Log(LogLevel level, const std::string& message);

private:
    std::ofstream m_logFile;
    LogLevel m_logLevel;
    std::mutex m_busyMutex;
};

static Logger LOGGER{"server.log"};

#endif