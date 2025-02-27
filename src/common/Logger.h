#pragma once

#include <spdlog/spdlog.h>
#include <string>

namespace GranularPlunderphonics {

    class Logger {
    public:
        explicit Logger(const std::string& name);
        ~Logger() = default;

        static bool initialize(const std::string& logFilePath = "GranularPlunderphonics.log");
        static void shutdown();

        // Updated methods to accept std::string directly
        void trace(const std::string& msg) const;
        void debug(const std::string& msg) const;
        void info(const std::string& msg) const;
        void warn(const std::string& msg) const;
        void error(const std::string& msg) const;
        void critical(const std::string& msg) const;

    private:
        std::shared_ptr<spdlog::logger> mLogger;
    };

} // namespace GranularPlunderphonics