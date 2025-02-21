/**
 * @file Resampler.cpp
 * @brief Implementation of the Resampler class
 */

#include "Resampler.h"
#include <cstring>

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
        mLogger.error(("Resampling error: " + std::string(src_strerror(error))).c_str());        return std::vector<float>();
    }

    // Resize output to actual frames processed
    output.resize(srcData.output_frames_gen);
    return output;
}

std::vector<std::vector<float>> Resampler::processMultiChannel(
    const std::vector<std::vector<float>>& inputs,
    double inputSampleRate,
    double outputSampleRate)
{
    if (inputs.empty()) {
        mLogger.warn("Empty input provided to multi-channel resampler");
        return std::vector<std::vector<float>>();
    }

    // Process each channel separately
    std::vector<std::vector<float>> outputs;
    outputs.reserve(inputs.size());

    for (const auto& channel : inputs) {
        outputs.push_back(process(channel, inputSampleRate, outputSampleRate));

        // Check if processing failed
        if (outputs.back().empty()) {
            mLogger.error("Failed to process channel in multi-channel resampling");
            return std::vector<std::vector<float>>();
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
    // Clean up existing resampler if any
    cleanup();

    // Create new resampler
    int error;
    mResampler = src_new(static_cast<int>(mQuality), 1, &error);

    if (!mResampler) {
        mLogger.error(("Failed to initialize resampler: " + std::string(src_strerror(error))).c_str());
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
