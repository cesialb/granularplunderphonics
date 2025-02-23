// PhaseVocoder.h
#pragma once

#include <vector>
#include <complex>
#include <memory>
#include <fftw3.h>
#include "../common/Logger.h"

namespace GranularPlunderphonics {

class PhaseVocoder {
public:
    struct Settings {
        float pitchScale{1.0f};        // Pitch shift factor (1.0 = no shift)
        float timeScale{1.0f};         // Time stretch factor (1.0 = no stretch)
        float formantScale{1.0f};      // Formant preservation factor
        size_t windowSize{2048};       // FFT window size
        size_t hopSize{512};           // Analysis/synthesis hop size
        bool preserveTransients{true}; // Enable transient preservation
        float transientThreshold{0.2f};// Threshold for transient detection
    };

    struct Frame {
        std::vector<std::complex<float>> spectrum;
        std::vector<float> magnitude;
        std::vector<float> phase;
        std::vector<float> frequency;
        bool isTransient{false};
    };

    PhaseVocoder(size_t maxWindowSize = 4096);
    ~PhaseVocoder();

    // Process a block of audio with current settings
    void process(const float* input, float* output, size_t numSamples);

    // Update processing settings
    void setSettings(const Settings& newSettings);

    // Reset internal state
    void reset();

private:
    // FFT/IFFT utilities
    void performFFT(const float* input, std::complex<float>* output);
    void performIFFT(const std::complex<float>* input, float* output);

    // Phase analysis and synthesis
    void analyzePhase(Frame& frame, const Frame& prevFrame);
    void synthesizePhase(Frame& frame, const Frame& prevFrame);

    // Transient detection and handling
    bool detectTransient(const Frame& frame);
    void preserveTransients(Frame& frame, const Frame& prevFrame);

    // Formant preservation
    void preserveFormants(Frame& frame, float formantScale);

    // Internal state
    Settings mSettings;
    std::vector<Frame> mFrameBuffer;
    size_t mFrameIndex{0};

    // FFT resources
    fftwf_plan mForwardPlan;
    fftwf_plan mInversePlan;
    float* mTimeBuffer;
    std::complex<float>* mFreqBuffer;

    // Analysis/Synthesis state
    std::vector<float> mLastPhase;
    std::vector<float> mSynthPhase;
    std::vector<float> mWindow;

    // Internal utilities
    void createWindow();
    void updatePlans();

    Logger mLogger{"PhaseVocoder"};
};
}