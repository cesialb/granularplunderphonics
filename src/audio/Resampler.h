/**
 * @file Resampler.h
 * @brief High-quality sample rate conversion using libsamplerate
 */

#pragma once

#include <vector>
#include <memory>
#include "../common/Logger.h"
#include "../common/ErrorHandling.h"
#include <samplerate.h>

namespace GranularPlunderphonics {

/**
 * @enum ResamplerQuality
 * @brief Quality settings for the resampler
 */
enum class ResamplerQuality {
    Best = SRC_SINC_BEST_QUALITY,
    Medium = SRC_SINC_MEDIUM_QUALITY,
    Fastest = SRC_SINC_FASTEST,
    ZeroOrderHold = SRC_ZERO_ORDER_HOLD,
    Linear = SRC_LINEAR
};

/**
 * @class Resampler
 * @brief Handles sample rate conversion using libsamplerate
 */
class Resampler {
public:
    /**
     * @brief Constructor
     * @param quality Resampling quality setting
     */
    explicit Resampler(ResamplerQuality quality = ResamplerQuality::Best);

    /**
     * @brief Destructor
     */
    ~Resampler();

    /**
     * @brief Process audio data with sample rate conversion
     * @param input Input audio data
     * @param inputSampleRate Input sample rate
     * @param outputSampleRate Desired output sample rate
     * @return Resampled audio data
     */
    std::vector<float> process(const std::vector<float>& input,
                             double inputSampleRate,
                             double outputSampleRate);

    /**
     * @brief Process multi-channel audio data
     * @param inputs Vector of input channels
     * @param inputSampleRate Input sample rate
     * @param outputSampleRate Desired output sample rate
     * @return Vector of resampled channels
     */
    std::vector<std::vector<float>> processMultiChannel(
        const std::vector<std::vector<float>>& inputs,
        double inputSampleRate,
        double outputSampleRate);

    /**
     * @brief Set resampling quality
     * @param quality New quality setting
     * @return true if successful
     */
    bool setQuality(ResamplerQuality quality);

    /**
     * @brief Get current quality setting
     * @return Current resampling quality
     */
    ResamplerQuality getQuality() const { return mQuality; }

    /**
     * @brief Reset the resampler state
     */
    void reset();

private:
    ResamplerQuality mQuality;
    SRC_STATE* mResampler;
    Logger mLogger{"Resampler"};

    /**
     * @brief Initialize or reinitialize the resampler
     * @return true if successful
     */
    bool initialize();

    /**
     * @brief Clean up resampler resources
     */
    void cleanup();
};

} // namespace GranularPlunderphonics
