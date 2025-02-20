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
set(VST3_SDK_ROOT "/Users/raydiator/CLionProjects/GranularPlunderphonics/vst3sdk")
if(NOT EXISTS "${VST3_SDK_ROOT}/CMakeLists.txt")
    message(FATAL_ERROR "VST3 SDK not found at ${VST3_SDK_ROOT}")
endif()

# Add VST3 SDK cmake modules path
list(APPEND CMAKE_MODULE_PATH
        "${VST3_SDK_ROOT}/cmake/modules"
)

# Include VST3 SDK modules
include(SMTG_Global)
include(SMTG_AddVST3Library)
include(SMTG_Bundle)
include(SMTG_ExportedSymbols)
include(SMTG_PrefixHeader)
include(SMTG_PlatformIOS)
include(SMTG_PlatformToolset)
include(SMTG_CoreAudioSupport)
include(SMTG_AAXSupport)
include(SMTG_VstGuiSupport)
include(SMTG_UniversalBinary)
include(SMTG_AddVST3Options)

# Add VST3 SDK as subdirectory
add_subdirectory(${VST3_SDK_ROOT})

# Enable testing
enable_testing()

# Add subdirectories
add_subdirectory(src)
add_subdirectory(test)

# Print configuration summary
message(STATUS "Configuration Summary:")
message(STATUS "  Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  VST3 SDK: ${VST3_SDK_ROOT}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Output directory: ${VST3_OUTPUT_DIR}")
