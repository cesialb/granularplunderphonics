// AudioBuffer.h
#pragma once

#include <vector>
#include "../common/Logger.h"

namespace GranularPlunderphonics {

    class AudioBuffer {
    public:
        AudioBuffer(size_t numChannels = 2, size_t numSamples = 0);

        // Move constructor
        AudioBuffer(AudioBuffer&& other) noexcept;

        // Move assignment operator
        AudioBuffer& operator=(AudioBuffer&& other) noexcept;

        size_t getNumChannels() const { return mBuffer.size(); }
        size_t getNumSamples() const { return mBuffer.empty() ? 0 : mBuffer[0].size(); }
        size_t getSize() const { return getNumSamples(); }

        bool read(size_t channel, float* data, size_t numSamples, size_t startPos = 0) const;
        bool write(size_t channel, const float* data, size_t numSamples, size_t startPos = 0);

        float getSample(size_t channel, size_t position) const;
        bool addSample(size_t channel, size_t position, float value);

        void clear();
        void resize(size_t numChannels, size_t numSamples);

        const std::vector<float>* getChannelData(size_t channel) const;

    private:
        bool isValidChannel(size_t channel) const { return channel < mBuffer.size(); }

        std::vector<std::vector<float>> mBuffer;
        mutable std::mutex mMutex;
        Logger mLogger{"AudioBuffer"};
    };

} // namespace GranularPlunderphonics