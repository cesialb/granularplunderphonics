/**
 * @file ResourceManager.h
 * @brief Resource management system for memory and CPU optimization
 */

#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <unordered_map>
#include "Logger.h"

namespace GranularPlunderphonics {

/**
 * @struct SystemResources
 * @brief Represents available system resources
 */
struct SystemResources {
    size_t totalMemory;    // Total system memory in bytes
    size_t availableMemory;// Available memory in bytes
    int numCPUCores;      // Number of CPU cores
    float cpuLoad;        // Current CPU load (0-1)
};

/**
 * @class MemoryPool
 * @brief Pool for efficient buffer allocation
 */
class MemoryPool {
public:
    MemoryPool(size_t blockSize, size_t maxBlocks);
    ~MemoryPool();

    float* acquireBlock();
    void releaseBlock(float* block);
    void getStats(size_t& free, size_t& total) const;

private:
    void allocateNewBlock();

    size_t mBlockSize;
    size_t mMaxBlocks;
    std::atomic<size_t> mAllocatedBlocks;
    std::vector<float*> mFreeBlocks;
    mutable std::mutex mMutex;
};

/**
 * @class ResourceManager
 * @brief Central resource management system
 */
class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();

    // Delete copy constructor and assignment operator
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    std::shared_ptr<float> acquireBuffer(size_t size);
    SystemResources getSystemResources() const;
    float getCPULoad() const;
    bool isUnderPressure() const;

private:
    void initializePools();
    MemoryPool* getOrCreatePool(size_t size);
    void startMonitoring();
    void stopMonitoring();

    size_t getTotalSystemMemory();

    size_t getAvailableSystemMemory();

    int getNumberOfCPUCores();

    float getCurrentCPULoad();

    void updateResourceStats();
    float measureCPULoad();
    void cleanup();

    Logger mLogger;
    std::unordered_map<size_t, std::unique_ptr<MemoryPool>> mPools;
    std::vector<float> mCPULoadHistory;
    std::atomic<float> mCurrentCPULoad;
    SystemResources mCurrentResources;
    std::thread mMonitoringThread;
    std::atomic<bool> mMonitoringActive;
    mutable std::mutex mPoolMutex;
    mutable std::mutex mResourceMutex;
};

} // namespace GranularPlunderphonics