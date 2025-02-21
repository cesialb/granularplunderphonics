/**
 * @file Resampler.h
 * @brief High-quality audio resampling using libsamplerate
 */

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
    explicit Resampler(ResamplerQuality quality = ResamplerQuality::Best)
        : mQuality(quality)
        , mResampler(nullptr)
        , mLogger("Resampler")
    {
        mLogger.info("Creating Resampler instance");
        initialize();
    }

    ~Resampler() {
        cleanup();
    }

    std::vector<float> process(const std::vector<float>& input,
                             double inputSampleRate,
                             double outputSampleRate) {
        if (input.empty()) {
            mLogger.warn("Empty input buffer provided to resampler");
            return std::vector<float>();
        }

        if (inputSampleRate <= 0 || outputSampleRate <= 0) {
            mLogger.error("Invalid sample rates provided");
            return std::vector<float>();
        }

        // Calculate ratio and expected output size
        double ratio = outputSampleRate / inputSampleRate;
        size_t expectedOutputSize = static_cast<size_t>(input.size() * ratio + 0.5);

        // Prepare output buffer
        std::vector<float> output(expectedOutputSize);

        // Setup SRC data structure
        SRC_DATA srcData;
        srcData.data_in = input.data();
        srcData.data_out = output.data();
        srcData.input_frames = input.size();
        srcData.output_frames = output.size();
        srcData.src_ratio = ratio;
        srcData.end_of_input = 1; // Process all input at once

        // Process the conversion
        int error = src_process(mResampler, &srcData);
        if (error) {
            mLogger.error((std::string("Resampling error: ") + src_strerror(error)).c_str());            return std::vector<float>();
        }

        // Resize output to actual frames processed
        output.resize(srcData.output_frames_gen);
        return output;
    }

    std::vector<std::vector<float>> processMultiChannel(
        const std::vector<std::vector<float>>& inputs,
        double inputSampleRate,
        double outputSampleRate) {

        if (inputs.empty()) {
            mLogger.warn("Empty input provided to multi-channel resampler");
            return std::vector<std::vector<float>>();
        }

        std::vector<std::vector<float>> outputs;
        outputs.reserve(inputs.size());

        for (const auto& channel : inputs) {
            outputs.push_back(process(channel, inputSampleRate, outputSampleRate));

            if (outputs.back().empty()) {
                mLogger.error("Failed to process channel in multi-channel resampling");
                return std::vector<std::vector<float>>();
            }
        }

        return outputs;
    }

    bool setQuality(ResamplerQuality quality) {
        if (quality == mQuality) {
            return true;
        }

        mQuality = quality;
        return initialize();
    }

    ResamplerQuality getQuality() const { return mQuality; }

    void reset() {
        if (mResampler) {
            src_reset(mResampler);
            mLogger.debug("Resampler state reset");
        }
    }

private:
    bool initialize() {
        cleanup();

        int error;
        mResampler = src_new(static_cast<int>(mQuality), 1, &error);

        if (!mResampler) {
            mLogger.error((std::string("Failed to initialize resampler: ") + src_strerror(error)).c_str());            return false;
        }

        return true;
    }

    void cleanup() {
        if (mResampler) {
            src_delete(mResampler);
            mResampler = nullptr;
        }
    }

    ResamplerQuality mQuality;
    SRC_STATE* mResampler;
    Logger mLogger;
};

} // namespace GranularPlunderphonics
