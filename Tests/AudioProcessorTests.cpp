#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "PluginProcessor.h"

TEST_CASE("Plugin initialization", "[processor]")
{
    // Create an instance of the processor
    GranularPlunderphonicsAudioProcessor processor;
    
    SECTION("Plugin name is correct")
    {
        REQUIRE(processor.getName() == "Granular Plunderphonics");
    }
    
    SECTION("MIDI capabilities are correctly reported")
    {
        REQUIRE_FALSE(processor.acceptsMidi());
        REQUIRE_FALSE(processor.producesMidi());
        REQUIRE_FALSE(processor.isMidiEffect());
    }
    
    SECTION("Channel configuration is correctly reported")
    {
        // Test mono input to stereo output configuration
        juce::AudioProcessor::BusesLayout layout;
        layout.inputBuses.add(juce::AudioChannelSet::mono());
        layout.outputBuses.add(juce::AudioChannelSet::stereo());
        
        REQUIRE(processor.isBusesLayoutSupported(layout));
        
        // Test stereo input to stereo output (should not be supported)
        layout.inputBuses.clear();
        layout.inputBuses.add(juce::AudioChannelSet::stereo());
        
        REQUIRE_FALSE(processor.isBusesLayoutSupported(layout));
    }
}

TEST_CASE("Audio processing functionality", "[processor]")
{
    // Create an instance of the processor
    GranularPlunderphonicsAudioProcessor processor;
    
    SECTION("Audio passes through preserving signal integrity")
    {
        // Prepare a simple mono audio buffer with a sine wave
        constexpr int numSamples = 512;
        constexpr float frequency = 440.0f; // A4 note
        constexpr float sampleRate = 44100.0f;
        
        juce::AudioBuffer<float> inputBuffer(1, numSamples);
        auto* monoChannel = inputBuffer.getWritePointer(0);
        
        // Fill with sine wave
        for (int i = 0; i < numSamples; ++i)
        {
            monoChannel[i] = std::sin(2.0f * juce::MathConstants<float>::pi * frequency * i / sampleRate);
        }
        
        // Create a copy for comparison
        juce::AudioBuffer<float> originalBuffer(inputBuffer);
        
        // Resize the buffer to stereo output
        inputBuffer.setSize(2, numSamples, true, true, true);
        
        // Process the buffer
        juce::MidiBuffer midiBuffer;
        processor.prepareToPlay(sampleRate, numSamples);
        processor.processBlock(inputBuffer, midiBuffer);
        
        // Check that the audio has been processed correctly
        const auto* leftChannel = inputBuffer.getReadPointer(0);
        const auto* rightChannel = inputBuffer.getReadPointer(1);
        const auto* originalChannel = originalBuffer.getReadPointer(0);
        
        // With default gain of 0.5, the output should be the original signal multiplied by 0.5
        float expectedGain = 0.5f;
        
        // Check both channels match the expected processed signal
        bool signalIntegrityMaintained = true;
        for (int i = 0; i < numSamples; ++i)
        {
            float expectedSample = originalChannel[i] * expectedGain;
            float tolerance = 1e-6f; // Floating point tolerance
            
            if (std::abs(leftChannel[i] - expectedSample) > tolerance ||
                std::abs(rightChannel[i] - expectedSample) > tolerance)
            {
                signalIntegrityMaintained = false;
                break;
            }
        }
        
        REQUIRE(signalIntegrityMaintained);
    }
}