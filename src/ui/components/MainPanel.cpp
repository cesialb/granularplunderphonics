/**
 * @file MainPanel.cpp
 * @brief Implementation of the main content panel
 */

#include "MainPanel.h"
#include "../GranularPlunderphonicsEditor.h"
#include "../LookAndFeel/GranularLookAndFeel.h"

namespace GranularPlunderphonics {

MainPanel::MainPanel(GranularPlunderphonicsEditor& editor)
    : mEditor(editor)
{
    // Initialize all components
    initializeComponents();

    // Start timer for UI updates (30 fps)
    startTimer(33);
}

MainPanel::~MainPanel()
{
    stopTimer();
}

void MainPanel::paint(juce::Graphics& g)
{
    // Get look and feel colors
    auto* lookAndFeel = dynamic_cast<GranularLookAndFeel*>(&getLookAndFeel());
    juce::Colour bgColor = lookAndFeel ? lookAndFeel->getBackgroundColor() : juce::Colours::darkgrey;

    // Fill with main background color
    g.fillAll(bgColor);
}

void MainPanel::resized()
{
    // Update tab component to fill the available space
    mTabComponent->setBounds(getLocalBounds());

    // Update cached background image size if needed
    auto bounds = getLocalBounds();
    if (mBackgroundImage.isNull() || mBackgroundImage.getWidth() != bounds.getWidth() ||
        mBackgroundImage.getHeight() != bounds.getHeight())
    {
        mBackgroundImage = juce::Image(juce::Image::ARGB, bounds.getWidth(), bounds.getHeight(), true);
    }
}

void MainPanel::themeChanged()
{
    // Propagate theme change to all child components
    for (int i = 0; i < getNumChildComponents(); ++i)
    {
        getChildComponent(i)->repaint();
    }

    // Force background cache update
    mBackgroundImage = juce::Image();

    repaint();
}

void MainPanel::timerCallback()
{
    // Update any animated components here
    // For now, just repaint the attractor tab if it's visible
    if (mTabComponent->getCurrentTabIndex() == 3 && mAttractorTab.get() != nullptr)
    {
        mAttractorTab->repaint();
    }
}

void MainPanel::initializeComponents()
{
    // Create tab layout first
    createTabLayout();

    // Setup each tab content
    createWaveformDisplay();
    createGrainControls();
    createModulationMatrix();
    createAttractorViz();
}

void MainPanel::createTabLayout()
{
    // Create tabs
    mTabComponent = std::make_unique<juce::TabbedComponent>(juce::TabbedButtonBar::TabsAtTop);

    // Create placeholder components for each tab
    mMainTab = std::make_unique<juce::Component>();
    mGrainTab = std::make_unique<juce::Component>();
    mModulationTab = std::make_unique<juce::Component>();
    mAttractorTab = std::make_unique<juce::Component>();

    // Add tabs
    mTabComponent->addTab("Main", mEditor.isDarkMode() ? juce::Colours::black : juce::Colours::white, mMainTab.get(), true);
    mTabComponent->addTab("Grain", mEditor.isDarkMode() ? juce::Colours::black : juce::Colours::white, mGrainTab.get(), true);
    mTabComponent->addTab("Modulation", mEditor.isDarkMode() ? juce::Colours::black : juce::Colours::white, mModulationTab.get(), true);
    mTabComponent->addTab("Attractor", mEditor.isDarkMode() ? juce::Colours::black : juce::Colours::white, mAttractorTab.get(), true);

    // Style tab bar
    mTabComponent->setTabBarDepth(30);
    mTabComponent->setOutline(0);
    mTabComponent->setIndent(0);

    addAndMakeVisible(mTabComponent.get());
}

void MainPanel::createWaveformDisplay()
{
    if (mMainTab.get() == nullptr) return;

    // Create a placeholder waveform display component
    class PlaceholderWaveformDisplay : public juce::Component
    {
    public:
        explicit PlaceholderWaveformDisplay(bool darkMode) : mDarkMode(darkMode) {}

        void paint(juce::Graphics& g) override
        {
            // Create a placeholder waveform visualization
            g.fillAll(mDarkMode ? juce::Colours::black.brighter(0.1f) : juce::Colours::white.darker(0.05f));

            // Draw waveform area outline
            g.setColour(mDarkMode ? juce::Colours::white.withAlpha(0.2f) : juce::Colours::black.withAlpha(0.2f));
            g.drawRect(getLocalBounds(), 1);

            // Draw placeholder waveform
            auto bounds = getLocalBounds().reduced(10, 20).toFloat();

            g.setColour(mDarkMode ? juce::Colours::cyan.withAlpha(0.6f) : juce::Colours::blue.withAlpha(0.6f));

            juce::Path waveformPath;
            waveformPath.startNewSubPath(bounds.getX(), bounds.getCentreY());

            // Generate a sine wave path
            const float frequency = 2.0f;
            const float amplitude = bounds.getHeight() * 0.4f;

            for (float x = 0; x < bounds.getWidth(); x += 1.0f)
            {
                float y = bounds.getCentreY() + amplitude * std::sin((x / bounds.getWidth()) * frequency * juce::MathConstants<float>::twoPi);
                waveformPath.lineTo(bounds.getX() + x, y);
            }

            g.strokePath(waveformPath, juce::PathStrokeType(2.0f));

            // Draw center line
            g.setColour(mDarkMode ? juce::Colours::white.withAlpha(0.3f) : juce::Colours::black.withAlpha(0.3f));
            g.drawHorizontalLine(juce::roundToInt(bounds.getCentreY()), bounds.getX(), bounds.getRight());

            // Draw placeholder text
            g.setColour(mDarkMode ? juce::Colours::white : juce::Colours::black);
            g.setFont(16.0f);
            g.drawText("Waveform Display (Placeholder)", getLocalBounds(),
                      juce::Justification::centred, true);
        }

    private:
        bool mDarkMode;
    };

    // Create and add the placeholder waveform display
    mWaveformDisplay = std::make_unique<PlaceholderWaveformDisplay>(mEditor.isDarkMode());
    mMainTab->addAndMakeVisible(mWaveformDisplay.get());
    mWaveformDisplay->setBounds(mMainTab->getLocalBounds().reduced(20, 20));
}

void MainPanel::createGrainControls()
{
    if (mGrainTab.get() == nullptr) return;

    // Create grain parameter controls
    const int numSliders = 6;
    const char* sliderNames[] = { "Grain Size", "Grain Density", "Grain Shape", "Randomization", "Pitch Shift", "Position" };

    // Clear existing components
    mGrainSliders.clear();
    mGrainLabels.clear();

    // Create labels and sliders for each grain parameter
    for (int i = 0; i < numSliders; ++i)
    {
        auto* slider = new juce::Slider(sliderNames[i]);
        slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
        slider->setRange(0.0, 1.0);
        slider->setValue(0.5);
        mGrainTab->addAndMakeVisible(slider);
        mGrainSliders.add(slider);

        auto* label = new juce::Label(sliderNames[i], sliderNames[i]);
        label->setJustificationType(juce::Justification::centred);
        label->attachToComponent(slider, false);
        mGrainTab->addAndMakeVisible(label);
        mGrainLabels.add(label);
    }

    // Position the sliders in a grid
    const int cols = 3;
    const int rows = (numSliders + cols - 1) / cols;

    auto bounds = mGrainTab->getLocalBounds().reduced(20, 40);
    const int sliderWidth = bounds.getWidth() / cols;
    const int sliderHeight = bounds.getHeight() / rows;

    for (int i = 0; i < numSliders; ++i)
    {
        const int row = i / cols;
        const int col = i % cols;

        juce::Rectangle<int> sliderBounds(
            bounds.getX() + col * sliderWidth,
            bounds.getY() + row * sliderHeight + 20, // Add space for label
            sliderWidth,
            sliderHeight - 20
        );

        mGrainSliders[i]->setBounds(sliderBounds.reduced(10, 10));
    }
}

void MainPanel::createModulationMatrix()
{
    if (mModulationTab.get() == nullptr) return;

    // Create a placeholder for the modulation matrix
    class PlaceholderModMatrix : public juce::Component
    {
    public:
        explicit PlaceholderModMatrix(bool darkMode) : mDarkMode(darkMode) {}

        void paint(juce::Graphics& g) override
        {
            g.fillAll(mDarkMode ? juce::Colours::black.brighter(0.1f) : juce::Colours::white.darker(0.05f));

            // Draw grid lines
            g.setColour(mDarkMode ? juce::Colours::white.withAlpha(0.2f) : juce::Colours::black.withAlpha(0.2f));

            // Vertical lines
            for (int x = 0; x < getWidth(); x += 80)
            {
                g.drawVerticalLine(x, 0, static_cast<float>(getHeight()));
            }

            // Horizontal lines
            for (int y = 0; y < getHeight(); y += 40)
            {
                g.drawHorizontalLine(y, 0, static_cast<float>(getWidth()));
            }

            // Draw placeholder text
            g.setColour(mDarkMode ? juce::Colours::white : juce::Colours::black);
            g.setFont(16.0f);
            g.drawText("Modulation Matrix (Placeholder)", getLocalBounds(),
                      juce::Justification::centred, true);
        }

    private:
        bool mDarkMode;
    };

    auto matrix = std::make_unique<PlaceholderModMatrix>(mEditor.isDarkMode());
    mModulationTab->addAndMakeVisible(matrix.get());
    matrix->setBounds(mModulationTab->getLocalBounds().reduced(20, 20));
}

void MainPanel::createAttractorViz()
{
    if (mAttractorTab.get() == nullptr) return;

    // Create a placeholder for the attractor visualization
    class PlaceholderAttractorViz : public juce::Component,
                                    private juce::Timer
    {
    public:
        explicit PlaceholderAttractorViz(bool darkMode) : mDarkMode(darkMode), mPhase(0.0f)
        {
            // Create points for Lorenz attractor visualization
            generateAttractorPoints();
            startTimer(30); // 30 fps
        }

        ~PlaceholderAttractorViz() override
        {
            stopTimer();
        }

        void paint(juce::Graphics& g) override
        {
            g.fillAll(mDarkMode ? juce::Colours::black.brighter(0.1f) : juce::Colours::white.darker(0.05f));

            // Draw placeholder attractor
            auto bounds = getLocalBounds().reduced(40, 40).toFloat();

            // Draw axes
            g.setColour(mDarkMode ? juce::Colours::white.withAlpha(0.3f) : juce::Colours::black.withAlpha(0.3f));
            g.drawLine(bounds.getX(), bounds.getCentreY(), bounds.getRight(), bounds.getCentreY(), 1.0f); // X axis
            g.drawLine(bounds.getCentreX(), bounds.getY(), bounds.getCentreX(), bounds.getBottom(), 1.0f); // Y axis

            // Draw attractor points
            g.setColour(mDarkMode ? juce::Colours::magenta.withAlpha(0.8f) : juce::Colours::purple.withAlpha(0.8f));

            // Scale factor for visualization
            float scaleFactor = bounds.getWidth() / 60.0f;

            // Draw points with a fade-out effect based on age
            const int visiblePoints = juce::jmin(static_cast<int>(mPoints.size()), 2000);
            for (int i = 0; i < visiblePoints; ++i)
            {
                // Calculate point position
                const auto& point = mPoints[(mPointIndex + i) % mPoints.size()];
                float x = bounds.getCentreX() + point.x * scaleFactor;
                float y = bounds.getCentreY() - point.y * scaleFactor;

                // Calculate alpha based on age (newer points are more opaque)
                float alpha = static_cast<float>(i) / static_cast<float>(visiblePoints);
                alpha = 1.0f - alpha;

                // Draw point
                g.setColour(mDarkMode ? juce::Colours::magenta.withAlpha(alpha) : juce::Colours::purple.withAlpha(alpha));
                float size = alpha * 4.0f; // Size also fades out
                g.fillEllipse(x - size * 0.5f, y - size * 0.5f, size, size);
            }

            // Draw explanatory text
            g.setColour(mDarkMode ? juce::Colours::white : juce::Colours::black);
            g.setFont(16.0f);
            g.drawText("Lorenz Attractor (Placeholder)", bounds.toNearestInt(),
                      juce::Justification::bottomRight, true);
        }

        void timerCallback() override
        {
            // Advance point index for animation
            mPointIndex = (mPointIndex + 1) % mPoints.size();
            repaint();
        }

    private:
        bool mDarkMode;
        float mPhase;

        struct Point3D
        {
            float x, y, z;
        };

        std::vector<Point3D> mPoints;
        size_t mPointIndex = 0;

        void generateAttractorPoints()
        {
            // Parameters for the Lorenz system
            const float sigma = 10.0f;
            const float rho = 28.0f;
            const float beta = 8.0f / 3.0f;

            // Generate a Lorenz attractor path
            Point3D point{0.1f, 0.1f, 0.1f}; // Starting point
            const float dt = 0.005f;

            // Generate 10000 points
            mPoints.clear();
            mPoints.reserve(10000);

            for (int i = 0; i < 10000; ++i)
            {
                // Lorenz attractor equations
                float dx = sigma * (point.y - point.x) * dt;
                float dy = (point.x * (rho - point.z) - point.y) * dt;
                float dz = (point.x * point.y - beta * point.z) * dt;

                point.x += dx;
                point.y += dy;
                point.z += dz;

                // Store point (only after initial transient)
                if (i > 1000)
                {
                    mPoints.push_back(point);
                }
            }

            mPointIndex = 0;
        }
    };

    auto attractor = std::make_unique<PlaceholderAttractorViz>(mEditor.isDarkMode());
    mAttractorTab->addAndMakeVisible(attractor.get());
    attractor->setBounds(mAttractorTab->getLocalBounds().reduced(20, 20));
}

} // namespace GranularPlunderphonics