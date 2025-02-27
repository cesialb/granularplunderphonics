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

    // Check if we have a valid audio file to read from
    if (!mAudioFile || !mAudioFile->isLoaded()) {
        mLogger.error("No audio file loaded for streaming");
        return false;
    }

    // Make sure position is within file bounds
    size_t fileFrames = mAudioFile->getInfo().numFrames;
    if (position >= fileFrames) {
        mLogger.warn("Stream position " + std::to_string(position) +
                    " exceeds file length " + std::to_string(fileFrames));

        // Fill with silence
        for (size_t channel = 0; channel < buffer->getNumChannels(); ++channel) {
            std::vector<float> silence(mBufferSize, 0.0f);
            if (!buffer->write(channel, silence.data(), mBufferSize, 0)) {
                mLogger.error("Failed to write silence to buffer");
                return false;
            }
        }
        return true;
    }

    // Calculate how much we can read from the file
    size_t framesToRead = std::min(mBufferSize, fileFrames - position);

    // Read data from the audio file
    std::vector<float> interleavedData(framesToRead * mAudioFile->getInfo().numChannels);
    size_t framesRead = mAudioFile->readBuffer(interleavedData.data(), framesToRead, position);

    if (framesRead < framesToRead) {
        mLogger.warn("Read fewer frames than requested: " +
                    std::to_string(framesRead) + " of " +
                    std::to_string(framesToRead));
    }

    // Handle the case where we couldn't read any frames
    if (framesRead == 0) {
        // Fill with silence
        for (size_t channel = 0; channel < buffer->getNumChannels(); ++channel) {
            std::vector<float> silence(mBufferSize, 0.0f);
            if (!buffer->write(channel, silence.data(), mBufferSize, 0)) {
                mLogger.error("Failed to write silence to buffer");
                return false;
            }
        }
        return true;
    }

    // Deinterleave the data and write to buffer
    size_t numChannels = std::min(buffer->getNumChannels(), mAudioFile->getInfo().numChannels);

    for (size_t ch = 0; ch < numChannels; ++ch) {
        std::vector<float> channelData(framesRead);

        // Deinterleave
        for (size_t i = 0; i < framesRead; ++i) {
            channelData[i] = interleavedData[i * mAudioFile->getInfo().numChannels + ch];
        }

        // Write to buffer
        if (!buffer->write(ch, channelData.data(), framesRead, 0)) {
            mLogger.error("Failed to write data to buffer for channel " + std::to_string(ch));
            return false;
        }
    }

    // If the buffer has more channels than the file, fill remaining channels with silence
    for (size_t ch = numChannels; ch < buffer->getNumChannels(); ++ch) {
        std::vector<float> silence(mBufferSize, 0.0f);
        if (!buffer->write(ch, silence.data(), mBufferSize, 0)) {
            mLogger.error("Failed to write silence to remaining channels");
            return false;
        }
    }

    // If we read fewer frames than the buffer size, fill the rest with silence
    if (framesRead < mBufferSize) {
        for (size_t ch = 0; ch < buffer->getNumChannels(); ++ch) {
            std::vector<float> silence(mBufferSize - framesRead, 0.0f);
            if (!buffer->write(ch, silence.data(), mBufferSize - framesRead, framesRead)) {
                mLogger.error("Failed to pad buffer with silence");
                return false;
            }
        }
    }

    return true;
}

} // namespace GranularPlunderphonics
