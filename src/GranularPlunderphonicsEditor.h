/**
 * @file GranularPlunderphonicsEditor.h
 * @brief Main editor component for the GranularPlunderphonics VST3 plugin
 */

#pragma once

#include <JuceHeader.h>
#include "LookAndFeel/GranularLookAndFeel.h"
#include "Components/MainPanel.h"
#include "Components/HeaderPanel.h"
#include "UI/UISettings.h"

namespace GranularPlunderphonics {

/**
 * @class GranularPlunderphonicsEditor
 * @brief Main editor component for the GranularPlunderphonics VST3 plugin
 *
 * This class serves as the main plugin editor, handling all UI-related functionality
 * and coordinating communication between the processor and the UI components.
 */
class GranularPlunderphonicsEditor  : public juce::AudioProcessorEditor,
                                      public juce::Timer,
                                      private juce::Value::Listener
{
public:
    /**
     * @brief Constructor
     * @param processor The audio processor to connect to
     */
    explicit GranularPlunderphonicsEditor(juce::AudioProcessor& processor);

    /**
     * @brief Destructor
     */
    ~GranularPlunderphonicsEditor() override;

    //==============================================================================
    /**
     * @brief Paint the editor component
     * @param g Graphics context for painting
     */
    void paint(juce::Graphics& g) override;

    /**
     * @brief Handle component resizing
     */
    void resized() override;

    /**
     * @brief Called periodically by the timer
     *
     * Used for UI updates that should happen at regular intervals
     */
    void timerCallback() override;

    /**
     * @brief Toggle dark mode
     * @param darkMode True to enable dark mode, false to disable
     */
    void setDarkMode(bool darkMode);

    /**
     * @brief Get current dark mode state
     * @return True if dark mode is enabled
     */
    bool isDarkMode() const { return mSettings.darkMode; }

    /**
     * @brief Set UI scale factor
     * @param scale Scale factor (0.5 to 2.0)
     */
    void setScaleFactor(float scale);

    /**
     * @brief Get current scale factor
     * @return Current UI scale factor
     */
    float getScaleFactor() const { return mSettings.scaleFactor; }

    /**
     * @brief Get access to the UI settings
     * @return Reference to UI settings
     */
    UISettings& getUISettings() { return mSettings; }

    /**
     * @brief Get access to the UI settings (const version)
     * @return Const reference to UI settings
     */
    const UISettings& getUISettings() const { return mSettings; }

private:
    /**
     * @brief Handle value changes from Value objects
     * @param value The Value that changed
     */
    void valueChanged(juce::Value& value) override;

    /**
     * @brief Initialize components and layout
     */
    void initializeComponents();

    /**
     * @brief Apply current theme to all components
     */
    void applyTheme();

    /**
     * @brief Update layout based on current size and scale
     */
    void updateLayout();

    /**
     * @brief Calculate component bounds
     * @return Rectangle representing component bounds
     */
    juce::Rectangle<int> calculateScaledBounds() const;

    //==============================================================================
    // Custom Look and Feel
    GranularLookAndFeel mLookAndFeel;

    // UI settings
    UISettings mSettings;

    // UI Components
    std::unique_ptr<HeaderPanel> mHeaderPanel;
    std::unique_ptr<MainPanel> mMainPanel;

    // Cached images for efficient rendering
    juce::Image mBackgroundImage;

    // State values
    juce::Value mDarkModeValue;
    juce::Value mScaleFactorValue;

    // Performance measurement
    juce::Time mLastRenderTime;
    float mCpuUsage = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GranularPlunderphonicsEditor)
};

} // namespace GranularPlunderphonics