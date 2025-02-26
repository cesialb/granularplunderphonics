/**
 * @file GranularLookAndFeel.cpp
 * @brief Implementation of the custom look and feel
 */

#include "GranularLookAndFeel.h"

namespace GranularPlunderphonics {

GranularLookAndFeel::GranularLookAndFeel()
{
    setColourScheme(getDarkColourScheme());

    // Set default colors for various component types
    setDarkMode(true);


juce::Colour GranularLookAndFeel::getBackgroundColor() const
{
    return mDarkMode ? mDarkBackground : mLightBackground;
}

juce::Colour GranularLookAndFeel::getPrimaryColor() const
{
    return mDarkMode ? mDarkPrimary : mLightPrimary;
}

juce::Colour GranularLookAndFeel::getSecondaryColor() const
{
    return mDarkMode ? mDarkSecondary : mLightSecondary;
}

juce::Colour GranularLookAndFeel::getHighlightColor() const
{
    return mDarkMode ? mDarkHighlight : mLightHighlight;
}

juce::Colour GranularLookAndFeel::getTextColor() const
{
    return mDarkMode ? mDarkText : mLightText;
}

juce::Colour GranularLookAndFeel::getDisabledTextColor() const
{
    return mDarkMode ? mDarkDisabledText : mLightDisabledText;
}

juce::Font GranularLookAndFeel::getTextButtonFont(juce::TextButton&, int buttonHeight)
{
    return juce::Font("Inter", juce::jmin(16.0f, buttonHeight * 0.6f), juce::Font::plain);
}

juce::Font GranularLookAndFeel::getLabelFont(juce::Label& label)
{
    return juce::Font("Inter", label.getFont().getHeight(), juce::Font::plain);
}

void GranularLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, const float rotaryStartAngle,
                                          const float rotaryEndAngle, juce::Slider& slider)
{
    // Basic properties
    const float radius = juce::jmin(width, height) * 0.4f;
    const float centerX = x + width * 0.5f;
    const float centerY = y + height * 0.5f;
    const float rx = centerX - radius;
    const float ry = centerY - radius;
    const float rw = radius * 2.0f;

    // Fill circle background
    const auto bgColor = slider.findColour(juce::Slider::rotarySliderOutlineColourId);
    g.setColour(bgColor);
    g.fillEllipse(rx, ry, rw, rw);

    // Outline
    g.setColour(bgColor.brighter(0.1f));
    g.drawEllipse(rx, ry, rw, rw, 1.0f);

    if (slider.isEnabled())
    {
        // Draw arc for value
        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        const float arcRadius = radius - 4.0f;

        // Path for arc
        juce::Path valueArc;
        valueArc.addCentredArc(centerX, centerY, arcRadius, arcRadius,
                               0.0f, rotaryStartAngle, angle, true);

        g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
        g.strokePath(valueArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Draw pointer
        juce::Path pointer;
        const float pointerLength = radius * 0.75f;
        const float pointerThickness = 2.0f;

        pointer.addRectangle(-pointerThickness * 0.5f, -radius + 3.0f,
                            pointerThickness, pointerLength);

        g.setColour(slider.findColour(juce::Slider::thumbColourId));
        g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centerX, centerY));
    }

    // Draw center dot
    g.setColour(slider.findColour(juce::Slider::thumbColourId).withAlpha(slider.isEnabled() ? 1.0f : 0.3f));
    g.fillEllipse(centerX - 3.0f, centerY - 3.0f, 6.0f, 6.0f);
}

void GranularLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float minSliderPos, float maxSliderPos,
                                          const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    // Only handle horizontal and vertical styles
    if (style == juce::Slider::LinearHorizontal || style == juce::Slider::LinearVertical)
    {
        const float thickness = juce::jmin(12.0f, style == juce::Slider::LinearHorizontal ? height * 0.25f : width * 0.25f);

        juce::Rectangle<float> sliderBounds;

        if (style == juce::Slider::LinearHorizontal)
        {
            sliderBounds = juce::Rectangle<float>(x, y + (height - thickness) * 0.5f, width, thickness);

            // Draw track background
            g.setColour(slider.findColour(juce::Slider::backgroundColourId));
            g.fillRoundedRectangle(sliderBounds, thickness * 0.5f);

            // Draw filled part
            g.setColour(slider.findColour(juce::Slider::trackColourId));
            g.fillRoundedRectangle(sliderBounds.withWidth(sliderPos - x), thickness * 0.5f);
        }
        else // LinearVertical
        {
            sliderBounds = juce::Rectangle<float>(x + (width - thickness) * 0.5f, y, thickness, height);

            // Draw track background
            g.setColour(slider.findColour(juce::Slider::backgroundColourId));
            g.fillRoundedRectangle(sliderBounds, thickness * 0.5f);

            // Draw filled part
            const float fillHeight = height - (sliderPos - y);
            g.setColour(slider.findColour(juce::Slider::trackColourId));
            g.fillRoundedRectangle(sliderBounds.withY(sliderPos).withHeight(fillHeight), thickness * 0.5f);
        }

        // Draw thumb
        const float thumbWidth = thickness * 1.5f;
        const float thumbHeight = thickness * 1.5f;

        juce::Rectangle<float> thumbBounds;

        if (style == juce::Slider::LinearHorizontal)
        {
            thumbBounds = juce::Rectangle<float>(sliderPos - thumbWidth * 0.5f,
                                              sliderBounds.getCentreY() - thumbHeight * 0.5f,
                                              thumbWidth, thumbHeight);
        }
        else // LinearVertical
        {
            thumbBounds = juce::Rectangle<float>(sliderBounds.getCentreX() - thumbWidth * 0.5f,
                                              sliderPos - thumbHeight * 0.5f,
                                              thumbWidth, thumbHeight);
        }

        g.setColour(slider.findColour(juce::Slider::thumbColourId).withAlpha(slider.isEnabled() ? 1.0f : 0.3f));
        g.fillRoundedRectangle(thumbBounds, thumbWidth * 0.5f);
    }
    else
    {
        // Fall back to default for other slider styles
        juce::LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
    }
}

void GranularLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                                      int buttonX, int buttonY, int buttonW, int buttonH,
                                      juce::ComboBox& comboBox)
{
    // Draw main body
    juce::Rectangle<int> boxBounds(0, 0, width, height);

    g.setColour(comboBox.findColour(juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle(boxBounds.toFloat(), 4.0f);

    g.setColour(comboBox.findColour(juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle(boxBounds.toFloat().reduced(0.5f, 0.5f), 4.0f, 1.0f);

    // Draw arrow
    const float arrowSize = 8.0f;
    const float arrowIndent = 6.0f;

    juce::Path arrow;
    arrow.addTriangle(buttonX + buttonW - arrowIndent - arrowSize, buttonY + buttonH * 0.3f,
                     buttonX + buttonW - arrowIndent, buttonY + buttonH * 0.7f,
                     buttonX + buttonW - arrowIndent - arrowSize * 2.0f, buttonY + buttonH * 0.7f);

    g.setColour(comboBox.findColour(juce::ComboBox::arrowColourId).withAlpha(comboBox.isEnabled() ? 1.0f : 0.3f));
    g.fillPath(arrow);
}

void GranularLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                              const juce::Colour& backgroundColour,
                                              bool isMouseOverButton, bool isButtonDown)
{
    // Handle toggle buttons specially
    if (auto* toggleButton = dynamic_cast<juce::ToggleButton*>(&button))
    {
        auto toggleBackgroundColor = getToggleButtonBackgroundColour(*toggleButton, backgroundColour);

        juce::Rectangle<float> bounds = button.getLocalBounds().toFloat().reduced(1.0f);

        g.setColour(toggleBackgroundColor);
        g.fillRoundedRectangle(bounds, 4.0f);

        if (isMouseOverButton || isButtonDown)
        {
            g.setColour(toggleBackgroundColor.brighter(0.1f));
            g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
        }

        return;
    }

    // Regular text buttons
    auto baseColor = button.getToggleState() ? button.findColour(juce::TextButton::buttonOnColourId)
                                            : backgroundColour;

    if (isButtonDown)
        baseColor = baseColor.darker(0.2f);
    else if (isMouseOverButton)
        baseColor = baseColor.brighter(0.1f);

    juce::Rectangle<float> bounds = button.getLocalBounds().toFloat().reduced(1.0f);

    g.setColour(baseColor);
    g.fillRoundedRectangle(bounds, 4.0f);

    g.setColour(button.findColour(button.getToggleState() ? juce::TextButton::buttonOnColourId
                                                        : juce::TextButton::buttonColourId)
                 .brighter(0.05f));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
}

juce::Colour GranularLookAndFeel::getToggleButtonBackgroundColour(juce::ToggleButton& button, const juce::Colour& backgroundColour)
{
    if (button.getToggleState())
    {
        return button.findColour(juce::ToggleButton::tickColourId);
    }
    else
    {
        return backgroundColour;
    }
}

void GranularLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                          bool isMouseOverButton, bool isButtonDown)
{
    // Use custom implementation for drawing toggle buttons with background color
    drawButtonBackground(g, button, button.findColour(juce::TextButton::buttonColourId),
                        isMouseOverButton, isButtonDown);

    // Draw text
    g.setColour(button.findColour(button.getToggleState() ? juce::TextButton::textColourOnId
                                                       : juce::TextButton::textColourOffId));
    g.setFont(getTextButtonFont(button, button.getHeight()));
    g.drawText(button.getButtonText(), button.getLocalBounds(),
               juce::Justification::centred, true);
}

void GranularLookAndFeel::drawGroupComponentOutline(juce::Graphics& g, int width, int height,
                                                  const juce::String& text, const juce::Justification& justification,
                                                  juce::GroupComponent& group)
{
    // Draw a more subtle group outline
    const float textH = 15.0f;
    const float indent = 3.0f;
    const float border = 1.2f;

    auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();
    auto textW = text.isEmpty() ? 0.0f
                                : group.getLookAndFeel().getLabelFont(group).getStringWidthFloat(text) + 4.0f;

    // Create a path for the outline with space for text
    juce::Path p;
    p.startNewSubPath(indent, textH);
    p.lineTo(indent, bounds.getHeight() - border);
    p.lineTo(bounds.getWidth() - border, bounds.getHeight() - border);
    p.lineTo(bounds.getWidth() - border, textH);

    if (textW > 0)
    {
        auto textX = indent;
        if (justification.getOnlyHorizontalFlags() & juce::Justification::horizontallyCentred)
            textX = bounds.getWidth() * 0.5f - textW * 0.5f;
        else if (justification.getOnlyHorizontalFlags() & juce::Justification::right)
            textX = bounds.getWidth() - textW - 5.0f;

        p.lineTo(textX, textH);
        p.lineTo(textX, 0.0f);
        p.lineTo(textX + textW, 0.0f);
        p.lineTo(textX + textW, textH);
    }

    p.closeSubPath();

    g.setColour(group.findColour(juce::GroupComponent::outlineColourId));
    g.strokePath(p, juce::PathStrokeType(1.0f));

    if (!text.isEmpty())
    {
        auto textX = indent;
        if (justification.getOnlyHorizontalFlags() & juce::Justification::horizontallyCentred)
            textX = bounds.getWidth() * 0.5f - textW * 0.5f;
        else if (justification.getOnlyHorizontalFlags() & juce::Justification::right)
            textX = bounds.getWidth() - textW - 5.0f;

        g.setColour(group.findColour(juce::GroupComponent::textColourId));
        g.setFont(getLabelFont(group));
        g.drawText(text, juce::Rectangle<float>(textX, 0.0f, textW, textH),
                  juce::Justification::centred, true);
    }
}

} // namespace GranularPlunderphonics

void GranularLookAndFeel::setDarkMode(bool darkMode)
{
    mDarkMode = darkMode;

    // Set up color scheme based on mode
    if (darkMode)
    {
        setColourScheme(getDarkColourScheme());

        // Component-specific colors
        setColour(juce::ResizableWindow::backgroundColourId, mDarkBackground);
        setColour(juce::PopupMenu::backgroundColourId, mDarkBackground);
        setColour(juce::TextEditor::backgroundColourId, mDarkBackground.brighter(0.1f));
        setColour(juce::TextEditor::textColourId, mDarkText);
        setColour(juce::TextEditor::highlightColourId, mDarkPrimary.withAlpha(0.3f));
        setColour(juce::TextEditor::highlightedTextColourId, mDarkText);
        setColour(juce::TextEditor::outlineColourId, mDarkBackground.brighter(0.2f));
        setColour(juce::TextEditor::focusedOutlineColourId, mDarkPrimary);

        setColour(juce::Label::textColourId, mDarkText);
        setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);

        setColour(juce::ScrollBar::backgroundColourId, juce::Colours::transparentBlack);
        setColour(juce::ScrollBar::thumbColourId, mDarkText.withAlpha(0.5f));
        setColour(juce::ScrollBar::trackColourId, mDarkBackground.brighter(0.05f));

        setColour(juce::ComboBox::backgroundColourId, mDarkBackground.brighter(0.1f));
        setColour(juce::ComboBox::textColourId, mDarkText);
        setColour(juce::ComboBox::outlineColourId, mDarkBackground.brighter(0.2f));
        setColour(juce::ComboBox::buttonColourId, mDarkPrimary);
        setColour(juce::ComboBox::arrowColourId, mDarkText);

        setColour(juce::TextButton::buttonColourId, mDarkBackground.brighter(0.15f));
        setColour(juce::TextButton::buttonOnColourId, mDarkPrimary);
        setColour(juce::TextButton::textColourOnId, mDarkText);
        setColour(juce::TextButton::textColourOffId, mDarkText);

        setColour(juce::ToggleButton::textColourId, mDarkText);
        setColour(juce::ToggleButton::tickColourId, mDarkPrimary);
        setColour(juce::ToggleButton::tickDisabledColourId, mDarkDisabledText);

        setColour(juce::Slider::backgroundColourId, mDarkBackground.brighter(0.05f));
        setColour(juce::Slider::thumbColourId, mDarkPrimary);
        setColour(juce::Slider::trackColourId, mDarkBackground.brighter(0.2f));
        setColour(juce::Slider::rotarySliderFillColourId, mDarkPrimary);
        setColour(juce::Slider::rotarySliderOutlineColourId, mDarkBackground.brighter(0.2f));
        setColour(juce::Slider::textBoxTextColourId, mDarkText);
        setColour(juce::Slider::textBoxBackgroundColourId, mDarkBackground.brighter(0.1f));
        setColour(juce::Slider::textBoxHighlightColourId, mDarkPrimary.withAlpha(0.3f));
        setColour(juce::Slider::textBoxOutlineColourId, mDarkBackground.brighter(0.2f));

        setColour(juce::GroupComponent::outlineColourId, mDarkBackground.brighter(0.2f));
        setColour(juce::GroupComponent::textColourId, mDarkText);
    }
    else
    {
        setColourScheme(getLightColourScheme());

        // Component-specific colors
        setColour(juce::ResizableWindow::backgroundColourId, mLightBackground);
        setColour(juce::PopupMenu::backgroundColourId, mLightBackground);
        setColour(juce::TextEditor::backgroundColourId, mLightBackground.darker(0.05f));
        setColour(juce::TextEditor::textColourId, mLightText);
        setColour(juce::TextEditor::highlightColourId, mLightPrimary.withAlpha(0.3f));
        setColour(juce::TextEditor::highlightedTextColourId, mLightText);
        setColour(juce::TextEditor::outlineColourId, mLightBackground.darker(0.2f));
        setColour(juce::TextEditor::focusedOutlineColourId, mLightPrimary);

        setColour(juce::Label::textColourId, mLightText);
        setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);

        setColour(juce::ScrollBar::backgroundColourId, juce::Colours::transparentBlack);
        setColour(juce::ScrollBar::thumbColourId, mLightText.withAlpha(0.5f));
        setColour(juce::ScrollBar::trackColourId, mLightBackground.darker(0.05f));

        setColour(juce::ComboBox::backgroundColourId, mLightBackground.darker(0.05f));
        setColour(juce::ComboBox::textColourId, mLightText);
        setColour(juce::ComboBox::outlineColourId, mLightBackground.darker(0.2f));
        setColour(juce::ComboBox::buttonColourId, mLightPrimary);
        setColour(juce::ComboBox::arrowColourId, mLightText);

        setColour(juce::TextButton::buttonColourId, mLightBackground.darker(0.1f));
        setColour(juce::TextButton::buttonOnColourId, mLightPrimary);
        setColour(juce::TextButton::textColourOnId, mLightBackground);
        setColour(juce::TextButton::textColourOffId, mLightText);

        setColour(juce::ToggleButton::textColourId, mLightText);
        setColour(juce::ToggleButton::tickColourId, mLightPrimary);
        setColour(juce::ToggleButton::tickDisabledColourId, mLightDisabledText);

        setColour(juce::Slider::backgroundColourId, mLightBackground.darker(0.05f));
        setColour(juce::Slider::thumbColourId, mLightPrimary);
        setColour(juce::Slider::trackColourId, mLightBackground.darker(0.1f));
        setColour(juce::Slider::rotarySliderFillColourId, mLightPrimary);
        setColour(juce::Slider::rotarySliderOutlineColourId, mLightBackground.darker(0.2f));
        setColour(juce::Slider::textBoxTextColourId, mLightText);
        setColour(juce::Slider::textBoxBackgroundColourId, mLightBackground.darker(0.05f));
        setColour(juce::Slider::textBoxHighlightColourId, mLightPrimary.withAlpha(0.3f));
        setColour(juce::Slider::textBoxOutlineColourId, mLightBackground.darker(0.2f));

        setColour(juce::GroupComponent::outlineColourId, mLightBackground.darker(0.2f));
        setColour(juce::GroupComponent::textColourId, mLightText);
    }
};