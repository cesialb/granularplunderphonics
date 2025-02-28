#pragma once

#include "PluginProcessor.h"

/**
 * GranularPlunderphonicsAudioProcessorEditor - GUI editor for the Granular Plunderphonics VST3 plugin
 * Provides a minimal interface for the plugin - will be expanded in future iterations
 */
class GranularPlunderphonicsAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    GranularPlunderphonicsAudioProcessorEditor(GranularPlunderphonicsAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~GranularPlunderphonicsAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    GranularPlunderphonicsAudioProcessor& audioProcessor;
    
    // UI Components
    juce::Slider gainSlider;
    juce::Label gainLabel;
    
    // Parameter attachment
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GranularPlunderphonicsAudioProcessorEditor)
};