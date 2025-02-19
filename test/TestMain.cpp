/**
* @file TestMain.cpp
 * @brief Main entry point for the test suite using Catch2
 */

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main()
#include <catch2/catch.hpp>

// Initialize the plugin logging system for tests
#include "common/Logger.h"

struct TestEnvironment {
    TestEnvironment() {
        // Initialize logger with test-specific file
        GranularPlunderphonics::Logger::initialize("GranularPlunderphonicsTests.log");
        GranularPlunderphonics::Logger testLogger("TestRunner");
        testLogger.info("Starting test suite");
    }

    ~TestEnvironment() {
        GranularPlunderphonics::Logger testLogger("TestRunner");
        testLogger.info("Test suite completed");
        GranularPlunderphonics::Logger::shutdown();
    }
};

// Create a global test environment
TestEnvironment testEnvironment;