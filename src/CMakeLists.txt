# Find required packages
find_package(SndFile REQUIRED)
find_package(PkgConfig REQUIRED)

# Create the core library
add_library(GranularPlunderphonicsCore STATIC
        audio/AudioBuffer.cpp
        audio/AudioFile.cpp
        audio/FileFormat.cpp
        audio/GrainCloud.cpp
        audio/GrainEngine.cpp
        audio/GrainGenerator.cpp
        audio/GrainProcessor.cpp
        audio/Resampler.cpp
        audio/StreamingManager.cpp
        common/ErrorHandling.cpp
        common/Logger.cpp
        common/ResourceManager.cpp
)

target_include_directories(GranularPlunderphonicsCore
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${VST3_SDK_ROOT}
        "${FFTW_ROOT}/include"
)

target_link_directories(GranularPlunderphonicsCore
        PUBLIC
        "${FFTW_ROOT}/lib"
)

target_link_libraries(GranularPlunderphonicsCore
        PUBLIC
        spdlog::spdlog
        SndFile::sndfile
        "${FFTW_ROOT}/lib/libfftw3f.dylib"
)

# Create the VST3 plugin as a module
add_library(GranularPlunderphonics MODULE
        plugin/GranularPlunderphonicsController.cpp
        plugin/GranularPlunderphonicsEntry.cpp
        plugin/GranularPlunderphonicsProcessor.cpp
        plugin/ParameterManager.cpp
)

# Add framework dependencies for macOS
if(APPLE)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    target_link_libraries(GranularPlunderphonics
            PRIVATE
            GranularPlunderphonicsCore
            sdk
            base
            pluginterfaces
            "-framework CoreFoundation"
    )
else()
    target_link_libraries(GranularPlunderphonics
            PRIVATE
            GranularPlunderphonicsCore
            sdk
            base
            pluginterfaces
    )
endif()

# Set the output name and properties for the VST3 plugin
set_target_properties(GranularPlunderphonics PROPERTIES
        PREFIX ""
        SUFFIX ".vst3"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/VST3"
)
