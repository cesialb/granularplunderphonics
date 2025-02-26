/**
 * @file GranularPlunderphonicsEditor.cpp
 * @brief Implementation of the main editor component
 */

#include "GranularPlunderphonicsEditor.h"
#include "plugin/GranularPlunderphonicsProcessor.h"

namespace GranularPlunderphonics {

// Default dimensions for the editor
static constexpr int DEFAULT_WIDTH = 800;
static constexpr int DEFAULT_HEIGHT = 600;

// Minimum dimensions
static constexpr int MIN_WIDTH = 400;
static constexpr int MIN_HEIGHT = 300;

// Timer update interval in milliseconds
static constexpr int UI_UPDATE_INTERVAL_MS = 30;

GranularPlunderphonicsEditor::GranularPlunderphonicsEditor(juce::AudioProcessor& processor)
    : AudioProcessorEditor(&processor)
    , mDarkModeValue(var(mSettings.darkMode))
    , mScaleFactorValue(var(mSettings.scaleFactor))
{
    // Initialize UI settings
    mSettings.darkMode = true;
    mSettings.scaleFactor = 1.0f;
    mSettings.fontSizePt = 14;

    // Set up value listeners
    mDarkModeValue.addListener(this);
    mScaleFactorValue.addListener(this);

    // Initialize components
    initializeComponents();

    // Apply look and feel
    applyTheme();

    // Set default size
    setResizable(true, true);
    setResizeLimits(MIN_WIDTH, MIN_HEIGHT,
                    DEFAULT_WIDTH * 2, DEFAULT_HEIGHT * 2);
    setSize(DEFAULT_WIDTH, DEFAULT_HEIGHT);

    // Start timer for UI updates
    startTimerHz(1000 / UI_UPDATE_INTERVAL_MS);
}

GranularPlunderphonicsEditor::~GranularPlunderphonicsEditor()
{
    // Stop timer first to prevent any callbacks during destruction
    stopTimer();

    // Remove value listeners
    mDarkModeValue.removeListener(this);
    mScaleFactorValue.removeListener(this);

    // Clear pointers
    mHeaderPanel = nullptr;
    mMainPanel = nullptr;
}

void GranularPlunderphonicsEditor::paint(juce::Graphics& g)
{
    // Start timing the render
    juce::Time renderStartTime = juce::Time::getMillisecondCounter();

    auto bounds = getLocalBounds();

    // Use cached background if available and valid
    if (mBackgroundImage.isNull() || mBackgroundImage.getWidth() != bounds.getWidth() ||
        mBackgroundImage.getHeight() != bounds.getHeight())
    {
        // Re-create background image at current size
        mBackgroundImage = juce::Image(juce::Image::ARGB, bounds.getWidth(), bounds.getHeight(), true);

        juce::Graphics bg(mBackgroundImage);

        // Fill with main background color
        bg.fillAll(mLookAndFeel.getBackgroundColor());

        // Add any background decorations here
        if (mSettings.darkMode)
        {
            // Subtle gradient for dark mode
            juce::ColourGradient gradient(
                mLookAndFeel.getBackgroundColor().brighter(0.05f),
                0.0f, 0.0f,
                mLookAndFeel.getBackgroundColor().darker(0.05f),
                static_cast<float>(bounds.getWidth()),
                static_cast<float>(bounds.getHeight()),
                false
            );
            bg.setGradientFill(gradient);
            bg.fillRect(bounds.toFloat());
        }
        else
        {
            // Light mode gradient
            juce::ColourGradient gradient(
                mLookAndFeel.getBackgroundColor().brighter(0.05f),
                0.0f, 0.0f,
                mLookAndFeel.getBackgroundColor().darker(0.05f),
                static_cast<float>(bounds.getWidth()),
                static_cast<float>(bounds.getHeight()),
                false
            );
            bg.setGradientFill(gradient);
            bg.fillRect(bounds.toFloat());
        }

        // Add subtle grid lines or texture if desired
        // ...
    }

    // Draw the cached background
    g.drawImageAt(mBackgroundImage, 0, 0);

    // Calculate and track render time for performance monitoring
    juce::Time renderEndTime = juce::Time::getMillisecondCounter();
    int renderTimeMs = static_cast<int>(renderEndTime.toMilliseconds() - renderStartTime.toMilliseconds());

    // Low-pass filter the CPU usage calculation to avoid jumps
    constexpr float alpha = 0.1f;
    float instantCpuUsage = static_cast<float>(renderTimeMs) / UI_UPDATE_INTERVAL_MS;
    mCpuUsage = mCpuUsage * (1.0f - alpha) + instantCpuUsage * alpha;

    // In development/debug builds, show rendering stats
#ifdef DEBUG
    g.setColour(mLookAndFeel.getTextColor().withAlpha(0.6f));
    g.setFont(mSettings.fontSizePt * 0.8f);
    g.drawText("UI CPU: " + juce::String(mCpuUsage * 100.0f, 1) + "%",
               bounds.removeFromBottom(20).reduced(5, 0),
               juce::Justification::bottomRight, true);
#endif
}

void GranularPlunderphonicsEditor::resized()
{
    updateLayout();
}

void GranularPlunderphonicsEditor::timerCallback()
{
    // Update realtime visualizations
    repaint();

    // Update processor state
    if (auto* granularProcessor = dynamic_cast<GranularPlunderphonicsProcessor*>(getAudioProcessor()))
    {
        // Update UI from processor state if needed
        // ...
    }
}

void GranularPlunderphonicsEditor::setDarkMode(bool darkMode)
{
    if (mSettings.darkMode != darkMode)
    {
        mSettings.darkMode = darkMode;
        mDarkModeValue = var(darkMode);
        applyTheme();

        // Force background cache update
        mBackgroundImage = juce::Image();

        repaint();
    }
}

void GranularPlunderphonicsEditor::setScaleFactor(float scale)
{
    // Clamp scale factor to valid range (50% to 200%)
    scale = juce::jlimit(0.5f, 2.0f, scale);

    if (!juce::approximatelyEqual(mSettings.scaleFactor, scale))
    {
        mSettings.scaleFactor = scale;
        mScaleFactorValue = var(scale);

        // Update font size based on scale
        mSettings.fontSizePt = juce::roundToInt(14.0f * scale);

        // Update layout for new scale
        updateLayout();

        // Force background cache update
        mBackgroundImage = juce::Image();

        repaint();
    }
}

void GranularPlunderphonicsEditor::valueChanged(juce::Value& value)
{
    if (value.refersToSameSourceAs(mDarkModeValue))
    {
        setDarkMode(static_cast<bool>(value.getValue()));
    }
    else if (value.refersToSameSourceAs(mScaleFactorValue))
    {
        setScaleFactor(static_cast<float>(static_cast<double>(value.getValue())));
    }
}

void GranularPlunderphonicsEditor::initializeComponents()
{
    // Initialize header panel
    mHeaderPanel = std::make_unique<HeaderPanel>(*this);
    addAndMakeVisible(mHeaderPanel.get());

    // Initialize main panel
    mMainPanel = std::make_unique<MainPanel>(*this);
    addAndMakeVisible(mMainPanel.get());
}

void GranularPlunderphonicsEditor::applyTheme()
{
    // Set up look and feel based on current settings
    mLookAndFeel.setDarkMode(mSettings.darkMode);

    // Apply look and feel to this component and all children
    setLookAndFeel(&mLookAndFeel);

    // Explicitly update header and main panel
    if (mHeaderPanel)
        mHeaderPanel->themeChanged();

    if (mMainPanel)
        mMainPanel->themeChanged();
}

void GranularPlunderphonicsEditor::updateLayout()
{
    auto bounds = getLocalBounds();

    // Layout header panel (fixed height, full width)
    const int headerHeight = juce::roundToInt(60.0f * mSettings.scaleFactor);
    if (mHeaderPanel)
        mHeaderPanel->setBounds(bounds.removeFromTop(headerHeight));

    // Layout main panel (remaining space)
    if (mMainPanel)
        mMainPanel->setBounds(bounds);
}

juce::Rectangle<int> GranularPlunderphonicsEditor::calculateScaledBounds() const
{
    // Calculate the size based on scale factor
    int width = juce::roundToInt(DEFAULT_WIDTH * mSettings.scaleFactor);
    int height = juce::roundToInt(DEFAULT_HEIGHT * mSettings.scaleFactor);

    return juce::Rectangle<int>(0, 0, width, height);
}

} // namespace GranularPlunderphonics