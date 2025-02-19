/**
 * @file Logger.cpp
 * @brief Implementation of the Logger class
 */

#include "Logger.h"
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <filesystem>

namespace GranularPlunderphonics {

// Static member initialization
std::shared_ptr<spdlog::logger> Logger::sMainLogger = nullptr;
bool Logger::sInitialized = false;

//------------------------------------------------------------------------
Logger::Logger(const std::string& name)
{
    if (!sInitialized) {
        // Try to initialize with default settings if not already initialized
        initialize();
    }

    // Get or create a logger with the given name
    mLogger = spdlog::get(name);
    if (!mLogger) {
        if (sMainLogger) {
            // Use the same sinks as the main logger
            mLogger = std::make_shared<spdlog::logger>(name, sMainLogger->sinks().begin(), sMainLogger->sinks().end());
            spdlog::register_logger(mLogger);
        } else {
            // Fallback to console-only logger if main logger not available
            mLogger = spdlog::stdout_color_mt(name);
        }
    }
}

//------------------------------------------------------------------------
bool Logger::initialize(const std::string& logFilePath, size_t maxFileSize, size_t maxFiles)
{
    if (sInitialized) {
        return true;  // Already initialized
    }

    try {
        // Create log directory if it doesn't exist
        std::filesystem::path logPath(logFilePath);
        std::filesystem::path logDir = logPath.parent_path();
        if (!logDir.empty() && !std::filesystem::exists(logDir)) {
            std::filesystem::create_directories(logDir);
        }

        // Initialize async logging
        spdlog::init_thread_pool(8192, 1);  // Queue size, thread count

        // Create sinks
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            logFilePath, maxFileSize, maxFiles);

        // Set log format
        std::string pattern = "[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v";
        console_sink->set_pattern(pattern);
        file_sink->set_pattern(pattern);

        // Create and register main logger with multiple sinks
        sMainLogger = std::make_shared<spdlog::logger>(
            "GranularPlunderphonics", spdlog::sinks_init_list({console_sink, file_sink}));

        // Set default log level
        sMainLogger->set_level(spdlog::level::debug);

        // Register as default logger
        spdlog::register_logger(sMainLogger);
        spdlog::set_default_logger(sMainLogger);

        // Enable async mode
        spdlog::set_async_mode(8192);

        sInitialized = true;
        sMainLogger->info("Logging system initialized");
        return true;
    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Logger initialization failed: " << ex.what() << std::endl;
        return false;
    }
}

//------------------------------------------------------------------------
void Logger::setGlobalLevel(Level level)
{
    spdlog::level::level_enum spdlogLevel;

    switch (level) {
        case Level::Trace:    spdlogLevel = spdlog::level::trace; break;
        case Level::Debug:    spdlogLevel = spdlog::level::debug; break;
        case Level::Info:     spdlogLevel = spdlog::level::info; break;
        case Level::Warning:  spdlogLevel = spdlog::level::warn; break;
        case Level::Error:    spdlogLevel = spdlog::level::err; break;
        case Level::Critical: spdlogLevel = spdlog::level::critical; break;
        case Level::Off:      spdlogLevel = spdlog::level::off; break;
        default:              spdlogLevel = spdlog::level::info;
    }

    spdlog::set_level(spdlogLevel);

    if (sMainLogger) {
        sMainLogger->info("Log level set to {}", spdlog::level::to_string_view(spdlogLevel));
    }
}

//------------------------------------------------------------------------
void Logger::shutdown()
{
    if (sInitialized) {
        if (sMainLogger) {
            sMainLogger->info("Shutting down logging system");
        }
        spdlog::shutdown();
        sInitialized = false;
    }
}

} // namespace GranularPlunderphonics