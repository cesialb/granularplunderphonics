// PhaseVocoder.cpp
#include "PhaseVocoder.h"
#include <cmath>
#include <algorithm>

namespace GranularPlunderphonics {

PhaseVocoder::PhaseVocoder(size_t fftSize)
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
    mLastMagnitude.resize(mFFTSize/2 + 1, 0.0f); // Initialize mLastMagnitude here
}

PhaseVocoder::~PhaseVocoder() {
    fftwf_destroy_plan(mForwardPlan);
    fftwf_destroy_plan(mInversePlan);
    fftwf_free(mTimeData);
    fftwf_free(mFreqData);
}

void PhaseVocoder::processFrame(const float* input, float* output,
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

    // Extract magnitude and phase
    for (size_t bin = 0; bin < mFFTSize/2 + 1; ++bin) {
        magnitude[bin] = std::abs(mFreqData[bin]);
        phase[bin] = std::arg(mFreqData[bin]);
    }

    bool isTransient = detectTransients(magnitude);

    for (size_t bin = 0; bin < mFFTSize/2 + 1; ++bin) {
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

bool PhaseVocoder::detectTransients(const std::vector<float>& magnitude) {
    float currentEnergy = 0.0f;
    float spectralFlux = 0.0f;

    // Make sure mLastMagnitude is initialized with the right size
    if (mLastMagnitude.size() != magnitude.size()) {
        mLastMagnitude.resize(magnitude.size(), 0.0f);
    }

    for (size_t bin = 0; bin < magnitude.size(); ++bin) {
        float mag = magnitude[bin];
        currentEnergy += mag * mag;

        if (bin < mLastMagnitude.size()) {
            float diff = mag - mLastMagnitude[bin];
            if (diff > 0) spectralFlux += diff;
        }
    }

    mLastMagnitude = magnitude;

    // Avoid division by zero
    if (currentEnergy < 1e-10f) {
        return false;
    }

    return (spectralFlux / std::sqrt(currentEnergy) > mTransientThreshold);
}

std::vector<float> PhaseVocoder::createAnalysisWindow(size_t size) {
    std::vector<float> window(size);
    // Hann window for good frequency resolution
    for (size_t i = 0; i < size; ++i) {
        window[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (size - 1)));
    }
    return window;
}

void PhaseVocoder::setSampleRate(float sampleRate) {
    if (sampleRate > 0.0f) {
        mSampleRate = sampleRate;
    }
}

void PhaseVocoder::setTransientThreshold(float threshold) {
    mTransientThreshold = std::max(0.0f, std::min(1.0f, threshold));
}

void PhaseVocoder::reset() {
    std::fill(mLastPhase.begin(), mLastPhase.end(), 0.0f);
    std::fill(mSynthPhase.begin(), mSynthPhase.end(), 0.0f);
    std::fill(mLastMagnitude.begin(), mLastMagnitude.end(), 0.0f);
}

// Standalone process method for simpler usage
void PhaseVocoder::process(const float* input, float* output,
                         size_t numFrames, float pitchShift, float timeStretch) {
    // Create temporary buffers for processing
    std::vector<float> inBuffer(mFFTSize, 0.0f);
    std::vector<float> outBuffer(mFFTSize, 0.0f);
    std::vector<float> accumBuffer(numFrames + mFFTSize, 0.0f);

    // For overlap-add processing
    for (size_t i = 0; i < numFrames; i += mHopSize) {
        // Copy input chunk to buffer with zero padding
        size_t remainingSamples = std::min(mFFTSize, numFrames - i);
        std::copy(input + i, input + i + remainingSamples, inBuffer.data());

        if (remainingSamples < mFFTSize) {
            std::fill(inBuffer.data() + remainingSamples, inBuffer.data() + mFFTSize, 0.0f);
        }

        // Process frame
        processFrame(inBuffer.data(), outBuffer.data(), mFFTSize, pitchShift, timeStretch);

        // Overlap-add to output
        for (size_t j = 0; j < mFFTSize; ++j) {
            accumBuffer[i + j] += outBuffer[j];
        }
    }

    // Copy accumulated output to output buffer
    std::copy(accumBuffer.data(), accumBuffer.data() + numFrames, output);
}

} // namespace GranularPlunderphonics