/**
 * @file AudioBuffer.cpp
 * @brief Implementation of the AudioBuffer class
 */

#include "AudioBuffer.h"
#include <algorithm>

namespace GranularPlunderphonics {

AudioBuffer::AudioBuffer(size_t numChannels, size_t initialSize)
    : mChannels(numChannels)
{
    mLogger.info(("Creating AudioBuffer with " + std::to_string(numChannels) +
                 " channels and size " + std::to_string(initialSize)).c_str());

    if (initialSize > 0) {
        for (auto& channel : mChannels) {
            channel.resize(initialSize, 0.0f);
        }
    }
}

bool AudioBuffer::write(size_t channelIndex, const float* data, size_t numSamples, size_t offset) {
    if (!isValidChannel(channelIndex)) {
        mLogger.error(("Invalid channel index: " + std::to_string(channelIndex)).c_str());
        return false;
    }

    if (!data) {
        mLogger.error("Null data pointer provided to write");
        return false;
    }

    std::lock_guard<std::mutex> lock(mMutex);

    auto& channel = mChannels[channelIndex];
    if (offset + numSamples > channel.size()) {
        mLogger.error("Write operation exceeds buffer bounds");
        return false;
    }

    std::copy(data, data + numSamples, channel.begin() + offset);
    return true;
}

bool AudioBuffer::read(size_t channelIndex, float* data, size_t numSamples, size_t offset) const {
    if (!isValidChannel(channelIndex)) {
        mLogger.error(("Invalid channel index: " + std::to_string(channelIndex)).c_str());
        return false;
    }

    if (!data) {
        mLogger.error("Null data pointer provided to read");
        return false;
    }

    std::lock_guard<std::mutex> lock(mMutex);

    const auto& channel = mChannels[channelIndex];
    if (offset + numSamples > channel.size()) {
        mLogger.error("Read operation exceeds buffer bounds");
        return false;
    }

    std::copy(channel.begin() + offset, channel.begin() + offset + numSamples, data);
    return true;
}

void AudioBuffer::resize(size_t newSize, size_t numChannels) {
    std::lock_guard<std::mutex> lock(mMutex);

    if (numChannels > 0 && numChannels != mChannels.size()) {
        mChannels.resize(numChannels);
    }

    for (auto& channel : mChannels) {
        channel.resize(newSize, 0.0f);
    }

    mLogger.info(("Buffer resized to " + std::to_string(newSize) + " samples").c_str());
}

void AudioBuffer::clear() {
    std::lock_guard<std::mutex> lock(mMutex);

    for (auto& channel : mChannels) {
        std::fill(channel.begin(), channel.end(), 0.0f);
    }

    mLogger.info("Buffer cleared");
}

size_t AudioBuffer::getSize() const {
    std::lock_guard<std::mutex> lock(mMutex);
    return mChannels.empty() ? 0 : mChannels[0].size();
}

const std::vector<float>* AudioBuffer::getChannelData(size_t channelIndex) const {
    if (!isValidChannel(channelIndex)) {
        mLogger.error(("Invalid channel index: " + std::to_string(channelIndex)).c_str());
        return nullptr;
    }

    return &mChannels[channelIndex];
}

bool AudioBuffer::isValidChannel(size_t channelIndex) const {
    return channelIndex < mChannels.size();
}

} // namespace GranularPlunderphonics
