#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GranularPlunderphonicsAudioProcessor::GranularPlunderphonicsAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::mono(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters", {
          std::make_unique<juce::AudioParameterFloat>("gain", "Gain", 0.0f, 1.0f, 0.5f)
      })
{
    gainParameter = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("gain"));
    jassert(gainParameter != nullptr);
}

GranularPlunderphonicsAudioProcessor::~GranularPlunderphonicsAudioProcessor()
{
    // Resource cleanup happens automatically through JUCE smart pointers
}

//==============================================================================
const juce::String GranularPlunderphonicsAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool GranularPlunderphonicsAudioProcessor::acceptsMidi() const
{
    return false;
}

bool GranularPlunderphonicsAudioProcessor::producesMidi() const
{
    return false;
}

bool GranularPlunderphonicsAudioProcessor::isMidiEffect() const
{
    return false;
}

double GranularPlunderphonicsAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GranularPlunderphonicsAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int GranularPlunderphonicsAudioProcessor::getCurrentProgram()
{
    return 0;
}

void GranularPlunderphonicsAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String GranularPlunderphonicsAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void GranularPlunderphonicsAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void GranularPlunderphonicsAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialization that you need..
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void GranularPlunderphonicsAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool GranularPlunderphonicsAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // This plugin supports mono input to stereo output
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // Input must be mono
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::mono())
        return false;

    return true;
}

void GranularPlunderphonicsAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Get the gain value from the parameter
    float gain = gainParameter->get();

    // Simple pass-through implementation (mono->stereo)
    // Copy the mono input to both stereo channels with gain applied
    if (totalNumInputChannels == 1 && totalNumOutputChannels == 2) {
        // Get mono input data
        auto* monoData = buffer.getReadPointer(0);
        
        // Get stereo output channels
        auto* leftChannel = buffer.getWritePointer(0);
        auto* rightChannel = buffer.getWritePointer(1);
        
        // Copy mono to stereo with gain applied
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            float sampleValue = monoData[sample] * gain;
            leftChannel[sample] = sampleValue;
            rightChannel[sample] = sampleValue;
        }
    }
}

//==============================================================================
bool GranularPlunderphonicsAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* GranularPlunderphonicsAudioProcessor::createEditor()
{
    return new GranularPlunderphonicsAudioProcessorEditor(*this, parameters);
}

//==============================================================================
void GranularPlunderphonicsAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Save parameters
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void GranularPlunderphonicsAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Restore parameters
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState.get() != nullptr && xmlState->hasTagName(parameters.state.getType()))
    {
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GranularPlunderphonicsAudioProcessor();
}