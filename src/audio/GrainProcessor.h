// GrainProcessor.h
#pragma once
#include "GrainGenerator.h"
#include "AudioBuffer.h"
#include <fftw3.h>
#include <vector>
#include <complex>

namespace GranularPlunderphonics {

enum class InterpolationType {
    Linear,
    Cubic,
    Sinc
};

struct GrainProcessingConfig {
    float pitchShift{1.0f};          // Pitch shift factor
    float timeStretch{1.0f};         // Time stretch factor
    float stereoPosition{0.5f};      // 0.0 = left, 1.0 = right
    InterpolationType interpolation{InterpolationType::Linear};
    size_t fftSize{2048};           // For phase vocoder
};

class GrainProcessor {
public:
    explicit GrainProcessor(double sampleRate);

    void processGrain(AudioBuffer& grain, const GrainProcessingConfig& config);

private:
    fftwf_plan mForwardPlan;
    fftwf_plan mInversePlan;
    std::vector<float> mTimeData;
    std::vector<float> mFreqData;
    void applyPhaseVocoder(AudioBuffer& grain, float pitchFactor);
    void applyTimeStretch(AudioBuffer& grain, float stretchFactor);
    void applyStereoPosition(AudioBuffer& grain, float position);
    float interpolateSample(const AudioBuffer& buffer, size_t channel,
                          float position, InterpolationType type);

    std::vector<std::complex<float>> mFFTBuffer;
    double mSampleRate;
    void applyPitchShift(AudioBuffer& grain, float pitchFactor);
};

} // namespace GranularPlunderphonics
