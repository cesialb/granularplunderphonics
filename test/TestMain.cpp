/**
 * @file TestMain.cpp
 * @brief Main entry point for the test suite (placeholder until Catch2 is available)
 */

#include <iostream>
#include <vector>
#include <functional>
#include <string>
#include <stdexcept>
#include "../src/common/Logger.h"

// Forward declarations of test functions
namespace GranularPlunderphonics {
    namespace Tests {
        // Each test file will declare its test functions here
        void runProcessorTests();
        void registerParameterTests();
    }
}

// Simple test framework until Catch2 is available
class TestFramework {
public:
    static TestFramework& getInstance() {
        static TestFramework instance;
        return instance;
    }

    void registerTest(const std::string& name, std::function<void()> testFunc) {
        mTests.push_back({name, testFunc});
    }

    bool runAllTests() {
        GranularPlunderphonics::Logger logger("TestFramework");
        logger.info("Running all tests...");

        int passed = 0;
        int failed = 0;

        for (const auto& test : mTests) {
            try {
                std::cout << "Running test: " << test.name << std::endl;
                logger.info("Starting test: {}", test.name);

                test.testFunc();

                logger.info("Test passed: {}", test.name);
                std::cout << "PASSED: " << test.name << std::endl;
                passed++;
            }
            catch (const std::exception& e) {
                logger.error("Test failed: {} - {}", test.name, e.what());
                std::cerr << "FAILED: " << test.name << " - " << e.what() << std::endl;
                failed++;
            }
        }

        std::cout << "\nTest summary: "
                  << passed << " passed, "
                  << failed << " failed, "
                  << (passed + failed) << " total" << std::endl;

        logger.info("Test summary: {} passed, {} failed, {} total",
                   passed, failed, passed + failed);

        return failed == 0;
    }

private:
    struct TestCase {
        std::string name;
        std::function<void()> testFunc;
    };

    std::vector<TestCase> mTests;

    // Private constructor for singleton
    TestFramework() {}
};

// Macro to simplify test registration
#define REGISTER_TEST(name, func) \
    static bool registered_##func = (TestFramework::getInstance().registerTest(name, func), true)

// Helper function for test registration
void registerTest(const std::string& name, std::function<void()> testFunc) {
    TestFramework::getInstance().registerTest(name, testFunc);
}

// Main entry point
int main(int argc, char* argv[]) {
    try {
        // Initialize logging
        GranularPlunderphonics::Logger::initialize("GranularPlunderphonicsTests.log");
        GranularPlunderphonics::Logger logger("TestMain");
        logger.info("Starting test suite");

        // Register all tests (will be replaced by Catch2 auto-discovery)
        GranularPlunderphonics::Tests::runProcessorTests();
        GranularPlunderphonics::Tests::registerParameterTests();

        // Run all tests
        bool allPassed = TestFramework::getInstance().runAllTests();

        // Cleanup
        logger.info("Test suite completed");
        GranularPlunderphonics::Logger::shutdown();

        return allPassed ? 0 : 1;
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error in test framework: " << e.what() << std::endl;
        return 2;
    }
}