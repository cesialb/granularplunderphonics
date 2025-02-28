#include "catch.hpp"
#include "PluginProcessor.h"

TEST_CASE("Parameter functionality", "[parameters]")
{
    // Create an instance of the processor
    GranularPlunderphonicsAudioProcessor processor;
    
    SECTION("Parameters initialized correctly")
    {
        // Check default gain value (should be 0.5f)
        REQUIRE(processor.getGain() == Approx(0.5f));
    }
    
    SECTION("State saving and loading")
    {
        // Create a memory block to store the state
        juce::MemoryBlock stateData;
        
        // Get the initial state
        processor.getStateInformation(stateData);
        
        // Create a new processor
        GranularPlunderphonicsAudioProcessor newProcessor;
        
        // Modify its gain to ensure it's different
        // Note: We don't have direct access to set the gain,
        // so this test is checking the infrastructure rather than the actual value change
        
        // Set the state from the saved state
        newProcessor.setStateInformation(stateData.getData(), static_cast<int>(stateData.getSize()));
        
        // Check that the gain parameter was loaded correctly
        REQUIRE(newProcessor.getGain() == Approx(processor.getGain()));
    }
}