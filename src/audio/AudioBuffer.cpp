// AudioBuffer.cpp
#include "AudioBuffer.h"
#include <algorithm>

namespace GranularPlunderphonics {

    AudioBuffer::AudioBuffer(size_t numChannels, size_t numSamples)
        : mBuffer(numChannels)
    {
        mLogger.info(("Creating AudioBuffer with " + std::to_string(numChannels) +
                    " channels and size " + std::to_string(numSamples)).c_str());  // Added .c_str()

        for (auto& channel : mBuffer) {
            channel.resize(numSamples, 0.0f);
        }
    }

    bool AudioBuffer::write(size_t channel, const float* data, size_t numSamples, size_t startPos) {
        if (!isValidChannel(channel)) {
            mLogger.error(("Invalid channel index: " + std::to_string(channel)).c_str());  // Added .c_str()
            return false;
        }

        if (!data) {
            mLogger.error("Null data pointer provided to write");  // String literal is already const char*
            return false;
        }

        std::lock_guard<std::mutex> lock(mMutex);

        if (startPos + numSamples > mBuffer[channel].size()) {
            mLogger.error("Write operation exceeds buffer bounds");  // String literal is already const char*
            return false;
        }

        std::copy(data, data + numSamples, mBuffer[channel].begin() + startPos);
        return true;
    }

    void AudioBuffer::resize(size_t numChannels, size_t numSamples) {
        std::lock_guard<std::mutex> lock(mMutex);
        mBuffer.resize(numChannels);
        for (auto& channel : mBuffer) {
            channel.resize(numSamples, 0.0f);
        }
        mLogger.info(("Buffer resized to " + std::to_string(numSamples) + " samples").c_str());  // Added .c_str()
    }
}