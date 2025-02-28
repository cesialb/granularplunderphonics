# Create a test executable
add_executable(GranularPlunderphonicsTests
        AudioProcessorTests.cpp
        ParameterTests.cpp
)

# Include necessary directories
target_include_directories(GranularPlunderphonicsTests
        PRIVATE
        ../Source
)

# Link against JUCE modules and the plugin
target_link_libraries(GranularPlunderphonicsTests
        PRIVATE
        GranularPlunderphonics
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
)

# Add the test to CTest
include(CTest)
add_test(NAME GranularPlunderphonicsTests COMMAND GranularPlunderphonicsTests)