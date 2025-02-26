/**
 * @file MainPanel.h
 * @brief Main content panel for the plugin UI
 */

#pragma once

#include <JuceHeader.h>
#include "../UI/UISettings.h"

namespace GranularPlunderphonics {

class GranularPlunderphonicsEditor;

/**
 * @class MainPanel
 * @brief Main content panel for the plugin UI
 *
 * This panel contains the main interface for the plugin, including
 * waveform display, grain controls, and modulation matrix.
 */
class MainPanel : public juce::Component,
                  private juce::Timer
{
public:
    /**
     * @brief Constructor
     * @param editor Reference to the main editor
     */
    explicit MainPanel(GranularPlunderphonicsEditor& editor);

    /**
     * @brief Destructor
     */
    ~MainPanel() override;

    //==============================================================================
    /**
     * @brief Paint the component
     * @param g Graphics context
     */
    void paint(juce::Graphics& g) override;

    /**
     * @brief Handle component resizing
     */
    void resized() override;

    /**
     * @brief Update component after theme changes
     */
    void themeChanged();

    /**
     * @brief Timer callback for UI updates
     */
    void timerCallback() override;

private:
    /**
     * @brief Initialize all child components
     */
    void initializeComponents();

    /**
     * @brief Create the main tab layout
     */
    void createTabLayout();

    /**
     * @brief Create the waveform display
     */
    void createWaveformDisplay();

    /**
     * @brief Create the grain controls
     */
    void createGrainControls();

    /**
     * @brief Create the modulation matrix
     */
    void createModulationMatrix();

    /**
     * @brief Create the attractor visualization
     */
    void createAttractorViz();

    //==============================================================================
    // Reference to main editor
    GranularPlunderphonicsEditor& mEditor;

    // Main tab component
    std::unique_ptr<juce::TabbedComponent> mTabComponent;

    // Tab content components
    std::unique_ptr<juce::Component> mMainTab;
    std::unique_ptr<juce::Component> mGrainTab;
    std::unique_ptr<juce::Component> mModulationTab;
    std::unique_ptr<juce::Component> mAttractorTab;

    // Main tab components
    std::unique_ptr<juce::Component> mWaveformDisplay;

    // Grain tab components
    juce::OwnedArray<juce::Slider> mGrainSliders;
    juce::OwnedArray<juce::Label> mGrainLabels;

    // Cached images for performance
    juce::Image mBackgroundImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainPanel)
};

} // namespace GranularPlunderphonics