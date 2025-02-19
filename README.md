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
- VST3 SDK
- spdlog library
- Catch2 (for testing)

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