#include <cmath>
#include <vector>
#include <memory>
#include <algorithm>
#include <cstring>
#include "AudioBuffer.h"
#include <fftw3.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace GranularPlunderphonics {

// Forward declarations
class AudioBuffer;

enum class InterpolationType {
    Linear,
    Cubic,
    Sinc4,
    Sinc8
};

struct PhaseVocoderSettings {
    bool phaseLocking{true};      // Enable phase locking for transients
    float transientThreshold{0.2f}; // Threshold for transient detection
    std::size_t analysisHopSize{256};   // Analysis hop size
    std::size_t synthesisHopSize{256};  // Synthesis hop size
    float coherenceThreshold{0.8f}; // Phase coherence threshold
};

struct ProcessingParameters {
    float timeStretch{1.0f};
    float pitchShift{1.0f};
    float formantShift{1.0f};
    InterpolationType interpolation{InterpolationType::Cubic};
    PhaseVocoderSettings vocoderSettings;
};

class GrainProcessor {
public:
    GrainProcessor(std::size_t fftSize)
        : mFFTSize(fftSize)
        , mPreviousMagnitudes(fftSize/2 + 1)
        , mIsFirstFrame(true) {
        initializeFFT();
    }

    ~GrainProcessor() {
        cleanupFFT();
    }

    void processGrain(AudioBuffer& grain, const ProcessingParameters& params);

private:
    std::size_t mFFTSize;
    std::vector<float> mPreviousMagnitudes;
    bool mIsFirstFrame;
    float* mTimeData{nullptr};
    fftwf_complex* mFreqData{nullptr};
    fftwf_plan mForwardPlan{nullptr};
    fftwf_plan mInversePlan{nullptr};

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

    float sincInterpolate(const float* buffer, std::size_t size, float position, int points) {
        float sum = 0.0f;
        int start = static_cast<int>(std::floor(position)) - points/2;

        for (int i = 0; i < points; ++i) {
            int idx = start + i;
            if (idx >= 0 && idx < static_cast<int>(size)) {
                float x = static_cast<float>(M_PI) * (position - idx);
                if (std::abs(x) > 1e-6f) {
                    sum += buffer[idx] * std::sin(x) / x;
                } else {
                    sum += buffer[idx];
                }
            }
        }
        return sum;
    }

    float interpolateSample(const float* buffer, std::size_t size, float position,
                          InterpolationType type);

    void processPhaseVocoder(fftwf_complex* freqData,
                            std::vector<float>& previousPhase,
                            std::vector<float>& synthesisPhase,
                            const ProcessingParameters& params);

    void detectTransients(fftwf_complex* freqData,
                         std::size_t binCount,
                         float threshold,
                         std::vector<bool>& isLockedPhase);

    std::vector<float> createWindow(std::size_t size);
};

} // namespace GranularPlunderphonics