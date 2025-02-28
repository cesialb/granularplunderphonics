cmake_minimum_required(VERSION 3.18)

project(GranularPlunderphonics VERSION 0.1.0)

# Configure C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "macOS architectures to build for")

# Add JUCE as a submodule or via FetchContent
option(JUCE_SUBMODULE "Use JUCE as a submodule" OFF)

if(JUCE_SUBMODULE)
    add_subdirectory(ThirdParty/JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(
            JUCE
            GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
            GIT_TAG 7.0.2
    )
    FetchContent_MakeAvailable(JUCE)
endif()

# Enable VST3 format
option(JUCE_BUILD_VST3 "Build VST3 plugin" ON)
option(JUCE_BUILD_STANDALONE "Build standalone plugin" ON)
option(BUILD_TESTING "Build the testing executable" ON)

# JUCE-specific settings
set_directory_properties(PROPERTIES JUCE_COMPANY_NAME "YourCompany")
set_directory_properties(PROPERTIES JUCE_COMPANY_WEBSITE "www.yourcompany.com")
set_directory_properties(PROPERTIES JUCE_COMPANY_EMAIL "info@yourcompany.com")

# Include JUCE modules
include(${JUCE_ROOT}/extras/Build/CMake/JUCEModuleSupport.cmake)

# Add our source directory
add_subdirectory(Source)

# Add test directory if testing is enabled
if(BUILD_TESTING)
    add_subdirectory(Tests)
endif()