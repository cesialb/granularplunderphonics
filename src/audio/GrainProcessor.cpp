/**
 * @file GrainProcessor.cpp
 * @brief Implementation of advanced grain processing techniques
 */

#include "GrainProcessor.h"
#include <cmath>
#include <complex>
#include <vector>

namespace GranularPlunderphonics {

GrainProcessor::GrainProcessor(size_t fftSize)
    : mFFTSize(fftSize)
    , mHopSize(fftSize / 4)
    , mLogger("GrainProcessor")
{
    initializeFFT();
}

GrainProcessor::~GrainProcessor() {
    cleanupFFT();
}

void GrainProcessor::initializeFFT() {
    // Initialize FFTW plans and buffers
    mTimeData = reinterpret_cast<float*>(fftwf_malloc(sizeof(float) * mFFTSize));
    mFreqData = reinterpret_cast<fftwf_complex*>(
        fftwf_malloc(sizeof(fftwf_complex) * (mFFTSize/2 + 1)));
    
    mForwardPlan = fftwf_plan_dft_r2c_1d(mFFTSize, mTimeData, mFreqData, FFTW_MEASURE);
    mInversePlan = fftwf_plan_dft_c2r_1d(mFFTSize, mFreqData, mTimeData, FFTW_MEASURE);
}

void GrainProcessor::cleanupFFT() {
    if (mForwardPlan) fftwf_destroy_plan(mForwardPlan);
    if (mInversePlan) fftwf_destroy_plan(mInversePlan);
    if (mTimeData) fftwf_free(mTimeData);
    if (mFreqData) fftwf_free(mFreqData);
}

void GrainProcessor::processGrain(AudioBuffer& grain, const ProcessingParameters& params) {
    if (params.timeStretch != 1.0f || params.pitchShift != 1.0f) {
        applyPhaseVocoder(grain, params);
    }
}

void GrainProcessor::applyPhaseVocoder(AudioBuffer& grain, const ProcessingParameters& params) {
    size_t numSamples = grain.getNumSamples();
    std::vector<float> processedData(numSamples);
    
    // Create analysis/synthesis windows
    std::vector<float> window = createWindow(mFFTSize);
    
    // Initialize phase tracking
    std::vector<float> previousPhase(mFFTSize/2 + 1, 0.0f);
    std::vector<float> synthesisPhase(mFFTSize/2 + 1, 0.0f);

    // Process in overlapping frames
    for (size_t i = 0; i < numSamples; i += mHopSize) {
        // Fill input buffer and apply window
        std::fill(mTimeData, mTimeData + mFFTSize, 0.0f);
        
        for (size_t j = 0; j < mFFTSize && (i + j) < numSamples; ++j) {
            mTimeData[j] = grain.getSample(0, i + j) * window[j];
        }

        // Forward FFT
        fftwf_execute(mForwardPlan);

        // Phase vocoder processing
        processFrame(mFreqData, previousPhase, synthesisPhase, params);

        // Inverse FFT
        fftwf_execute(mInversePlan);

        // Overlap-add to output
        for (size_t j = 0; j < mFFTSize && (i + j) < numSamples; ++j) {
            processedData[i + j] += (mTimeData[j] * window[j]) / (mFFTSize * 0.5f);
        }
    }

    // Write processed data back to grain
    for (size_t i = 0; i < numSamples; ++i) {
        grain.write(0, &processedData[i], 1, i);
    }
}

void GrainProcessor::processFrame(fftwf_complex* freqData, 
                                std::vector<float>& previousPhase,
                                std::vector<float>& synthesisPhase,
                                const ProcessingParameters& params) {
    const float pi = 3.14159265358979323846f;
    const float timeScale = params.timeStretch;
    const float pitchScale = params.pitchShift;
    
    // Expected phase advancement per hop at each bin
    std::vector<float> omega(mFFTSize/2 + 1);
    for (size_t bin = 0; bin < omega.size(); ++bin) {
        omega[bin] = 2.0f * pi * bin * mHopSize / mFFTSize;
    }

    // Process each bin
    for (size_t bin = 0; bin < mFFTSize/2 + 1; ++bin) {
        // Convert to magnitude/phase
        float real = freqData[bin][0];
        float imag = freqData[bin][1];
        float magnitude = std::sqrt(real * real + imag * imag);
        float phase = std::atan2(imag, real);

        // Phase unwrapping
        float phaseDiff = phase - previousPhase[bin];
        phaseDiff = phaseDiff - 2.0f * pi * std::round(phaseDiff / (2.0f * pi));

        // True frequency
        float freq = omega[bin] + phaseDiff;

        // Apply pitch shift and time stretch
        float newPhase = synthesisPhase[bin] + freq * pitchScale * timeScale;

        // Store phases for next frame
        previousPhase[bin] = phase;
        synthesisPhase[bin] = newPhase;

        // Convert back to complex
        freqData[bin][0] = magnitude * std::cos(newPhase);
        freqData[bin][1] = magnitude * std::sin(newPhase);
    }
}

std::vector<float> GrainProcessor::createWindow(size_t size) {
    std::vector<float> window(size);
    for (size_t i = 0; i < size; ++i) {
        // Hann window
        window[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (size - 1)));
    }
    return window;
}

void GrainProcessor::applyTimeStretch(AudioBuffer& grain, float stretchFactor) {
    ProcessingParameters params;
    params.timeStretch = stretchFactor;
    params.pitchShift = 1.0f;
    processGrain(grain, params);
}

void GrainProcessor::applyPitchShift(AudioBuffer& grain, float pitchFactor) {
    ProcessingParameters params;
    params.timeStretch = 1.0f;
    params.pitchShift = pitchFactor;
    processGrain(grain, params);
}

} // namespace GranularPlunderphonics