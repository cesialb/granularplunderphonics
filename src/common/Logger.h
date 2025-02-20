#pragma once

#include <spdlog/spdlog.h>
#include <string>
#include <memory>

namespace GranularPlunderphonics {

    class Logger {
    public:
        explicit Logger(const std::string& name);
        ~Logger() = default;

        static bool initialize(const std::string& logFilePath = "GranularPlunderphonics.log");
        static void shutdown();

        void trace(const char* msg) const;
        void debug(const char* msg) const;
        void info(const char* msg) const;
        void warn(const char* msg) const;
        void error(const char* msg) const;
        void critical(const char* msg) const;

    private:
        std::shared_ptr<spdlog::logger> mLogger;
    };

} // namespace GranularPlunderphonics
