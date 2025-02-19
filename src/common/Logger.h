/**
 * @file Logger.h
 * @brief Logger class definition for the GranularPlunderphonics plugin
 */

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/fmt/ostr.h>
#include <memory>
#include <string>

namespace GranularPlunderphonics {

/**
 * @class Logger
 * @brief Wrapper around spdlog for consistent logging throughout the plugin
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
     * @param maxFileSize Maximum size of each log file in bytes
     * @param maxFiles Maximum number of rotating log files
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
     * @tparam Args Variadic template for format arguments
     * @param fmt Format string
     * @param args Format arguments
     */
    template<typename... Args>
    void trace(const char* fmt, const Args&... args) {
        if (mLogger) {
            mLogger->trace(fmt, args...);
        }
    }

    /**
     * @brief Log a debug message
     * @tparam Args Variadic template for format arguments
     * @param fmt Format string
     * @param args Format arguments
     */
    template<typename... Args>
    void debug(const char* fmt, const Args&... args) {
        if (mLogger) {
            mLogger->debug(fmt, args...);
        }
    }

    /**
     * @brief Log an info message
     * @tparam Args Variadic template for format arguments
     * @param fmt Format string
     * @param args Format arguments
     */
    template<typename... Args>
    void info(const char* fmt, const Args&... args) {
        if (mLogger) {
            mLogger->info(fmt, args...);
        }
    }

    /**
     * @brief Log a warning message
     * @tparam Args Variadic template for format arguments
     * @param fmt Format string
     * @param args Format arguments
     */
    template<typename... Args>
    void warn(const char* fmt, const Args&... args) {
        if (mLogger) {
            mLogger->warn(fmt, args...);
        }
    }

    /**
     * @brief Log an error message
     * @tparam Args Variadic template for format arguments
     * @param fmt Format string
     * @param args Format arguments
     */
    template<typename... Args>
    void error(const char* fmt, const Args&... args) {
        if (mLogger) {
            mLogger->error(fmt, args...);
        }
    }

    /**
     * @brief Log a critical message
     * @tparam Args Variadic template for format arguments
     * @param fmt Format string
     * @param args Format arguments
     */
    template<typename... Args>
    void critical(const char* fmt, const Args&... args) {
        if (mLogger) {
            mLogger->critical(fmt, args...);
        }
    }

private:
    std::shared_ptr<spdlog::logger> mLogger;             // Logger instance
    static std::shared_ptr<spdlog::logger> sMainLogger;  // Main logger for the application
    static bool sInitialized;                           // Flag to track initialization
};

} // namespace GranularPlunderphonics