/**
 * @file AudioBuffer.h
 * @brief Audio buffer management for granular synthesis
 */

#pragma once

#include <vector>
#include <memory>
#include <stdexcept>

namespace GranularPlunderphonics {

class AudioBuffer {
public:
    /**
     * @brief Constructor
     * @param numChannels Number of audio channels
     * @param numSamples Number of samples per channel
     */
    AudioBuffer(size_t numChannels = 2, size_t numSamples = 0)
        : mNumChannels(numChannels)
        , mNumSamples(numSamples)
    {
        // Allocate buffer memory
        mBuffer.resize(mNumChannels);
        for (auto& channel : mBuffer) {
            channel.resize(mNumSamples, 0.0f);
        }
    }

    /**
     * @brief Get number of channels
     * @return Number of channels
     */
    size_t getNumChannels() const { return mNumChannels; }

    /**
     * @brief Get number of samples per channel
     * @return Number of samples
     */
    size_t getNumSamples() const { return mNumSamples; }

    /**
 * @brief Get buffer size (number of samples)
 * @return Number of samples per channel
 */
    size_t getSize() const { return getNumSamples(); }

    /**
     * @brief Read samples from buffer
     * @param channel Channel index
     * @param data Destination buffer
     * @param numSamples Number of samples to read
     * @param startPos Start position in buffer
     * @return true if successful
     */
    bool read(size_t channel, float* data, size_t numSamples, size_t startPos = 0) const {
        if (channel >= mNumChannels ||
            startPos + numSamples > mNumSamples ||
            !data) {
            return false;
            }

        std::copy(mBuffer[channel].begin() + startPos,
                  mBuffer[channel].begin() + startPos + numSamples,
                  data);
        return true;
    }


    /**
     * @brief Get sample from buffer
     * @param channel Channel index
     * @param position Sample position
     * @return Sample value
     */
    float getSample(size_t channel, size_t position) const {
        if (channel >= mNumChannels || position >= mNumSamples) {
            return 0.0f;
        }
        return mBuffer[channel][position];
    }

    /**
     * @brief Write samples to buffer
     * @param channel Channel index
     * @param data Source data
     * @param numSamples Number of samples to write
     * @param startPos Start position in buffer
     * @return true if successful
     */
    bool write(size_t channel, const float* data, size_t numSamples, size_t startPos = 0) {
        if (channel >= mNumChannels || startPos + numSamples > mNumSamples || !data) {
            return false;
        }

        std::copy(data, data + numSamples, mBuffer[channel].begin() + startPos);
        return true;
    }

    /**
     * @brief Add sample to buffer (mix)
     * @param channel Channel index
     * @param position Sample position
     * @param value Value to add
     * @return true if successful
     */
    bool addSample(size_t channel, size_t position, float value) {
        if (channel >= mNumChannels || position >= mNumSamples) {
            return false;
        }
        mBuffer[channel][position] += value;
        return true;
    }

    /**
     * @brief Clear buffer contents
     */
    void clear() {
        for (auto& channel : mBuffer) {
            std::fill(channel.begin(), channel.end(), 0.0f);
        }
    }

    /**
     * @brief Resize buffer
     * @param numChannels New number of channels
     * @param numSamples New number of samples per channel
     */
    void resize(size_t numChannels, size_t numSamples) {
        mNumChannels = numChannels;
        mNumSamples = numSamples;

        mBuffer.resize(mNumChannels);
        for (auto& channel : mBuffer) {
            channel.resize(mNumSamples, 0.0f);
        }
    }

private:
    size_t mNumChannels;
    size_t mNumSamples;
    std::vector<std::vector<float>> mBuffer;
};

} // namespace GranularPlunderphonics
