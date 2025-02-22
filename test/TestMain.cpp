#include <catch2/catch_all.hpp>
#include "../src/common/Logger.h"
#include <string>
#include <sstream>
#include <iostream>

using namespace GranularPlunderphonics;

// Note: when using Catch2, this file can be mostly empty since Catch2 provides its own main
int main(int argc, char* argv[]) {
    try {
        // Initialize logging
        Logger::initialize("GranularPlunderphonicsTests.log");
        Logger logger("TestMain");
        logger.info("Starting test suite");

        // Let Catch2 handle test execution
        int result = Catch::Session().run(argc, argv);

        // Cleanup
        logger.info("Test suite completed");
        Logger::shutdown();

        return result;
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error in test framework: " << e.what() << std::endl;
        return 2;
    }
}