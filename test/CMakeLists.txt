# test/CMakeLists.txt

# Find Catch2
find_package(Catch2 3 REQUIRED)

# Find other dependencies
find_package(SndFile REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFTW REQUIRED fftw3f)

# Find JUCE for UI tests
find_package(JUCE REQUIRED)

add_executable(GranularTests
        TestMain.cpp
        BasicTests.cpp
        AudioFileTests.cpp
        GrainCloudTests.cpp
        GrainGeneratorTests.cpp
        GrainProcessorTests.cpp
        LorenzAttractorTests.cpp
        ModulationIntegrationTests.cpp
        ModulationMatrixTests.cpp
        ParameterTests.cpp
        ProcessorTests.cpp
        ResourceManagerTests.cpp
        UITests.cpp  # New UI tests
        # Add the processor source files directly to the test executable
        ${CMAKE_SOURCE_DIR}/src/plugin/GranularPlunderphonicsProcessor.cpp
        ${CMAKE_SOURCE_DIR}/src/plugin/GranularPlunderphonicsController.cpp
        ${CMAKE_SOURCE_DIR}/src/GranularPlunderphonicsEditor.cpp
        ${CMAKE_SOURCE_DIR}/src/LookAndFeel/GranularLookAndFeel.cpp
        ${CMAKE_SOURCE_DIR}/src/Components/HeaderPanel.cpp
        ${CMAKE_SOURCE_DIR}/src/Components/MainPanel.cpp
)

# Add the TESTING definition to enable test-specific code paths
target_compile_definitions(GranularTests PRIVATE TESTING)

target_include_directories(GranularTests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${VST3_SDK_ROOT}
        ${FFTW_ROOT}/include
        ${SAMPLERATE_INCLUDE_DIRS}
)

target_link_directories(GranularTests
        PRIVATE
        ${FFTW_ROOT}/lib
        ${SAMPLERATE_ROOT}/lib
)

if(APPLE)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    target_link_libraries(GranularTests
            PRIVATE
            Catch2::Catch2WithMain
            GranularPlunderphonicsCore
            sdk
            base
            pluginterfaces
            SndFile::sndfile
            "${FFTW_ROOT}/lib/libfftw3f.dylib"
            "${SAMPLERATE_ROOT}/lib/libsamplerate.dylib"
            juce::juce_gui_basics
            juce::juce_gui_extra
            juce::juce_audio_basics
            juce::juce_audio_processors
            juce::juce_dsp
            "-framework CoreFoundation"
    )
else()
    target_link_libraries(GranularTests
            PRIVATE
            Catch2::Catch2WithMain
            GranularPlunderphonicsCore
            sdk
            base
            pluginterfaces
            SndFile::sndfile
            "${FFTW_ROOT}/lib/libfftw3f.dylib"
            "${SAMPLERATE_ROOT}/lib/libsamplerate.dylib"
            juce::juce_gui_basics
            juce::juce_gui_extra
            juce::juce_audio_basics
            juce::juce_audio_processors
            juce::juce_dsp
    )
endif()

include(CTest)
include(Catch)
catch_discover_tests(GranularTests)