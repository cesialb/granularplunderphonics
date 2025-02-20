# Granular Plunderphonics VST3 Plugin

A modern C++ VST3 plugin for granular sound processing and audio manipulation inspired by plunderphonics techniques.
## Overview

This VST3 plugin provides a framework for granular audio processing, specifically tailored for creative plunderphonics techniques. The current implementation provides a minimal mono-to-stereo pass-through with the architecture ready for more complex granular processing features.

## Features

- Modern C++17 implementation
- VST3 SDK integration
- Mono/Stereo input to stereo output configuration
- Comprehensive parameter management system with:
   - Float, Boolean, Integer, and Enum parameter types
   - Parameter smoothing and interpolation
   - Thread-safe parameter access
   - Parameter persistence (save/load state)
- Comprehensive logging system with spdlog
- Robust error handling with custom exception types
- Unit testing with Catch2

## Prerequisites

- C++17 compatible compiler
- CMake 3.15 or higher
- VST3 SDK (included as submodule)
- Catch2 (for testing)

### External Dependencies

1. **spdlog** - A fast C++ logging library
   - Required for logging functionality
   - Install using your package manager or from [https://github.com/gabime/spdlog](https://github.com/gabime/spdlog)

2. **Catch2** - A modern C++ test framework
   - Required for unit testing
   - Install using your package manager or from [https://github.com/catchorg/Catch2](https://github.com/catchorg/Catch2)

### VST3 SDK Installation

The VST3 SDK is included as a git submodule. After cloning the repository, initialize it with:

```bash
git submodule update --init --recursive

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

# Initialize and update submodules
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

- `src/`
   - `plugin/` - Core plugin implementation
      - Parameter management
      - Audio processing
      - VST3 integration
   - `common/` - Shared utilities
      - Logging system
      - Error handling
- `test/` - Unit tests
   - Parameter tests
   - Processor tests

## Development Roadmap

1. ✅ Basic plugin architecture
2. ✅ Audio pass-through functionality
3. ✅ Logging system
4. ✅ Error handling
5. ✅ Unit tests
6. ✅ Parameter management system
7. ⬜ Audio file management system
8. ⬜ Basic granular processing engine
9. ⬜ Advanced plunderphonics algorithms:
   - Time stretching/compression
   - Pitch shifting
   - Sample chopping/rearrangement
   - Layering and mixing
   - Real-time manipulation
10. ⬜ Custom UI
11. ⬜ Presets system

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Steinberg for the VST3 SDK
- spdlog developers
- Catch2 team