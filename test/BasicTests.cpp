#include <catch2/catch_test_macros.hpp>

TEST_CASE("Basic Test - Environment Setup", "[basic]") {
    REQUIRE(true);  // Most basic test possible
    
    SECTION("Basic math operations") {
        REQUIRE(2 + 2 == 4);
        REQUIRE(10 - 5 == 5);
    }
}