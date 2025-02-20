# Granular Plunderphonics VST3 Plugin

A modern C++ VST3 plugin for granular sound processing and audio manipulation inspired by plunderphonics techniques.

## Overview

This VST3 plugin provides a framework for granular audio processing, specifically tailored for creative plunderphonics techniques. The current implementation provides a minimal mono-to-stereo pass-through with the architecture ready for more complex granular processing features.

## Features

- Modern C++17 implementation
- VST3 SDK integration
- Mono input to stereo output configuration
- Comprehensive logging system with spdlog
- Robust error handling
- Unit testing with Catch2

## Prerequisites

- C++17 compatible compiler
- CMake 3.15 or higher
- VST3 SDK (must be installed/downloaded separately)
- Catch2 (for testing)

### External Dependencies

The project is designed to work with minimal external dependencies initially. When you're ready for the full feature set:

1. **spdlog** - A fast C++ logging library
    - The project includes a simple logging implementation that will be replaced with spdlog
    - Install using your package manager or from [https://github.com/gabime/spdlog](https://github.com/gabime/spdlog)

2. **Catch2** - A modern C++ test framework
    - Install using your package manager or from [https://github.com/catchorg/Catch2](https://github.com/catchorg/Catch2)

### VST3 SDK Installation

Before building this project, you'll need to download the VST3 SDK:

1. Download the latest VST3 SDK from [Steinberg's Developer Portal](https://developer.steinberg.help/display/VST/VST+3+SDK)
2. Extract it to a suitable location on your machine
3. When configuring the project, set the `VST3_SDK_ROOT` CMake variable to point to your VST3 SDK location

```bash
# Example: Setting the VST3 SDK path during CMake configuration
cmake .. -DVST3_SDK_ROOT=/path/to/vst3sdk
```

## Building with CLion

This project is configured to work well with CLion. To build:

1. Open the project in CLion
2. Configure the VST3 SDK path
3. Select the build configuration (Debug/Release)
4. Build the project using the Build menu

## Manual Build

```bash
# Clone the repository
git clone https://github.com/yourusername/GranularPlunderphonics.git
cd GranularPlunderphonics

# Initialize and update submodules if VST3 SDK is added as a submodule
git submodule update --init --recursive

# Create build directory
mkdir build
cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release

# Run tests
ctest -C Release
```

## Project Structure

- `src/plugin/` - Core plugin implementation
- `src/common/` - Shared utilities (logging, error handling)
- `test/` - Unit tests

## Development Roadmap

1. ✅ Basic plugin architecture
2. ✅ Audio pass-through functionality
3. ✅ Logging system
4. ✅ Error handling
5. ✅ Unit tests
6. ⬜ Granular processing engine
7. ⬜ Parameter system for granular controls
8. ⬜ Custom UI
9. ⬜ Presets system
10. ⬜ Advanced plunderphonics algorithms

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Steinberg for the VST3 SDK
- spdlog developers
- Catch2 team