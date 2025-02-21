/**
 * @file StreamingManager.cpp
 * @brief Implementation of real-time audio streaming and buffer management system
 */

#include "StreamingManager.h"

namespace GranularPlunderphonics {

StreamingManager::StreamingManager(size_t bufferSize, size_t numBuffers)
    : mBufferSize(bufferSize)
    , mNumBuffers(numBuffers)
    , mSampleRate(44100.0)
    , mIsStreaming(false)
    , mCurrentPosition(0)
    , mLogger("StreamingManager")
{
    mLogger.info(("Creating StreamingManager with buffer size " +
                  std::to_string(bufferSize) + " and " +
                  std::to_string(numBuffers) + " buffers").c_str());
    initializeBufferPool();
}

StreamingManager::~StreamingManager() {
    stopStreaming();
    mLogger.info("StreamingManager destroyed");
}

bool StreamingManager::initialize(double sampleRate) {
    std::lock_guard<std::mutex> lock(mBufferMutex);

    mSampleRate = sampleRate;
    mLogger.info(("Initialized StreamingManager with sample rate " +
                  std::to_string(sampleRate)).c_str());

    return true;
}

bool StreamingManager::startStreaming(size_t startPosition) {
    std::lock_guard<std::mutex> lock(mBufferMutex);

    if (mIsStreaming) {
        mLogger.warn("Streaming already in progress");
        return false;
    }

    mCurrentPosition = startPosition;
    mIsStreaming = true;

    // Pre-fill some buffers
    for (size_t i = 0; i < mNumBuffers / 2; ++i) {
        if (!mFreeBuffers.empty()) {
            auto buffer = mFreeBuffers.front();
            mFreeBuffers.pop();

            if (fillBuffer(buffer, mCurrentPosition)) {
                mFilledBuffers.push(buffer);
                mCurrentPosition += mBufferSize;
            }
        }
    }

    mLogger.info(("Started streaming from position " +
                  std::to_string(startPosition)).c_str());
    return true;
}

void StreamingManager::stopStreaming() {
    std::lock_guard<std::mutex> lock(mBufferMutex);

    mIsStreaming = false;

    // Return all filled buffers to free pool
    while (!mFilledBuffers.empty()) {
        mFreeBuffers.push(mFilledBuffers.front());
        mFilledBuffers.pop();
    }

    mLogger.info("Streaming stopped");
}

std::shared_ptr<AudioBuffer> StreamingManager::getNextBuffer(size_t numSamples) {
    std::lock_guard<std::mutex> lock(mBufferMutex);

    if (!mIsStreaming || mFilledBuffers.empty()) {
        return nullptr;
    }

    // Get next filled buffer
    auto buffer = mFilledBuffers.front();
    mFilledBuffers.pop();

    // Start filling a new buffer if we have free ones
    if (!mFreeBuffers.empty()) {
        auto newBuffer = mFreeBuffers.front();
        mFreeBuffers.pop();

        if (fillBuffer(newBuffer, mCurrentPosition)) {
            mFilledBuffers.push(newBuffer);
            mCurrentPosition += mBufferSize;
        } else {
            mFreeBuffers.push(newBuffer);
        }
    }

    return buffer;
}

void StreamingManager::returnBuffer(std::shared_ptr<AudioBuffer> buffer) {
    if (!buffer) {
        return;
    }

    std::lock_guard<std::mutex> lock(mBufferMutex);
    mFreeBuffers.push(buffer);
}

bool StreamingManager::initializeBufferPool() {
    std::lock_guard<std::mutex> lock(mBufferMutex);

    // Clear existing buffers
    while (!mFreeBuffers.empty()) mFreeBuffers.pop();
    while (!mFilledBuffers.empty()) mFilledBuffers.pop();

    // Create new buffer pool
    for (size_t i = 0; i < mNumBuffers; ++i) {
        auto buffer = std::make_shared<AudioBuffer>(2, mBufferSize); // Stereo buffers
        if (!buffer) {
            mLogger.error("Failed to create buffer in pool");
            return false;
        }
        mFreeBuffers.push(buffer);
    }

    mLogger.info(("Buffer pool initialized with " +
                  std::to_string(mNumBuffers) + " buffers").c_str());
    return true;
}

bool StreamingManager::fillBuffer(std::shared_ptr<AudioBuffer> buffer, size_t position) {
    if (!buffer) {
        mLogger.error("Null buffer provided to fillBuffer");
        return false;
    }

    // This is where we'll implement the actual audio file reading
    // For now, just fill with silence
    for (size_t channel = 0; channel < buffer->getNumChannels(); ++channel) {
        std::vector<float> silence(mBufferSize, 0.0f);
        if (!buffer->write(channel, silence.data(), mBufferSize, 0)) {
            mLogger.error("Failed to write silence to buffer");
            return false;
        }
    }

    return true;
}

} // namespace GranularPlunderphonics
