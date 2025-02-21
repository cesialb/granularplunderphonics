/**
 * @file GrainProcessor.h
 * @brief Advanced grain processing with phase vocoder implementation
 */

#pragma once

#include <vector>
#include <memory>
#include <fftw3.h>
#include "AudioBuffer.h"
#include "../common/Logger.h"

namespace GranularPlunderphonics {

/**
 * @struct ProcessingParameters
 * @brief Parameters for grain processing operations
 */
struct ProcessingParameters {
    float timeStretch{1.0f};    // Time stretching factor
    float pitchShift{1.0f};     // Pitch shifting factor (in semitones)
    float formantShift{1.0f};   // Formant preservation factor
};

/**
 * @class GrainProcessor
 * @brief Implements advanced grain processing techniques using phase vocoder
 */
class GrainProcessor {
public:
    /**
     * @brief Constructor
     * @param fftSize Size of FFT (must be power of 2)
     */
    explicit GrainProcessor(size_t fftSize = 2048);

    /**
     * @brief Destructor
     */
    ~GrainProcessor();

    /**
     * @brief Process a grain with specified parameters
     * @param grain Grain buffer to process
     * @param params Processing parameters
     */
    void processGrain(AudioBuffer& grain, const ProcessingParameters& params);

    /**
     * @brief Apply time stretching to a grain
     * @param grain Grain buffer to process
     * @param stretchFactor Time stretching factor
     */
    void applyTimeStretch(AudioBuffer& grain, float stretchFactor);

    /**
     * @brief Apply pitch shifting to a grain
     * @param grain Grain buffer to process
     * @param pitchFactor Pitch shifting factor
     */
    void applyPitchShift(AudioBuffer& grain, float pitchFactor);

private:
    /**
     * @brief Initialize FFT resources
     */
    void initializeFFT();

    /**
     * @brief Clean up FFT resources
     */
    void cleanupFFT();

    /**
     * @brief Apply phase vocoder processing
     * @param grain Grain buffer to process
     * @param params Processing parameters
     */
    void applyPhaseVocoder(AudioBuffer& grain, const ProcessingParameters& params);

    /**
     * @brief Process a single FFT frame
     * @param freqData Frequency domain data
     * @param previousPhase Previous phase values
     * @param synthesisPhase Synthesis phase values
     * @param params Processing parameters
     */
    void processFrame(fftwf_complex* freqData,
                     std::vector<float>& previousPhase,
                     std::vector<float>& synthesisPhase,
                     const ProcessingParameters& params);

    /**
     * @brief Create window function
     * @param size Window size
     * @return Window function values
     */
    std::vector<float> createWindow(size_t size);

    // FFT resources
    size_t mFFTSize;
    size_t mHopSize;
    float* mTimeData{nullptr};
    fftwf_complex* mFreqData{nullptr};
    fftwf_plan mForwardPlan{nullptr};
    fftwf_plan mInversePlan{nullptr};

    Logger mLogger;
};

} // namespace GranularPlunderphonics