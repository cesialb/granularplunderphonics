/**
 * @file Logger.cpp
 * @brief Implementation of the Logger class
 */

#include "Logger.h"
#include <ctime>
#include <iomanip>
#include <filesystem>

namespace GranularPlunderphonics {

// Initialize static members
Logger::Level Logger::sGlobalLevel = Logger::Level::Info;
std::string Logger::sLogFilePath = "GranularPlunderphonics.log";
std::ofstream Logger::sLogFile;
bool Logger::sInitialized = false;
std::mutex Logger::sLogMutex;

//------------------------------------------------------------------------
Logger::Logger(const std::string& name)
    : mName(name)
{
    if (!sInitialized) {
        // Try to initialize with default settings if not already initialized
        initialize();
    }
}

//------------------------------------------------------------------------
bool Logger::initialize(const std::string& logFilePath, size_t maxFileSize, size_t maxFiles)
{
    if (sInitialized) {
        return true;  // Already initialized
    }

    try {
        std::lock_guard<std::mutex> lock(sLogMutex);

        // Store the log file path
        sLogFilePath = logFilePath;

        // Create log directory if it doesn't exist
        std::filesystem::path logPath(logFilePath);
        std::filesystem::path logDir = logPath.parent_path();
        if (!logDir.empty() && !std::filesystem::exists(logDir)) {
            std::filesystem::create_directories(logDir);
        }

        // Open the log file
        sLogFile.open(logFilePath, std::ios::out | std::ios::app);
        if (!sLogFile.is_open()) {
            std::cerr << "Failed to open log file: " << logFilePath << std::endl;
            return false;
        }

        sInitialized = true;

        // Log initialization message
        Logger initLogger("LogSystem");
        initLogger.info("Logging system initialized");

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Logger initialization failed: " << e.what() << std::endl;
        return false;
    }
}

//------------------------------------------------------------------------
void Logger::setGlobalLevel(Logger::Level newLevel)
{
    std::lock_guard<std::mutex> lock(sLogMutex);
    sGlobalLevel = newLevel;

    // Log level change if initialized
    if (sInitialized) {
        Logger levelLogger("LogSystem");
        levelLogger.info("Log level set to %s", levelToString(newLevel));
    }
}

//------------------------------------------------------------------------
void Logger::shutdown()
{
    std::lock_guard<std::mutex> lock(sLogMutex);

    if (sInitialized) {
        if (sLogFile.is_open()) {
            Logger shutdownLogger("LogSystem");
            shutdownLogger.info("Shutting down logging system");
            sLogFile.close();
        }
        sInitialized = false;
    }
}

//------------------------------------------------------------------------
void Logger::logMessage(Logger::Level level, const char* msg)
{
    if (level < sGlobalLevel) {
        return;  // Skip messages below the current log level
    }

    try {
        std::lock_guard<std::mutex> lock(sLogMutex);

        if (!sInitialized || !sLogFile.is_open()) {
            return;
        }

        // Get current time
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        // Format the log message
        std::tm tm_buf;
#ifdef _WIN32
        localtime_s(&tm_buf, &time);
#else
        localtime_r(&time, &tm_buf);
#endif

        sLogFile << "["
                 << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S") << "."
                 << std::setfill('0') << std::setw(3) << ms.count() << "] "
                 << "[" << mName << "] "
                 << "[" << levelToString(level) << "] "
                 << msg << std::endl;

        // Also print to console in debug builds
#ifdef DEBUG_BUILD
        std::cout << "["
                 << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S") << "."
                 << std::setfill('0') << std::setw(3) << ms.count() << "] "
                 << "[" << mName << "] "
                 << "[" << levelToString(level) << "] "
                 << msg << std::endl;
#endif
    } catch (const std::exception& e) {
        // Last resort fallback
        std::cerr << "Logging error: " << e.what() << std::endl;
    }
}

//------------------------------------------------------------------------
const char* Logger::levelToString(Logger::Level level)
{
    switch (level) {
        case Logger::Level::Trace:    return "TRACE";
        case Logger::Level::Debug:    return "DEBUG";
        case Logger::Level::Info:     return "INFO";
        case Logger::Level::Warning:  return "WARNING";
        case Logger::Level::Error:    return "ERROR";
        case Logger::Level::Critical: return "CRITICAL";
        case Logger::Level::Off:      return "OFF";
        default:                      return "UNKNOWN";
    }
}

} // namespace GranularPlunderphonics