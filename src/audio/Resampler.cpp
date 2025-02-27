// Resampler.cpp
#include "Resampler.h"

namespace GranularPlunderphonics {

Resampler::Resampler(ResamplerQuality quality)
    : mQuality(quality)
    , mResampler(nullptr)
{
    mLogger.info("Creating Resampler instance");
    initialize();
}

Resampler::~Resampler() {
    cleanup();
}

std::vector<float> Resampler::process(
    const std::vector<float>& input,
    double inputSampleRate,
    double outputSampleRate)
{
    if (input.empty()) {
        mLogger.warn("Empty input buffer provided to resampler");
        return {};
    }

    if (inputSampleRate <= 0 || outputSampleRate <= 0) {
        mLogger.error("Invalid sample rates provided");
        return {};
    }

    // Implementation here...
    return {}; // Placeholder
}

std::vector<std::vector<float>> Resampler::processMultiChannel(
    const std::vector<std::vector<float>>& inputs,
    double inputSampleRate,
    double outputSampleRate)
{
    if (inputs.empty()) {
        mLogger.warn("Empty input provided to multi-channel resampler");
        return {};
    }

    std::vector<std::vector<float>> outputs;
    outputs.reserve(inputs.size());

    for (const auto& channel : inputs) {
        outputs.push_back(process(channel, inputSampleRate, outputSampleRate));

        if (outputs.back().empty()) {
            mLogger.error("Failed to process channel in multi-channel resampling");
            return {};
        }
    }

    return outputs;
}

bool Resampler::setQuality(ResamplerQuality quality) {
    if (quality == mQuality) {
        return true;
    }

    mQuality = quality;
    return initialize();
}

void Resampler::reset() {
    if (mResampler) {
        src_reset(mResampler);
        mLogger.debug("Resampler state reset");
    }
}

bool Resampler::initialize() {
    cleanup();

    int error;
    mResampler = src_new(static_cast<int>(mQuality), 1, &error);

    if (!mResampler) {
        mLogger.error("Failed to initialize resampler: " + std::string(src_strerror(error)));
        return false;
    }

    mLogger.info("Resampler initialized successfully");
    return true;
}

void Resampler::cleanup() {
    if (mResampler) {
        src_delete(mResampler);
        mResampler = nullptr;
        mLogger.debug("Resampler resources cleaned up");
    }
}

} // namespace GranularPlunderphonics
