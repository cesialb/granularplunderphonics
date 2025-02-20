/**
 * @file Logger.h
 * @brief Logger class definition for the GranularPlunderphonics plugin
 */

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <fstream>
#include <mutex>

namespace GranularPlunderphonics {

/**
 * @class Logger
 * @brief Simple logging implementation that will be replaced with spdlog when available
 *
 * This class provides a simple interface for logging with different levels
 * and ensures consistent configuration across all plugin components.
 */
class Logger {
public:
    /**
     * @brief Supported log levels
     */
    enum class Level {
        Trace,
        Debug,
        Info,
        Warning,
        Error,
        Critical,
        Off
    };

    /**
     * @brief Construct a new Logger with the specified name
     * @param name The logger name, typically the class or component name
     */
    explicit Logger(const std::string& name);

    /**
     * @brief Destructor
     */
    ~Logger() = default;

    /**
     * @brief Set the global logging level
     * @param level The desired logging level
     */
    static void setGlobalLevel(Level level);

    /**
     * @brief Initialize the logging system
     * @param logFilePath Path to the log file
     * @param maxFileSize Maximum size of each log file in bytes (not used in this implementation)
     * @param maxFiles Maximum number of rotating log files (not used in this implementation)
     * @return true if initialization succeeded, false otherwise
     */
    static bool initialize(
        const std::string& logFilePath = "GranularPlunderphonics.log",
        size_t maxFileSize = 5 * 1024 * 1024,  // 5 MB
        size_t maxFiles = 3);

    /**
     * @brief Shutdown the logging system
     */
    static void shutdown();

    /**
     * @brief Log a trace message
     * @param msg The message to log
     * @param args Format arguments (not used in this implementation)
     */
    template<typename... Args>
    void trace(const char* msg, const Args&... args) const {
        logMessage(Level::Trace, msg);
    }

    /**
     * @brief Log a debug message
     * @param msg The message to log
     * @param args Format arguments (not used in this implementation)
     */
    template<typename... Args>
    void debug(const char* msg, const Args&... args) const {
        logMessage(Level::Debug, msg);
    }

    /**
     * @brief Log an info message
     * @param msg The message to log
     * @param args Format arguments (not used in this implementation)
     */
    template<typename... Args>
    void info(const char* msg, const Args&... args) const {
        logMessage(Level::Info, msg);
    }

    /**
     * @brief Log a warning message
     * @param msg The message to log
     * @param args Format arguments (not used in this implementation)
     */
    template<typename... Args>
    void warn(const char* msg, const Args&... args) const {
        logMessage(Level::Warning, msg);
    }

    /**
     * @brief Log an error message
     * @param msg The message to log
     * @param args Format arguments (not used in this implementation)
     */
    template<typename... Args>
    void error(const char* msg, const Args&... args) const {
        logMessage(Level::Error, msg);
    }

    /**
     * @brief Log a critical message
     * @param msg The message to log
     * @param args Format arguments (not used in this implementation)
     */
    template<typename... Args>
    void critical(const char* msg, const Args&... args) const {
        logMessage(Level::Critical, msg);
    }

    // Non-template overloads for simple string messages
    void info(const char* msg) const {
        logMessage(Level::Info, msg);
    }

    void error(const char* msg) const {
        logMessage(Level::Error, msg);
    }

private:
    /**
     * @brief Log a message with the specified level
     * @param level The log level
     * @param msg The message to log
     */
    void logMessage(Level level, const char* msg) const;

    /**
     * @brief Convert a log level to a string
     * @param level The log level
     * @return String representation of the log level
     */
    static const char* levelToString(Level level);

    std::string mName;                      // Logger name
    static Level sGlobalLevel;              // Global log level
    static std::string sLogFilePath;        // Path to log file
    static std::ofstream sLogFile;          // Log file stream
    static bool sInitialized;               // Initialization flag
    static std::mutex sLogMutex;            // Mutex for thread safety
};

} // namespace GranularPlunderphonics
