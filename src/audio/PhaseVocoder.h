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
        PhaseVocoder(size_t fftSize);
        ~PhaseVocoder();

        void processFrame(const float* input, float* output,
                         size_t numSamples, float pitchShift, float timeStretch);

        // Standalone process method for simpler usage
        void process(const float* input, float* output,
                    size_t numFrames, float pitchShift, float timeStretch);

        void setSampleRate(float sampleRate);
        void setTransientThreshold(float threshold);
        void reset();

    private:
        bool detectTransients(const std::vector<float>& magnitude);
        std::vector<float> createAnalysisWindow(size_t size);

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

} // namespace GranularPlunderphonics