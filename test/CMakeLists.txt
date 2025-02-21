# Test directory CMakeLists.txt
find_package(Catch2 REQUIRED)

set(TEST_SOURCES
        ProcessorTests.cpp
        ParameterTests.cpp
        TestMain.cpp
)

set(TEST_SOURCES
        ${TEST_SOURCES}
        ResourceManagerTests.cpp
)

add_executable(GranularPlunderphonicsTests ${TEST_SOURCES})

target_include_directories(GranularPlunderphonicsTests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${VST3_SDK_ROOT}
)

target_link_libraries(GranularPlunderphonicsTests
        PRIVATE
        sdk
        sdk_common
        spdlog::spdlog
        Catch2::Catch2
)

# Copy test data files to build directory if needed
# file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Add tests to CTest
include(Catch)
catch_discover_tests(GranularPlunderphonicsTests)