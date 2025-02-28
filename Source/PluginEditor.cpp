#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GranularPlunderphonicsAudioProcessorEditor::GranularPlunderphonicsAudioProcessorEditor(
    GranularPlunderphonicsAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Set up gain slider
    gainSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 20);
    gainSlider.setRange(0.0, 1.0, 0.01);
    addAndMakeVisible(gainSlider);

    // Set up gain label
    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(gainLabel);

    // Create parameter attachment
    gainAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        vts, "gain", gainSlider));

    // Make sure that before the constructor returns, you've set the
    // editor's size to whatever you need it to be.
    setSize(400, 300);
}

GranularPlunderphonicsAudioProcessorEditor::~GranularPlunderphonicsAudioProcessorEditor()
{
    // Clean up resources
    gainAttachment = nullptr;
}

//==============================================================================
void GranularPlunderphonicsAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Fill background
    g.fillAll(juce::Colours::darkgrey);
    
    // Set font
    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
    
    // Draw title
    g.drawFittedText("Granular Plunderphonics", getLocalBounds(), juce::Justification::centredTop, 1);
    
    // Draw version info
    g.setFont(12.0f);
    g.drawFittedText("v0.1.0 - Audio Pass-through", 
                     getLocalBounds().removeFromBottom(20),
                     juce::Justification::centredBottom, 1);
}

void GranularPlunderphonicsAudioProcessorEditor::resized()
{
    // Position the slider and label
    auto area = getLocalBounds();
    auto topSection = area.removeFromTop(40); // Space for title
    
    // Center the gain control
    auto sliderArea = area.reduced(50).removeFromTop(200);
    gainSlider.setBounds(sliderArea);
    
    // Position the label above the slider
    gainLabel.setBounds(sliderArea.removeFromTop(20));
}