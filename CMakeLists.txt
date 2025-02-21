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

# Define DEVELOPMENT for Debug builds and RELEASE for Release builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEVELOPMENT=1 -D_DEBUG=1)
else()
    add_definitions(-DRELEASE=1 -DNDEBUG=1)
endif()

# Find dependencies
find_package(spdlog REQUIRED)

# VST3 SDK setup
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/vst3sdk/CMakeLists.txt)
    add_subdirectory(vst3sdk)
else()
    message(FATAL_ERROR "VST3 SDK not found. Please add it as a submodule or specify VST3_SDK_ROOT.")
endif()

# Enable testing
enable_testing()

# Add subdirectories
add_subdirectory(src)
add_subdirectory(test)

# In your root CMakeLists.txt or the relevant subdirectory

# In your root CMakeLists.txt

# Find FFTW3
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFTW3 REQUIRED fftw3f)

# Add to your target
target_include_directories(YourTarget
        PRIVATE
        ${FFTW3_INCLUDE_DIRS}
)

target_link_libraries(YourTarget
        PRIVATE
        ${FFTW3_LIBRARIES}
)

