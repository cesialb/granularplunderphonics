cmake_minimum_required(VERSION 3.15)
project(GranularPlunderphonics VERSION 0.1.0)

# Add Homebrew paths explicitly
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/Cellar/fftw/3.3.10_2")
set(FFTW_ROOT "/opt/homebrew/Cellar/fftw/3.3.10_2")

# C++17 Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Define DEVELOPMENT for Debug builds and RELEASE for Release builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEVELOPMENT=1 -D_DEBUG=1)
else()
    add_definitions(-DRELEASE=1 -DNDEBUG=1)
endif()

# Set VST3 SDK path
set(VST3_SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vst3sdk" CACHE PATH "Path to VST3 SDK")

# Find dependencies
find_package(spdlog REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFTW REQUIRED fftw3f)

# Add explicit link directories
link_directories(${FFTW_ROOT}/lib)

# VST3 SDK setup
if(EXISTS ${VST3_SDK_ROOT}/CMakeLists.txt)
    add_subdirectory(${VST3_SDK_ROOT})
else()
    message(FATAL_ERROR "VST3 SDK not found. Please add it as a submodule or specify VST3_SDK_ROOT.")
endif()

# Enable testing
enable_testing()

# Add subdirectories
add_subdirectory(src)
add_subdirectory(test)
