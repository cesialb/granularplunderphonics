/**
 * @file ResourceManager.cpp
 * @brief Implementation of resource management system
 */

#include "ResourceManager.h"

namespace GranularPlunderphonics {

// MemoryPool implementation
MemoryPool::MemoryPool(size_t blockSize, size_t maxBlocks)
    : mBlockSize(blockSize)
    , mMaxBlocks(maxBlocks)
    , mAllocatedBlocks(0)
{
    // Pre-allocate initial blocks
    mFreeBlocks.reserve(maxBlocks);
    for (size_t i = 0; i < maxBlocks / 4; ++i) {
        allocateNewBlock();
    }
}

MemoryPool::~MemoryPool() {
    std::lock_guard<std::mutex> lock(mMutex);
    for (auto* block : mFreeBlocks) {
        delete[] block;
    }
    mFreeBlocks.clear();
}

float* MemoryPool::acquireBlock() {
    std::lock_guard<std::mutex> lock(mMutex);

    if (mFreeBlocks.empty()) {
        if (mAllocatedBlocks >= mMaxBlocks) {
            return nullptr; // Pool exhausted
        }
        allocateNewBlock();
    }

    float* block = mFreeBlocks.back();
    mFreeBlocks.pop_back();
    return block;
}

void MemoryPool::releaseBlock(float* block) {
    if (!block) return;

    std::lock_guard<std::mutex> lock(mMutex);
    mFreeBlocks.push_back(block);
}

void MemoryPool::getStats(size_t& free, size_t& total) const {
    std::lock_guard<std::mutex> lock(mMutex);
    free = mFreeBlocks.size();
    total = mAllocatedBlocks;
}

void MemoryPool::allocateNewBlock() {
    float* newBlock = new float[mBlockSize];
    mFreeBlocks.push_back(newBlock);
    mAllocatedBlocks++;
}

// ResourceManager implementation
ResourceManager::ResourceManager()
    : mLogger("ResourceManager")
    , mCPULoadHistory(100, 0.0f)
    , mCurrentCPULoad(0.0f)
    , mMonitoringActive(false)
{
    mLogger.info("Creating ResourceManager");
    initializePools();
    startMonitoring();
}

ResourceManager::~ResourceManager() {
    stopMonitoring();
    cleanup();
}

std::shared_ptr<float> ResourceManager::acquireBuffer(size_t size) {
    auto pool = getOrCreatePool(size);
    if (!pool) {
        mLogger.error(("Failed to get memory pool for size " + std::to_string(size)).c_str());
        return nullptr;
    }

    float* buffer = pool->acquireBlock();
    if (!buffer) {
        mLogger.error(("Failed to acquire buffer of size " + std::to_string(size)).c_str());
        return nullptr;
    }

    return std::shared_ptr<float>(buffer,
        [this, pool](float* ptr) { pool->releaseBlock(ptr); });
}

SystemResources ResourceManager::getSystemResources() const {
    std::lock_guard<std::mutex> lock(mResourceMutex);
    return mCurrentResources;
}

float ResourceManager::getCPULoad() const {
    return mCurrentCPULoad.load();
}

bool ResourceManager::isUnderPressure() const {
    auto resources = getSystemResources();
    return resources.cpuLoad > 0.8f ||
           resources.availableMemory < 1024 * 1024 * 100; // 100MB threshold
}

void ResourceManager::initializePools() {
    mPools[1024] = std::make_unique<MemoryPool>(1024, 1000);
    mPools[2048] = std::make_unique<MemoryPool>(2048, 500);
    mPools[4096] = std::make_unique<MemoryPool>(4096, 250);
}

MemoryPool* ResourceManager::getOrCreatePool(size_t size) {
    std::lock_guard<std::mutex> lock(mPoolMutex);

    // Round up to nearest power of 2
    size_t poolSize = 1024;
    while (poolSize < size) poolSize *= 2;

    auto it = mPools.find(poolSize);
    if (it != mPools.end()) {
        return it->second.get();
    }

    // Create new pool if under resource limits
    if (mPools.size() < 10) { // Limit number of pools
        auto newPool = std::make_unique<MemoryPool>(poolSize, 100);
        auto* poolPtr = newPool.get();
        mPools[poolSize] = std::move(newPool);
        return poolPtr;
    }

    return nullptr;
}

void ResourceManager::startMonitoring() {
    mMonitoringActive = true;
    mMonitoringThread = std::thread([this]() {
        while (mMonitoringActive) {
            updateResourceStats();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
}

void ResourceManager::stopMonitoring() {
    mMonitoringActive = false;
    if (mMonitoringThread.joinable()) {
        mMonitoringThread.join();
    }
}

void ResourceManager::updateResourceStats() {
    // Update CPU load
    float currentLoad = measureCPULoad();
    mCurrentCPULoad.store(currentLoad);

    // Update resource stats
    SystemResources resources{};
    // Platform-specific code to get system resources would go here

    std::lock_guard<std::mutex> lock(mResourceMutex);
    mCurrentResources = resources;
}

float ResourceManager::measureCPULoad() {
    // Platform-specific CPU measurement would go here
    return 0.5f; // 50% load placeholder
}

void ResourceManager::cleanup() {
    std::lock_guard<std::mutex> lock(mPoolMutex);
    mPools.clear();
}

} // namespace GranularPlunderphonics