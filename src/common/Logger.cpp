#include "Logger.h"
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <iostream>

namespace GranularPlunderphonics {

    Logger::Logger(const std::string& name) {
        mLogger = spdlog::get(name);
        if (!mLogger) {
            mLogger = spdlog::basic_logger_mt(name, "GranularPlunderphonics.log");
        }
    }

    bool Logger::initialize(const std::string& logFilePath) {
        try {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath);

            spdlog::set_default_logger(
                std::make_shared<spdlog::logger>(
                    "default",
                    spdlog::sinks_init_list({console_sink, file_sink})
                )
            );

            spdlog::set_level(spdlog::level::debug);
            spdlog::flush_on(spdlog::level::debug);

            return true;
        } catch (const spdlog::spdlog_ex& ex) {
            std::cerr << "Logger initialization failed: " << ex.what() << std::endl;
            return false;
        }
    }

    void Logger::shutdown() {
        spdlog::shutdown();
    }

    // Updated implementations to accept std::string directly
    void Logger::trace(const std::string& msg) const { mLogger->trace(msg); }
    void Logger::debug(const std::string& msg) const { mLogger->debug(msg); }
    void Logger::info(const std::string& msg) const { mLogger->info(msg); }
    void Logger::warn(const std::string& msg) const { mLogger->warn(msg); }
    void Logger::error(const std::string& msg) const { mLogger->error(msg); }
    void Logger::critical(const std::string& msg) const { mLogger->critical(msg); }

} // namespace GranularPlunderphonics