/**
 * @file HeaderPanel.h
 * @brief Header panel component with logo, title, and UI controls
 */

#pragma once

#include <JuceHeader.h>
#include "../UI/UISettings.h"

namespace GranularPlunderphonics {

class GranularPlunderphonicsEditor;

/**
 * @class HeaderPanel
 * @brief Header panel component for the plugin UI
 *
 * This panel contains the plugin logo, title, and UI controls like
 * dark mode toggle and UI scaling.
 */
class HeaderPanel : public juce::Component,
                    private juce::Button::Listener,
                    private juce::Slider::Listener
{
public:
    /**
     * @brief Constructor
     * @param editor Reference to the main editor
     */
    explicit HeaderPanel(GranularPlunderphonicsEditor& editor);

    /**
     * @brief Destructor
     */
    ~HeaderPanel() override;

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

private:
    /**
     * @brief Handle button clicks
     * @param button Button that was clicked
     */
    void buttonClicked(juce::Button* button) override;

    /**
     * @brief Handle slider value changes
     * @param slider Slider that changed
     */
    void sliderValueChanged(juce::Slider* slider) override;

    //==============================================================================
    // Reference to main editor
    GranularPlunderphonicsEditor& mEditor;

    // UI components
    juce::Label mTitleLabel;
    juce::ToggleButton mDarkModeButton;
    juce::Slider mScaleSlider;
    juce::Label mScaleLabel;
    juce::TextButton mPresetButton;
    juce::TextButton mSettingsButton;

    // Logo image
    juce::Image mLogo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderPanel)
};

} // namespace GranularPlunderphonics