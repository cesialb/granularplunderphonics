// Resampler.h
#pragma once

#include <memory>
#include <vector>
#include <samplerate.h>
#include "../common/Logger.h"

namespace GranularPlunderphonics {

    enum class ResamplerQuality {
        Best = SRC_SINC_BEST_QUALITY,
        Medium = SRC_SINC_MEDIUM_QUALITY,
        Fastest = SRC_SINC_FASTEST,
        ZeroOrderHold = SRC_ZERO_ORDER_HOLD,
        Linear = SRC_LINEAR
    };

    class Resampler {
    public:
        explicit Resampler(ResamplerQuality quality = ResamplerQuality::Best);
        ~Resampler();

        std::vector<float> process(const std::vector<float>& input,
                                 double inputSampleRate,
                                 double outputSampleRate);

        std::vector<std::vector<float>> processMultiChannel(
            const std::vector<std::vector<float>>& inputs,
            double inputSampleRate,
            double outputSampleRate);

        bool setQuality(ResamplerQuality quality);
        ResamplerQuality getQuality() const { return mQuality; }
        void reset();

    private:
        bool initialize();
        void cleanup();

        ResamplerQuality mQuality;
        SRC_STATE* mResampler;
        Logger mLogger{"Resampler"};  // Initialize Logger with name
    };

} // namespace GranularPlunderphonics
