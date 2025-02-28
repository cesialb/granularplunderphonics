#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

/**
 * GranularPlunderphonicsAudioProcessor - Main audio processor class for the Granular Plunderphonics VST3 plugin
 * Currently implements a simple pass-through processor with mono->stereo capability
 */
class GranularPlunderphonicsAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    GranularPlunderphonicsAudioProcessor();
    ~GranularPlunderphonicsAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    // Parameter management - just a gain parameter for demonstration
    float getGain() const { return *gainParameter; }

private:
    //==============================================================================
    // Parameters
    juce::AudioParameterFloat* gainParameter = nullptr;

    // Parameter handling
    juce::AudioProcessorValueTreeState parameters;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GranularPlunderphonicsAudioProcessor)
};