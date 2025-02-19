cmake_minimum_required(VERSION 3.15)
project(GranularPlunderphonics VERSION 0.1.0)

# C++17 Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(VST3_OUTPUT_DIR ${CMAKE_BINARY_DIR}/VST3)

# Find dependencies
find_package(spdlog REQUIRED)

# VST3 SDK setup
# Option 1: If VST3 SDK is in the project as a submodule
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/vst3sdk/CMakeLists.txt)
    add_subdirectory(vst3sdk)
else()
    # Option 2: VST3 SDK is installed externally
    # You need to specify the path to VST3 SDK
    # set(VST3_SDK_ROOT "path/to/vst3sdk" CACHE PATH "Path to VST3 SDK")
    # include("${VST3_SDK_ROOT}/CMakeLists.txt")
    message(FATAL_ERROR "VST3 SDK not found. Please add it as a submodule or specify VST3_SDK_ROOT.")
endif()

# Include VST3 SDK setup (after adding the sdk subdirectory)
include(vst3sdk/cmake/modules/SMTG_VST3_SDK.cmake)

# Enable testing
enable_testing()

# Add subdirectories
add_subdirectory(src)
add_subdirectory(test)

# Installation rules
smtg_create_vst3_bundle(GranularPlunderphonics
        PACKAGE_NAME "GranularPlunderphonics"
        PACKAGE_VENDOR "YourCompany"
        PACKAGE_VERSION "${PROJECT_VERSION}"
        PACKAGE_EMAIL "your.email@example.com"
)

# Install VST3 plugin to standard locations
smtg_install_vst3plugin(GranularPlunderphonics
        DESTINATION ${VST3_OUTPUT_DIR}
)

# Add custom target for running tests
add_custom_target(run_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS GranularPlunderphonicsTests
        COMMENT "Running tests..."
)

# Print configuration summary
message(STATUS "Configuration Summary:")
message(STATUS "  Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  VST3 SDK: ${VST3_SDK_ROOT}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Output directory: ${VST3_OUTPUT_DIR}")