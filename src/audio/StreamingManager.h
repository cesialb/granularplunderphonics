/**
 * @file StreamingManager.h
 * @brief Real-time audio streaming and buffer management system
 */

#pragma once

#include <memory>
#include <vector>
#include <mutex>
#include <atomic>
#include <queue>
#include "AudioBuffer.h"
#include "../common/Logger.h"
#include "../common/ErrorHandling.h"

namespace GranularPlunderphonics {

/**
 * @class StreamingManager
 * @brief Manages real-time audio streaming and buffer operations
 */
class StreamingManager {
public:
    /**
     * @brief Constructor
     * @param bufferSize Size of each streaming buffer in samples
     * @param numBuffers Number of buffers in the pool
     */
    StreamingManager(size_t bufferSize = 8192, size_t numBuffers = 4);

    /**
     * @brief Destructor
     */
    ~StreamingManager();

    /**
     * @brief Initialize the streaming system
     * @param sampleRate Playback sample rate
     * @return true if successful
     */
    bool initialize(double sampleRate);

    /**
     * @brief Start streaming from a position
     * @param startPosition Start position in samples
     * @return true if successful
     */
    bool startStreaming(size_t startPosition = 0);

    /**
     * @brief Stop streaming
     */
    void stopStreaming();

    /**
     * @brief Get next buffer of audio data
     * @param numSamples Number of samples needed
     * @return Shared pointer to audio buffer, or nullptr if not available
     */
    std::shared_ptr<AudioBuffer> getNextBuffer(size_t numSamples);

    /**
     * @brief Return a buffer to the pool
     * @param buffer Buffer to return
     */
    void returnBuffer(std::shared_ptr<AudioBuffer> buffer);

    /**
     * @brief Check if streaming is active
     * @return true if streaming
     */
    bool isStreaming() const { return mIsStreaming; }

    /**
     * @brief Get the current streaming position
     * @return Current position in samples
     */
    size_t getCurrentPosition() const { return mCurrentPosition; }

private:
    size_t mBufferSize;                      // Size of each buffer
    size_t mNumBuffers;                      // Number of buffers in pool
    double mSampleRate;                      // Current sample rate
    std::atomic<bool> mIsStreaming;          // Streaming state
    std::atomic<size_t> mCurrentPosition;    // Current streaming position

    // Buffer pool management
    std::queue<std::shared_ptr<AudioBuffer>> mFreeBuffers;
    std::queue<std::shared_ptr<AudioBuffer>> mFilledBuffers;
    mutable std::mutex mBufferMutex;         // Protects buffer queues

    Logger mLogger{"StreamingManager"};       // Logger instance

    /**
     * @brief Fill a buffer with audio data
     * @param buffer Buffer to fill
     * @param position Start position in samples
     * @return true if successful
     */
    bool fillBuffer(std::shared_ptr<AudioBuffer> buffer, size_t position);

    /**
     * @brief Initialize buffer pool
     * @return true if successful
     */
    bool initializeBufferPool();
};

} // namespace GranularPlunderphonics
