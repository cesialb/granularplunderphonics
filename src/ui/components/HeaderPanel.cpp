/**
 * @file HeaderPanel.cpp
 * @brief Implementation of the header panel component
 */

#include "HeaderPanel.h"
#include "../../GranularPlunderphonicsEditor.h"
#include "../../LookAndFeel/GranularLookAndFeel.h"

namespace GranularPlunderphonics {

HeaderPanel::HeaderPanel(GranularPlunderphonicsEditor& editor)
    : mEditor(editor)
{
    // Set up title label
    mTitleLabel.setText("Granular Plunderphonics", juce::dontSendNotification);
    mTitleLabel.setFont(juce::Font("Inter", 20.0f, juce::Font::bold));
    mTitleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(mTitleLabel);

    // Set up dark mode toggle
    mDarkModeButton.setButtonText("Dark Mode");
    mDarkModeButton.setToggleState(editor.isDarkMode(), juce::dontSendNotification);
    mDarkModeButton.addListener(this);
    addAndMakeVisible(mDarkModeButton);

    // Set up scale slider
    mScaleSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    mScaleSlider.setRange(0.5, 2.0, 0.01);
    mScaleSlider.setValue(editor.getScaleFactor(), juce::dontSendNotification);
    mScaleSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    mScaleSlider.addListener(this);
    addAndMakeVisible(mScaleSlider);

    // Set up scale label
    mScaleLabel.setText("UI Scale", juce::dontSendNotification);
    mScaleLabel.setFont(juce::Font("Inter", 14.0f, juce::Font::plain));
    mScaleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(mScaleLabel);

    // Set up preset button
    mPresetButton.setButtonText("Presets");
    mPresetButton.addListener(this);
    addAndMakeVisible(mPresetButton);

    // Set up settings button
    mSettingsButton.setButtonText("Settings");
    mSettingsButton.addListener(this);
    addAndMakeVisible(mSettingsButton);

    // Create placeholder logo
    mLogo = juce::Image(juce::Image::ARGB, 40, 40, true);
    juce::Graphics g(mLogo);
    g.setColour(juce::Colours::white);
    g.fillEllipse(4.0f, 4.0f, 32.0f, 32.0f);
    g.setColour(juce::Colours::black);
    g.drawEllipse(4.0f, 4.0f, 32.0f, 32.0f, 2.0f);
}

HeaderPanel::~HeaderPanel()
{
    mDarkModeButton.removeListener(this);
    mScaleSlider.removeListener(this);
    mPresetButton.removeListener(this);
    mSettingsButton.removeListener(this);
}

void HeaderPanel::paint(juce::Graphics& g)
{
    // Get look and feel colors
    auto* lookAndFeel = dynamic_cast<GranularLookAndFeel*>(&getLookAndFeel());
    juce::Colour bgColor = lookAndFeel ? lookAndFeel->getBackgroundColor() : juce::Colours::darkgrey;
    juce::Colour textColor = lookAndFeel ? lookAndFeel->getTextColor() : juce::Colours::white;

    // Fill background with slightly darker/lighter than main background
    g.fillAll(mEditor.isDarkMode() ? bgColor.darker(0.2f) : bgColor.darker(0.05f));

    // Draw subtle bottom border
    g.setColour(mEditor.isDarkMode() ? bgColor.brighter(0.1f) : bgColor.darker(0.1f));
    g.fillRect(0, getHeight() - 1, getWidth(), 1);

    // Draw logo
    g.drawImageAt(mLogo, 10, (getHeight() - mLogo.getHeight()) / 2);
}

void HeaderPanel::resized()
{
    auto bounds = getLocalBounds().reduced(10, 5);

    // Logo space
    bounds.removeFromLeft(50);

    // Title label
    mTitleLabel.setBounds(bounds.removeFromLeft(250));

    // Right-aligned controls
    auto rightControls = bounds.removeFromRight(400);

    // Settings button
    mSettingsButton.setBounds(rightControls.removeFromRight(80).reduced(2, 5));
    rightControls.removeFromRight(10); // spacing

    // Preset button
    mPresetButton.setBounds(rightControls.removeFromRight(80).reduced(2, 5));
    rightControls.removeFromRight(10); // spacing

    // Dark mode toggle
    mDarkModeButton.setBounds(rightControls.removeFromRight(100).reduced(2, 5));
    rightControls.removeFromRight(10); // spacing

    // Scale controls
    mScaleLabel.setBounds(rightControls.removeFromLeft(60).reduced(2, 5));
    mScaleSlider.setBounds(rightControls.reduced(2, 5));
}

void HeaderPanel::themeChanged()
{
    // Update component colors based on the current theme
    auto* lookAndFeel = dynamic_cast<GranularLookAndFeel*>(&getLookAndFeel());
    if (lookAndFeel)
    {
        mTitleLabel.setColour(juce::Label::textColourId, lookAndFeel->getTextColor());
        mScaleLabel.setColour(juce::Label::textColourId, lookAndFeel->getTextColor());
    }

    // Update toggle state to match editor
    mDarkModeButton.setToggleState(mEditor.isDarkMode(), juce::dontSendNotification);

    repaint();
}

void HeaderPanel::buttonClicked(juce::Button* button)
{
    if (button == &mDarkModeButton)
    {
        mEditor.setDarkMode(mDarkModeButton.getToggleState());
    }
    else if (button == &mPresetButton)
    {
        // Show preset manager (to be implemented)
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                                              "Presets",
                                              "Preset manager will be implemented in a future version.",
                                              "OK");
    }
    else if (button == &mSettingsButton)
    {
        // Show settings (to be implemented)
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                                              "Settings",
                                              "Settings panel will be implemented in a future version.",
                                              "OK");
    }
}

void HeaderPanel::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &mScaleSlider)
    {
        mEditor.setScaleFactor(static_cast<float>(mScaleSlider.getValue()));
    }
}

} // namespace GranularPlunderphonics