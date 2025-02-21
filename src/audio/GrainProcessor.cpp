#include "GrainProcessor.h"
#include <cmath>
#include <numbers>

namespace GranularPlunderphonics {

GrainProcessor::GrainProcessor(double sampleRate)
    : mSampleRate(sampleRate) {
    mFFTBuffer.resize(4096); // Maximum FFT size
}

void GrainProcessor::processGrain(AudioBuffer& grain,
                                const GrainProcessingConfig& config) {
    if (config.pitchShift != 1.0f) {
        applyPitchShift(grain, config.pitchShift);
    }

    if (config.timeStretch != 1.0f) {
        applyTimeStretch(grain, config.timeStretch);
    }

    if (config.stereoPosition != 0.5f) {
        applyStereoPosition(grain, config.stereoPosition);
    }
}

void GrainProcessor::applyPitchShift(AudioBuffer& grain, float pitchFactor) {
    // Simple resampling-based pitch shift for now
    size_t originalSize = grain.getNumSamples();
    size_t newSize = static_cast<size_t>(originalSize / pitchFactor);

    AudioBuffer pitched(grain.getNumChannels(), newSize);

    for (size_t ch = 0; ch < grain.getNumChannels(); ++ch) {
        for (size_t i = 0; i < newSize; ++i) {
            float pos = i * pitchFactor;
            float sample = interpolateSample(grain, ch, pos,
                InterpolationType::Cubic);
            pitched.write(ch, &sample, 1, i);
        }
    }

    grain = std::move(pitched);
}

void GrainProcessor::applyTimeStretch(AudioBuffer& grain, float stretchFactor) {
    size_t newSize = static_cast<size_t>(grain.getNumSamples() * stretchFactor);
    AudioBuffer stretched(grain.getNumChannels(), newSize);

    for (size_t ch = 0; ch < grain.getNumChannels(); ++ch) {
        for (size_t i = 0; i < newSize; ++i) {
            float pos = i / stretchFactor;
            float sample = interpolateSample(grain, ch, pos,
                InterpolationType::Cubic);
            stretched.write(ch, &sample, 1, i);
        }
    }

    grain = std::move(stretched);
}

void GrainProcessor::applyPhaseVocoder(AudioBuffer& grain, float pitchFactor) {
    const size_t fftSize = 2048;
    const size_t hopSize = fftSize / 4;

    // Initialize FFTW data structures if not already done
    if (mTimeData.size() != fftSize) {
        mTimeData.resize(fftSize);
        mFreqData.resize(fftSize * 2);  // Real and imaginary parts stored consecutively

        mForwardPlan = fftwf_plan_dft_r2c_1d(fftSize,
            mTimeData.data(),
            reinterpret_cast<fftwf_complex*>(mFreqData.data()),
            FFTW_MEASURE);

        mInversePlan = fftwf_plan_dft_c2r_1d(fftSize,
            reinterpret_cast<fftwf_complex*>(mFreqData.data()),
            mTimeData.data(),
            FFTW_MEASURE);
    }

    // Create analysis window
    std::vector<float> window(fftSize);
    for (size_t i = 0; i < fftSize; i++) {
        window[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (fftSize - 1)));
    }

    // Process each channel
    for (size_t channel = 0; channel < grain.getNumChannels(); channel++) {
        std::vector<std::complex<float>> prevPhase(fftSize/2 + 1);
        std::vector<float> outputBuffer(grain.getNumSamples(), 0.0f);

        // Process in overlapping frames
        for (size_t i = 0; i < grain.getNumSamples(); i += hopSize) {
            // Fill input buffer and apply window
            std::fill(mTimeData.begin(), mTimeData.end(), 0.0f);
            for (size_t j = 0; j < fftSize && (i + j) < grain.getNumSamples(); j++) {
                mTimeData[j] = grain.getSample(channel, i + j) * window[j];
            }

            // Forward FFT
            fftwf_execute(mForwardPlan);

            // Phase processing
            float freqPerBin = mSampleRate / fftSize;
            for (size_t bin = 0; bin <= fftSize/2; bin++) {
                float real = mFreqData[bin * 2];
                float imag = mFreqData[bin * 2 + 1];
                float magnitude = std::sqrt(real * real + imag * imag);
                float phase = std::atan2(imag, real);

                // Calculate true frequency
                float phaseDiff = phase - std::arg(prevPhase[bin]);
                if (phaseDiff > M_PI) phaseDiff -= 2.0f * M_PI;
                if (phaseDiff < -M_PI) phaseDiff += 2.0f * M_PI;

                float trueFreq = (phaseDiff * mSampleRate / (2.0f * M_PI * hopSize))
                                + bin * freqPerBin;

                // Apply pitch shift
                trueFreq *= pitchFactor;

                // Calculate new phase
                float newPhase = std::fmod(phase +
                    2.0f * M_PI * trueFreq * hopSize / mSampleRate, 2.0f * M_PI);

                // Store new complex value
                mFreqData[bin * 2] = magnitude * std::cos(newPhase);
                mFreqData[bin * 2 + 1] = magnitude * std::sin(newPhase);
                prevPhase[bin] = std::complex<float>(std::cos(newPhase), std::sin(newPhase));
            }

            // Inverse FFT
            fftwf_execute(mInversePlan);

            // Overlap-add to output
            for (size_t j = 0; j < fftSize && (i + j) < grain.getNumSamples(); j++) {
                outputBuffer[i + j] += (mTimeData[j] * window[j]) / (fftSize * 2.0f);
            }
        }

        // Write processed data back to grain
        for (size_t i = 0; i < grain.getNumSamples(); i++) {
            grain.write(channel, &outputBuffer[i], 1, i);  // Using write instead of setSample
        }
    }
}

void GrainProcessor::applyStereoPosition(AudioBuffer& grain, float position) {
    static const float PI = 3.14159265358979323846f;
    float leftGain = std::cos(position * PI * 0.5f);
    float rightGain = std::sin(position * PI * 0.5f);

    std::vector<float> monoData(grain.getNumSamples());
    grain.read(0, monoData.data(), monoData.size(), 0);

    // Create new stereo buffer
    AudioBuffer stereoGrain(2, grain.getNumSamples());

    for (size_t i = 0; i < monoData.size(); ++i) {
        float left = monoData[i] * leftGain;
        float right = monoData[i] * rightGain;
        stereoGrain.write(0, &left, 1, i);
        stereoGrain.write(1, &right, 1, i);
    }

    grain = std::move(stereoGrain);
}

float GrainProcessor::interpolateSample(const AudioBuffer& buffer,
    size_t channel, float position, InterpolationType type) {

    // Bounds checking
    if (position < 0 || position >= buffer.getNumSamples() - 1) {
        return 0.0f;
    }

    // Get integer and fractional parts of the position
    size_t pos0 = static_cast<size_t>(position);
    float frac = position - pos0;

    switch (type) {
        case InterpolationType::Linear: {
            size_t pos1 = pos0 + 1;
            float sample0 = buffer.getSample(channel, pos0);
            float sample1 = buffer.getSample(channel, pos1);
            return sample0 + frac * (sample1 - sample0);
        }

        case InterpolationType::Cubic: {
            float y0 = (pos0 > 0) ? buffer.getSample(channel, pos0 - 1) : 0.0f;
            float y1 = buffer.getSample(channel, pos0);
            float y2 = buffer.getSample(channel, pos0 + 1);
            float y3 = (pos0 + 2 < buffer.getNumSamples()) ?
                      buffer.getSample(channel, pos0 + 2) : y2;

            float a0 = y3 - y2 - y0 + y1;
            float a1 = y0 - y1 - a0;
            float a2 = y2 - y0;
            float a3 = y1;

            return a0 * frac * frac * frac +
                   a1 * frac * frac +
                   a2 * frac +
                   a3;
        }

        case InterpolationType::Sinc: {
            static const float PI = 3.14159265358979323846f;
            float sum = 0.0f;
            static const int SINC_POINTS = 4;

            for (int i = -SINC_POINTS; i <= SINC_POINTS; ++i) {
                int idx = static_cast<int>(pos0) + i;
                if (idx >= 0 && idx < static_cast<int>(buffer.getNumSamples())) {
                    float x = PI * (position - idx);
                    float sinc = (x != 0.0f) ? std::sin(x) / x : 1.0f;
                    // Apply Hann window
                    float window = 0.5f * (1.0f + std::cos(PI * i / SINC_POINTS));
                    sum += buffer.getSample(channel, idx) * sinc * window;
                }
            }
            return sum;
        }

        default:
            return buffer.getSample(channel, pos0);
    }
}

} // namespace GranularPlunderphonics
