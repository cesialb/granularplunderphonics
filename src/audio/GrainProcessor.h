#pragma once

#include <vector>
#include <complex>
#include <memory>
#include <fftw3.h>
#include "AudioBuffer.h"
#include "../common/Logger.h"

namespace GranularPlunderphonics {

enum class InterpolationType {
    Linear,
    Cubic,
    Sinc4,
    Sinc8
};

struct PhaseVocoderSettings {
    bool phaseLocking{true};       // Enable phase locking for transients
    float transientThreshold{0.2f};// Threshold for transient detection
    size_t analysisHopSize{256};   // Analysis hop size
    size_t synthesisHopSize{256};  // Synthesis hop size
    float coherenceThreshold{0.8f}; // Phase coherence threshold
    bool preserveFormants{true};   // Enable formant preservation
    float formantScale{1.0f};      // Formant scaling factor
};

    struct FFTFrame {
        std::vector<std::complex<float>> spectrum;
        std::vector<float> magnitude;
        std::vector<float> phase;
        std::vector<float> frequency;
        bool isTransient;
    };

struct ProcessingParameters {
    float timeStretch{1.0f};
    float pitchShift{1.0f};
    float formantShift{1.0f};
    InterpolationType interpolation{InterpolationType::Cubic};
    PhaseVocoderSettings vocoderSettings;
};

class PhaseVocoder {
public:
    PhaseVocoder(size_t fftSize)
        : mFFTSize(fftSize)
        , mHopSize(fftSize/4)
        , mWindow(createAnalysisWindow(fftSize))
    {
        mTimeData = fftwf_alloc_real(mFFTSize);
        mFreqData = reinterpret_cast<std::complex<float>*>(fftwf_alloc_complex(mFFTSize/2 + 1));

        mForwardPlan = fftwf_plan_dft_r2c_1d(mFFTSize, mTimeData,
            reinterpret_cast<fftwf_complex*>(mFreqData), FFTW_MEASURE);
        mInversePlan = fftwf_plan_dft_c2r_1d(mFFTSize,
            reinterpret_cast<fftwf_complex*>(mFreqData), mTimeData, FFTW_MEASURE);

        mLastPhase.resize(mFFTSize/2 + 1, 0.0f);
        mSynthPhase.resize(mFFTSize/2 + 1, 0.0f);
    }

    ~PhaseVocoder() {
        fftwf_destroy_plan(mForwardPlan);
        fftwf_destroy_plan(mInversePlan);
        fftwf_free(mTimeData);
        fftwf_free(mFreqData);
    }

    void processFrame(const float* input, float* output,
                     size_t numSamples, float pitchShift, float timeStretch) {
        // Apply analysis window and perform FFT
        for (size_t i = 0; i < mFFTSize; ++i) {
            mTimeData[i] = (i < numSamples) ? input[i] * mWindow[i] : 0.0f;
        }

        fftwf_execute(mForwardPlan);

        // Convert to polar form and analyze/modify
        std::vector<float> magnitude(mFFTSize/2 + 1);
        std::vector<float> phase(mFFTSize/2 + 1);
        std::vector<float> frequency(mFFTSize/2 + 1);

        const float pi2 = 2.0f * M_PI;
        const float freqPerBin = static_cast<float>(mSampleRate) / mFFTSize;

        bool isTransient = detectTransients(magnitude);

        for (size_t bin = 0; bin < mFFTSize/2 + 1; ++bin) {
            // Get magnitude and phase
            magnitude[bin] = std::abs(mFreqData[bin]);
            phase[bin] = std::arg(mFreqData[bin]);

            // Phase unwrapping and frequency estimation
            float phaseDiff = phase[bin] - mLastPhase[bin];
            phaseDiff = phaseDiff - pi2 * std::round(phaseDiff / pi2);

            // True frequency
            float freq = (phaseDiff * mSampleRate / mHopSize) +
                        (bin * freqPerBin);

            // Pitch shifting
            if (!isTransient) {
                mSynthPhase[bin] += (freq * pitchShift * timeStretch * mHopSize / mSampleRate);
                mSynthPhase[bin] = std::fmod(mSynthPhase[bin], pi2);
            } else {
                // Preserve transient phase relationships
                mSynthPhase[bin] = phase[bin];
            }

            // Back to rectangular form
            mFreqData[bin] = std::polar(magnitude[bin], mSynthPhase[bin]);
            mLastPhase[bin] = phase[bin];
        }

        // Inverse FFT
        fftwf_execute(mInversePlan);

        // Apply synthesis window and scale
        const float scale = 1.0f / (mFFTSize * 0.5f);
        for (size_t i = 0; i < numSamples; ++i) {
            output[i] = mTimeData[i] * mWindow[i] * scale;
        }
    }

private:
    bool detectTransients(const std::vector<float>& magnitude) {
        float currentEnergy = 0.0f;
        float spectralFlux = 0.0f;

        for (size_t bin = 0; bin < magnitude.size(); ++bin) {
            float mag = magnitude[bin];
            currentEnergy += mag * mag;

            if (bin < mLastMagnitude.size()) {
                float diff = mag - mLastMagnitude[bin];
                if (diff > 0) spectralFlux += diff;
            }
        }

        mLastMagnitude = magnitude;
        return (spectralFlux / std::sqrt(currentEnergy) > mTransientThreshold);
    }

    std::vector<float> createAnalysisWindow(size_t size) {
        std::vector<float> window(size);
        // Hann window for good frequency resolution
        for (size_t i = 0; i < size; ++i) {
            window[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (size - 1)));
        }
        return window;
    }

    size_t mFFTSize;
    size_t mHopSize;
    float mSampleRate{44100.0f};
    float mTransientThreshold{0.2f};

    std::vector<float> mWindow;
    std::vector<float> mLastPhase;
    std::vector<float> mSynthPhase;
    std::vector<float> mLastMagnitude;

    float* mTimeData;
    std::complex<float>* mFreqData;
    fftwf_plan mForwardPlan;
    fftwf_plan mInversePlan;
};

class GrainProcessor {
public:
    GrainProcessor(size_t fftSize)
        : mVocoder(std::make_unique<PhaseVocoder>(fftSize))
        , mFFTSize(fftSize)
        , mLogger("GrainProcessor")
    {
        mLogger.info(("GrainProcessor initialized with FFT size " + std::to_string(fftSize)).c_str());
    }

    void processGrain(AudioBuffer& grain, const ProcessingParameters& params) {
        if (params.timeStretch == 1.0f && params.pitchShift == 1.0f) {
            return;
        }

        // Calculate output size for time stretching
        size_t outputSize = static_cast<size_t>(grain.getNumSamples() * params.timeStretch);
        outputSize = std::max(outputSize, size_t(1));  // Ensure non-zero size

        // Create temporary buffers
        std::vector<float> inputBuffer(mFFTSize);
        std::vector<float> outputBuffer(outputSize);
        std::vector<float> overlapAdd(outputSize, 0.0f);

        // Process in overlapping frames
        size_t hopSize = mFFTSize / 4;
        size_t position = 0;

        while (position < outputSize) {
            // Get input frame
            size_t inputPos = static_cast<size_t>(position / params.timeStretch);
            for (size_t i = 0; i < mFFTSize; ++i) {
                size_t readPos = inputPos + i;
                inputBuffer[i] = (readPos < grain.getNumSamples()) ?
                    grain.getSample(0, readPos) : 0.0f;
            }

            // Process frame
            std::vector<float> processedFrame(mFFTSize);
            mVocoder->processFrame(inputBuffer.data(), processedFrame.data(),
                                 mFFTSize, params.pitchShift, params.timeStretch);

            // Overlap-add to output
            for (size_t i = 0; i < mFFTSize; ++i) {
                size_t writePos = position + i;
                if (writePos < outputSize) {
                    overlapAdd[writePos] += processedFrame[i];
                }
            }

            position += hopSize;
        }

        // Create new buffer with correct size
        AudioBuffer newGrain(grain.getNumChannels(), outputSize);

        // Write processed audio to output
        for (size_t i = 0; i < outputSize; ++i) {
            newGrain.write(0, &overlapAdd[i], 1, i);
        }

        // Copy processed audio back to input buffer
        grain = std::move(newGrain);
    }

private:
    std::unique_ptr<PhaseVocoder> mVocoder;
    size_t mFFTSize;
    Logger mLogger;
};

} // namespace GranularPlunderphonics