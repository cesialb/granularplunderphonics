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
        , mTimeData(nullptr)
        , mFreqData(nullptr)
        , mForwardPlan(nullptr)
        , mInversePlan(nullptr)
        , mPreviousMagnitudes(fftSize/2 + 1)
        , mPreviousPhase(fftSize/2 + 1)
        , mSynthesisPhase(fftSize/2 + 1)
        , mLogger("PhaseVocoder")
    {
        initializeFFT();
    }

    ~PhaseVocoder() {
        cleanupFFT();
    }

    void processFrame(const float* input, float* output, size_t numSamples, const ProcessingParameters& params) {
        // Apply window function and perform FFT
        for (size_t i = 0; i < mFFTSize; ++i) {
            float windowVal = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (mFFTSize - 1)));
            mTimeData[i] = (i < numSamples) ? input[i] * windowVal : 0.0f;
        }

        fftwf_execute(mForwardPlan);

        // Process in frequency domain
        processSpectrum(params);

        // Inverse FFT and apply window
        fftwf_execute(mInversePlan);

        // Copy to output with normalization
        float normFactor = 1.0f / (mFFTSize * 0.5f);
        for (size_t i = 0; i < numSamples; ++i) {
            float windowVal = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (mFFTSize - 1)));
            output[i] = mTimeData[i] * windowVal * normFactor;
        }
    }

private:
    void initializeFFT() {
        mTimeData = reinterpret_cast<float*>(fftwf_malloc(sizeof(float) * mFFTSize));
        mFreqData = reinterpret_cast<fftwf_complex*>(
            fftwf_malloc(sizeof(fftwf_complex) * (mFFTSize/2 + 1)));

        mForwardPlan = fftwf_plan_dft_r2c_1d(mFFTSize, mTimeData, mFreqData, FFTW_MEASURE);
        mInversePlan = fftwf_plan_dft_c2r_1d(mFFTSize, mFreqData, mTimeData, FFTW_MEASURE);
    }

    void cleanupFFT() {
        if (mForwardPlan) fftwf_destroy_plan(mForwardPlan);
        if (mInversePlan) fftwf_destroy_plan(mInversePlan);
        if (mTimeData) fftwf_free(mTimeData);
        if (mFreqData) fftwf_free(mFreqData);
    }

    void processSpectrum(const ProcessingParameters& params) {
        const size_t binCount = mFFTSize/2 + 1;
        const float pi2 = 2.0f * M_PI;

        std::vector<bool> isLockedPhase(binCount, false);
        if (params.vocoderSettings.phaseLocking) {
            detectTransients(binCount, params.vocoderSettings.transientThreshold, isLockedPhase);
        }

        for (size_t bin = 0; bin < binCount; ++bin) {
            // Calculate magnitude and phase
            float real = mFreqData[bin][0];
            float imag = mFreqData[bin][1];
            float magnitude = std::sqrt(real * real + imag * imag);
            float phase = std::atan2(imag, real);

            // Phase processing
            float phaseDiff = phase - mPreviousPhase[bin];
            phaseDiff = phaseDiff - pi2 * std::round(phaseDiff / pi2);

            float omega = pi2 * bin * params.vocoderSettings.analysisHopSize / mFFTSize;
            float trueFreq = omega + phaseDiff;

            // Phase synthesis
            float newPhase;
            if (isLockedPhase[bin]) {
                newPhase = phase;
            } else {
                newPhase = mSynthesisPhase[bin] +
                          trueFreq * params.pitchShift * params.timeStretch;
            }

            // Formant preservation
            if (params.vocoderSettings.preserveFormants) {
                float formantShift = params.formantShift;
                size_t formantBin = static_cast<size_t>(bin / formantShift);
                if (formantBin < binCount) {
                    magnitude *= mPreviousMagnitudes[formantBin] /
                               (mPreviousMagnitudes[bin] + 1e-6f);
                }
            }

            // Store current values for next frame
            mPreviousMagnitudes[bin] = magnitude;
            mPreviousPhase[bin] = phase;
            mSynthesisPhase[bin] = newPhase;

            // Convert back to rectangular form
            mFreqData[bin][0] = magnitude * std::cos(newPhase);
            mFreqData[bin][1] = magnitude * std::sin(newPhase);
        }
    }

    void detectTransients(size_t binCount, float threshold, std::vector<bool>& isLockedPhase) {
        float flux = 0.0f;
        for (size_t bin = 0; bin < binCount; ++bin) {
            float magnitude = std::sqrt(mFreqData[bin][0] * mFreqData[bin][0] +
                                      mFreqData[bin][1] * mFreqData[bin][1]);
            float diff = magnitude - mPreviousMagnitudes[bin];
            if (diff > 0) {
                flux += diff;
            }
        }

        if (flux > threshold) {
            std::fill(isLockedPhase.begin(), isLockedPhase.end(), true);
        }
    }

    size_t mFFTSize;
    float* mTimeData;
    fftwf_complex* mFreqData;
    fftwf_plan mForwardPlan;
    fftwf_plan mInversePlan;
    std::vector<float> mPreviousMagnitudes;
    std::vector<float> mPreviousPhase;
    std::vector<float> mSynthesisPhase;
    Logger mLogger;
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

        // Process grain using phase vocoder
        std::vector<float> inputBuffer(mFFTSize);
        std::vector<float> outputBuffer(grain.getNumSamples());

        // Process in overlapping frames
        size_t hopSize = mFFTSize / 4;
        for (size_t i = 0; i < grain.getNumSamples(); i += hopSize) {
            // Copy grain data to input buffer
            for (size_t j = 0; j < mFFTSize; ++j) {
                size_t grainPos = i + j;
                inputBuffer[j] = (grainPos < grain.getNumSamples()) ?
                    grain.getSample(0, grainPos) : 0.0f;
            }

            // Process frame
            std::vector<float> frameOutput(mFFTSize);
            mVocoder->processFrame(inputBuffer.data(), frameOutput.data(), mFFTSize, params);

            // Overlap-add to output
            for (size_t j = 0; j < mFFTSize; ++j) {
                size_t outputPos = i + j;
                if (outputPos < grain.getNumSamples()) {
                    outputBuffer[outputPos] += frameOutput[j];
                }
            }
        }

        // Write processed data back to grain
        for (size_t i = 0; i < grain.getNumSamples(); ++i) {
            grain.write(0, &outputBuffer[i], 1, i);
        }
    }

private:
    std::unique_ptr<PhaseVocoder> mVocoder;
    size_t mFFTSize;
    Logger mLogger;
};

} // namespace GranularPlunderphonics