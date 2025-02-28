# Granular Plunderphonics VST3

A minimal VST3 plugin framework using JUCE for Granular Plunderphonics processing.

## Description

This project provides a foundational VST3 plugin structure that currently implements a simple audio pass-through (mono to stereo) with gain control. It's designed to be a starting point for developing a full-featured granular synthesis plugin focused on plunderphonics techniques.

## Features (Current Implementation)

- VST3 plugin format support
- Mono to stereo audio routing
- Basic gain parameter
- Clean project structure using CMake
- Unit tests using Catch2
- macOS Apple Silicon support

## Requirements

- CMake 3.18 or higher
- C++17 compatible compiler
- JUCE 7.0.2 or higher

## Building for macOS (Apple Silicon)

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/GranularPlunderphonics.git
   cd GranularPlunderphonics
   ```

2. Configure with CMake:
   ```bash
   mkdir build
   cd build
   cmake ..
   ```

3. Build the plugin:
   ```bash
   cmake --build .
   ```

4. The built VST3 plugin will be in `build/Source/GranularPlunderphonics_artefacts/VST3/`.

## Running the Tests

Tests are built automatically when the `BUILD_TESTING` option is enabled (default):

```bash
cd build
ctest
```

## Project Structure

- `Source/` - Contains the plugin source code
  -