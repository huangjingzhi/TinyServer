
#include "Logger.h"
#include <ctime>
#include <iomanip>
#include <chrono>
#include <vector>

std::vector<std::string>  g_logLevelTag{
     "DEBUG", "TRACE", "INFO", "WARN", "ERROR", "FATAL"
};

Logger::Logger(const std::string& fileName, LogLevel level) : m_logFile(fileName, std::ios::out | std::ios::app), m_logLevel(level) {
    if (!m_logFile.is_open()) {
        std::cerr << "Failed to open log file: " << fileName << std::endl;
        std::abort();
    }
}

Logger::~Logger() {
    if (m_logFile.is_open()) {
        m_logFile.close();
    }
}

void Logger::SetLogLevel(LogLevel level) {
    m_logLevel = level;
}

void Logger::Log(LogLevel level, const std::string &message) {
    std::unique_lock<std::mutex> lock(m_busyMutex);

    if ((m_logLevel <= level) && m_logFile.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto nowTime = std::chrono::system_clock::to_time_t(now);
        std::tm nowTimeTm = *std::localtime(&nowTime);
        char timeBuffer[32];
        strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", &nowTimeTm);

        m_logFile << "[" << timeBuffer << "] [" << g_logLevelTag[level] << "] " << message << std::endl;
    }
}
 Logger LOGGER{"server.log"};