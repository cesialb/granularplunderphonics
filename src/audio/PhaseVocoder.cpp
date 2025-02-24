#include "PhaseVocoder.h"
#include <cstring>

namespace GranularPlunderphonics {

// PhaseVocoder implementation
PhaseVocoder::PhaseVocoder(size_t maxWindowSize)
    : mLogger("PhaseVocoder")
{
    mSettings.windowSize = std::min(maxWindowSize, size_t(4096));
    mSettings.hopSize = mSettings.windowSize / 4;

    // Allocate buffers
    mTimeBuffer = fftwf_alloc_real(mSettings.windowSize);
    mFreqBuffer = reinterpret_cast<std::complex<float>*>(
        fftwf_alloc_complex(mSettings.windowSize/2 + 1));

    // Create FFT plans
    mForwardPlan = fftwf_plan_dft_r2c_1d(
        mSettings.windowSize, mTimeBuffer,
        reinterpret_cast<fftwf_complex*>(mFreqBuffer),
        FFTW_MEASURE);

    mInversePlan = fftwf_plan_dft_c2r_1d(
        mSettings.windowSize,
        reinterpret_cast<fftwf_complex*>(mFreqBuffer),
        mTimeBuffer, FFTW_MEASURE);

    // Initialize state vectors
    size_t specSize = mSettings.windowSize/2 + 1;
    mLastPhase.resize(specSize, 0.0f);
    mSynthPhase.resize(specSize, 0.0f);

    createWindow();
    reset();

    // Convert to const char* for logger
    mLogger.info(("PhaseVocoder initialized with window size " +
                  std::to_string(mSettings.windowSize)).c_str());
}

PhaseVocoder::~PhaseVocoder() {
    fftwf_destroy_plan(mForwardPlan);
    fftwf_destroy_plan(mInversePlan);
    fftwf_free(mTimeBuffer);
    fftwf_free(mFreqBuffer);
}

void PhaseVocoder::process(const float* input, float* output, size_t numSamples) {
    const size_t hopSizePitch = static_cast<size_t>(
        mSettings.hopSize * mSettings.pitchScale);
    const size_t hopSizeTime = static_cast<size_t>(
        mSettings.hopSize * mSettings.timeScale);

    // Process frame by frame
    for (size_t i = 0; i < numSamples; i += hopSizeTime) {
        // Create analysis frame
        Frame currentFrame;
        currentFrame.spectrum.resize(mSettings.windowSize/2 + 1);
        currentFrame.magnitude.resize(mSettings.windowSize/2 + 1);
        currentFrame.phase.resize(mSettings.windowSize/2 + 1);
        currentFrame.frequency.resize(mSettings.windowSize/2 + 1);

        // Apply window and perform FFT
        for (size_t j = 0; j < mSettings.windowSize; ++j) {
            size_t inputIdx = i + j;
            mTimeBuffer[j] = (inputIdx < numSamples) ?
                input[inputIdx] * mWindow[j] : 0.0f;
        }
        performFFT(mTimeBuffer, currentFrame.spectrum.data());

        // Analyze magnitude and phase
        for (size_t bin = 0; bin < currentFrame.spectrum.size(); ++bin) {
            currentFrame.magnitude[bin] = std::abs(currentFrame.spectrum[bin]);
            currentFrame.phase[bin] = std::arg(currentFrame.spectrum[bin]);
        }

        // Detect transients
        currentFrame.isTransient = detectTransient(currentFrame);

        if (mFrameBuffer.empty()) {
            // First frame - just store it
            mFrameBuffer.push_back(currentFrame);
            continue;
        }

        // Analyze phase changes
        analyzePhase(currentFrame, mFrameBuffer.back());

        if (mSettings.preserveTransients && currentFrame.isTransient) {
            preserveTransients(currentFrame, mFrameBuffer.back());
        }

        if (std::abs(mSettings.formantScale - 1.0f) > 0.001f) {
            preserveFormants(currentFrame, mSettings.formantScale);
        }

        // Synthesize new phase
        synthesizePhase(currentFrame, mFrameBuffer.back());

        // Convert back to complex spectrum
        for (size_t bin = 0; bin < currentFrame.spectrum.size(); ++bin) {
            float mag = currentFrame.magnitude[bin];
            float phase = currentFrame.phase[bin];
            currentFrame.spectrum[bin] = std::polar(mag, phase);
        }

        // Perform IFFT and overlap-add
        performIFFT(currentFrame.spectrum.data(), mTimeBuffer);

        // Overlap-add to output
        for (size_t j = 0; j < mSettings.windowSize; ++j) {
            size_t outputIdx = i + j;
            if (outputIdx < numSamples) {
                output[outputIdx] += mTimeBuffer[j] * mWindow[j];
            }
        }

        // Store frame for next iteration
        mFrameBuffer[mFrameIndex] = currentFrame;
        mFrameIndex = (mFrameIndex + 1) % mFrameBuffer.size();
    }

    // Normalize output
    float normFactor = 1.0f / (mSettings.windowSize * 0.5f);
    for (size_t i = 0; i < numSamples; ++i) {
        output[i] *= normFactor;
    }
}

void PhaseVocoder::analyzePhase(Frame& frame, const Frame& prevFrame) {
    const float twoPi = 2.0f * M_PI;
    const float freqPerBin = 44100.0f / mSettings.windowSize;

    for (size_t bin = 0; bin < frame.phase.size(); ++bin) {
        // Calculate phase difference
        float phaseDiff = frame.phase[bin] - prevFrame.phase[bin];

        // Unwrap phase
        phaseDiff = phaseDiff - twoPi * std::round(phaseDiff / twoPi);

        // Convert to frequency
        float freq = (phaseDiff * 44100.0f / mSettings.hopSize) +
                    (bin * freqPerBin);

        frame.frequency[bin] = freq;
    }
}

void PhaseVocoder::synthesizePhase(Frame& frame, const Frame& prevFrame) {
    const float twoPi = 2.0f * M_PI;

    for (size_t bin = 0; bin < frame.phase.size(); ++bin) {
        if (frame.isTransient) {
            // Keep original phase for transients
            continue;
        }

        // Calculate new phase based on frequency
        float expectedPhase = prevFrame.phase[bin] +
            (frame.frequency[bin] * mSettings.hopSize / 44100.0f);

        // Apply pitch scaling
        float pitchScaledPhase = expectedPhase * mSettings.pitchScale;

        // Wrap to [-π, π]
        frame.phase[bin] = std::fmod(pitchScaledPhase + M_PI, twoPi) - M_PI;
    }
}

bool PhaseVocoder::detectTransient(const Frame& frame) {
    float currentEnergy = 0.0f;
    float spectralFlux = 0.0f;

    for (size_t bin = 0; bin < frame.magnitude.size(); ++bin) {
        float mag = frame.magnitude[bin];
        currentEnergy += mag * mag;

        if (!mFrameBuffer.empty()) {
            float prevMag = mFrameBuffer.back().magnitude[bin];
            float diff = mag - prevMag;
            if (diff > 0) {
                spectralFlux += diff;
            }
        }
    }

    return (spectralFlux / std::sqrt(currentEnergy) > mSettings.transientThreshold);
}

void PhaseVocoder::preserveTransients(Frame& frame, const Frame& prevFrame) {
    // For transients, preserve original phase relationships
    if (!frame.isTransient) return;

    const size_t binCount = frame.phase.size();
    std::vector<float> phaseDiff(binCount);

    // Calculate and preserve phase differences between bins
    for (size_t bin = 1; bin < binCount; ++bin) {
        phaseDiff[bin] = frame.phase[bin] - frame.phase[bin-1];
    }

    // Reapply phase differences after pitch shifting
    for (size_t bin = 1; bin < binCount; ++bin) {
        frame.phase[bin] = frame.phase[bin-1] + phaseDiff[bin];
    }
}

void PhaseVocoder::preserveFormants(Frame& frame, float formantScale) {
    const size_t binCount = frame.magnitude.size();
    std::vector<float> envelope(binCount);

    // Extract spectral envelope using simple peak detection
    for (size_t bin = 1; bin < binCount-1; ++bin) {
        if (frame.magnitude[bin] > frame.magnitude[bin-1] &&
            frame.magnitude[bin] > frame.magnitude[bin+1]) {
            envelope[bin] = frame.magnitude[bin];
        } else {
            envelope[bin] = 0.0f;
        }
    }

    // Interpolate between peaks
    float lastPeak = 0.0f;
    size_t lastPeakBin = 0;
    for (size_t bin = 0; bin < binCount; ++bin) {
        if (envelope[bin] > 0.0f) {
            // Fill in gaps between peaks
            if (lastPeakBin < bin-1) {
                float step = (envelope[bin] - lastPeak) / (bin - lastPeakBin);
                for (size_t i = lastPeakBin+1; i < bin; ++i) {
                    envelope[i] = lastPeak + step * (i - lastPeakBin);
                }
            }
            lastPeak = envelope[bin];
            lastPeakBin = bin;
        }
    }

    // Apply formant scaling
    for (size_t bin = 0; bin < binCount; ++bin) {
        size_t scaledBin = static_cast<size_t>(bin * formantScale);
        if (scaledBin < binCount) {
            frame.magnitude[bin] *= envelope[scaledBin] /
                (envelope[bin] + 1e-6f);
        }
    }
}

void PhaseVocoder::createWindow() {
    mWindow.resize(mSettings.windowSize);

    // Create Hann window
    for (size_t i = 0; i < mSettings.windowSize; ++i) {
        mWindow[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i /
                   (mSettings.windowSize - 1)));
    }
}

void PhaseVocoder::reset() {
    mFrameBuffer.clear();
    mFrameIndex = 0;
    std::fill(mLastPhase.begin(), mLastPhase.end(), 0.0f);
    std::fill(mSynthPhase.begin(), mSynthPhase.end(), 0.0f);
}

void PhaseVocoder::setSettings(const Settings& newSettings) {
    bool needsReset = (newSettings.windowSize != mSettings.windowSize);
    mSettings = newSettings;

    if (needsReset) {
        updatePlans();
        createWindow();
        reset();
    }
}

void PhaseVocoder::updatePlans() {
    // Cleanup old plans
    fftwf_destroy_plan(mForwardPlan);
    fftwf_destroy_plan(mInversePlan);
    fftwf_free(mTimeBuffer);
    fftwf_free(mFreqBuffer);

    // Create new buffers and plans
    mTimeBuffer = fftwf_alloc_real(mSettings.windowSize);
    mFreqBuffer = reinterpret_cast<std::complex<float>*>(
        fftwf_alloc_complex(mSettings.windowSize/2 + 1));

    // Create new FFT plans
    mForwardPlan = fftwf_plan_dft_r2c_1d(
        mSettings.windowSize, mTimeBuffer,
        reinterpret_cast<fftwf_complex*>(mFreqBuffer),
        FFTW_MEASURE);

    mInversePlan = fftwf_plan_dft_c2r_1d(
        mSettings.windowSize,
        reinterpret_cast<fftwf_complex*>(mFreqBuffer),
        mTimeBuffer, FFTW_MEASURE);

    // Resize state vectors
    size_t specSize = mSettings.windowSize/2 + 1;
    mLastPhase.resize(specSize, 0.0f);
    mSynthPhase.resize(specSize, 0.0f);
}

void PhaseVocoder::performFFT(const float* input, std::complex<float>* output) {
    // Copy input to FFT buffer
    std::memcpy(mTimeBuffer, input, mSettings.windowSize * sizeof(float));

    // Perform FFT
    fftwf_execute(mForwardPlan);

    // Copy to output buffer
    std::memcpy(output, mFreqBuffer,
                (mSettings.windowSize/2 + 1) * sizeof(std::complex<float>));
}

void PhaseVocoder::performIFFT(const std::complex<float>* input, float* output) {
    // Copy input to FFT buffer
    std::memcpy(mFreqBuffer, input,
                (mSettings.windowSize/2 + 1) * sizeof(std::complex<float>));

    // Perform IFFT
    fftwf_execute(mInversePlan);

    // Copy to output buffer
    std::memcpy(output, mTimeBuffer, mSettings.windowSize * sizeof(float));
}

} // namespace GranularPlunderphonics