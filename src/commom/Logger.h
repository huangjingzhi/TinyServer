
#include <fstream>
#include <iostream>
#include <string>
#include <mutex>

#ifndef LOGGER_H
#define LOGGER_H

typedef enum  LogLevel {
    DEBUG = 0,
    TRACE = 1,
    INFO  = 2,
    WARN  = 3,
    ERROR = 4,
    FATAL = 5
} LogLevel;

class Logger {
public:
    Logger(const std::string& fileName, LogLevel level=INFO);
    ~Logger();

    void SetLogLevel(LogLevel level);
    void Log(LogLevel level, const std::string& message);
private:
    std::ofstream m_logFile;
    LogLevel m_logLevel;
    std::mutex m_busyMutex;
};

extern Logger LOGGER;
#endif