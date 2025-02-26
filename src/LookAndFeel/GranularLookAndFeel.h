/**
 * @file GranularLookAndFeel.h
 * @brief Custom look and feel class for the GranularPlunderphonics plugin
 */

#pragma once

#include <JuceHeader.h>

namespace GranularPlunderphonics {

/**
 * @class GranularLookAndFeel
 * @brief Custom look and feel class for a consistent visual style across the plugin
 *
 * This class extends the JUCE LookAndFeel_V4 to provide a custom appearance
 * for all UI elements in the GranularPlunderphonics plugin. It supports both
 * dark and light modes with high contrast visuals.
 */
class GranularLookAndFeel : public juce::LookAndFeel_V4
{
public:
    /**
     * @brief Constructor
     */
    GranularLookAndFeel();

    /**
     * @brief Set dark mode status
     * @param darkMode True for dark mode, false for light mode
     */
    void setDarkMode(bool darkMode);

    /**
     * @brief Get whether dark mode is active
     * @return True if dark mode is enabled
     */
    bool isDarkMode() const { return mDarkMode; }

    /**
     * @brief Get the background color based on current theme
     * @return Background color
     */
    juce::Colour getBackgroundColor() const;

    /**
     * @brief Get the primary accent color based on current theme
     * @return Primary accent color
     */
    juce::Colour getPrimaryColor() const;

    /**
     * @brief Get the secondary accent color based on current theme
     * @return Secondary accent color
     */
    juce::Colour getSecondaryColor() const;

    /**
     * @brief Get the highlight color based on current theme
     * @return Highlight color
     */
    juce::Colour getHighlightColor() const;

    /**
     * @brief Get the main text color based on current theme
     * @return Text color
     */
    juce::Colour getTextColor() const;

    /**
     * @brief Get the disabled text color based on current theme
     * @return Disabled text color
     */
    juce::Colour getDisabledTextColor() const;

    //==============================================================================
    // LookAndFeel overrides

    /**
     * @brief Create a font for drawing text
     * @param fontName Type of component to get font for
     * @return Font to use
     */
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;

    /**
     * @brief Get font for labels
     * @param label Label to get font for
     * @return Font to use
     */
    juce::Font getLabelFont(juce::Label& label) override;

    /**
     * @brief Draw a rotary slider
     * @param g Graphics context to draw on
     * @param x X position
     * @param y Y position
     * @param width Width of slider
     * @param height Height of slider
     * @param sliderPos Current position (0-1)
     * @param rotaryStartAngle Start angle in radians
     * @param rotaryEndAngle End angle in radians
     * @param slider Slider being drawn
     */
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, const float rotaryStartAngle,
                           const float rotaryEndAngle, juce::Slider& slider) override;

    /**
     * @brief Draw a linear slider
     * @param g Graphics context to draw on
     * @param x X position
     * @param y Y position
     * @param width Width of slider
     * @param height Height of slider
     * @param sliderPos Current position (pixels)
     * @param minSliderPos Minimum position (pixels)
     * @param maxSliderPos Maximum position (pixels)
     * @param style Slider style
     * @param slider Slider being drawn
     */
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const juce::Slider::SliderStyle style, juce::Slider& slider) override;

    /**
     * @brief Draw a combo box
     * @param g Graphics context to draw on
     * @param width Width of combo box
     * @param height Height of combo box
     * @param isButtonDown Whether button is pressed
     * @param buttonX X position of button area
     * @param buttonY Y position of button area
     * @param buttonW Width of button area
     * @param buttonH Height of button area
     * @param comboBox Combo box being drawn
     */
    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox& comboBox) override;

    /**
     * @brief Draw a button
     * @param g Graphics context to draw on
     * @param button Button to draw
     * @param backgroundColour Base color for the button
     * @param isMouseOverButton Whether mouse is over the button
     * @param isButtonDown Whether button is pressed
     */
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool isMouseOverButton, bool isButtonDown) override;

    /**
     * @brief Get background color for a toggle button
     * @param button Toggle button
     * @param backgroundColour Suggested background color
     * @return Background color to use
     */
    juce::Colour getToggleButtonBackgroundColour(juce::ToggleButton& button, const juce::Colour& backgroundColour);

    /**
     * @brief Draw a toggle button
     * @param g Graphics context to draw on
     * @param button Toggle button to draw
     * @param isMouseOverButton Whether mouse is over the button
     * @param isButtonDown Whether button is pressed
     */
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                           bool isMouseOverButton, bool isButtonDown) override;

    /**
     * @brief Draw a group box outline
     * @param g Graphics context
     * @param width Width
     * @param height Height
     * @param text Group box title
     * @param justification Text justification
     * @param group The GroupComponent
     */
    void drawGroupComponentOutline(juce::Graphics& g, int width, int height,
                                    const juce::String& text, const juce::Justification& justification,
                                    juce::GroupComponent& group) override;

private:
    bool mDarkMode = true;

    // Dark mode colors
    juce::Colour mDarkBackground = juce::Colour(0xFF1E1E1E);
    juce::Colour mDarkPrimary = juce::Colour(0xFF48A9E6);
    juce::Colour mDarkSecondary = juce::Colour(0xFFE67E22);
    juce::Colour mDarkHighlight = juce::Colour(0xFF9B59B6);
    juce::Colour mDarkText = juce::Colour(0xFFE0E0E0);
    juce::Colour mDarkDisabledText = juce::Colour(0xFF808080);

    // Light mode colors
    juce::Colour mLightBackground = juce::Colour(0xFFF5F5F5);
    juce::Colour mLightPrimary = juce::Colour(0xFF2980B9);
    juce::Colour mLightSecondary = juce::Colour(0xFFD35400);
    juce::Colour mLightHighlight = juce::Colour(0xFF8E44AD);
    juce::Colour mLightText = juce::Colour(0xFF202020);
    juce::Colour mLightDisabledText = juce::Colour(0xFF909090);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GranularLookAndFeel)
};

} // namespace GranularPlunderphonics