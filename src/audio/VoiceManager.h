#pragma once

#include <vector>
#include <memory>
#include <queue>
#include <atomic>
#include <chrono>
#include "../common/Logger.h"
#include "GrainCloud.h"

namespace GranularPlunderphonics {

enum class StealingStrategy {
    Oldest,
    Quietest,
    LeastImportant,
    Smart
};

struct VoiceState {
    bool active{false};
    float amplitude{0.0f};
    float importance{1.0f};
    std::chrono::steady_clock::time_point startTime;
    size_t grainCount{0};
    float cpuLoad{0.0f};
};

class VoiceManager {
public:
    VoiceManager(size_t maxVoices = 32)
        : mMaxVoices(maxVoices)
        , mActiveVoices(0)
        , mCPULimit(0.8f)
        , mStrategy(StealingStrategy::Smart)
        , mLogger("VoiceManager")
        , mPeakCPULoad(0.0f)
    {
        mVoices.resize(maxVoices);
        mVoiceStates.resize(maxVoices);
        mLogger.info(("VoiceManager initialized with " + std::to_string(maxVoices) + " voices").c_str());
    }

    size_t allocateVoice() {
        std::lock_guard<std::mutex> lock(mVoiceMutex);

        if (isUnderPressure()) {
            reduceCPULoad();
        }

        for (size_t i = 0; i < mMaxVoices; ++i) {
            if (!mVoiceStates[i].active) {
                activateVoice(i);
                return i;
            }
        }

        return stealVoice();
    }

    void releaseVoice(size_t index) {
        if (index >= mMaxVoices) return;

        std::lock_guard<std::mutex> lock(mVoiceMutex);
        deactivateVoice(index);
    }

    void updateVoiceState(size_t index, float amplitude, float importance) {
        if (index >= mMaxVoices) return;

        std::lock_guard<std::mutex> lock(mVoiceMutex);
        auto& state = mVoiceStates[index];
        state.amplitude = amplitude;
        state.importance = importance;
        state.cpuLoad = estimateVoiceCPULoad(index);
    }

    void setStealingStrategy(StealingStrategy strategy) {
        mStrategy = strategy;
    }

    void setCPULimit(float limit) {
        mCPULimit = std::clamp(limit, 0.1f, 1.0f);
    }

    struct SystemState {
        size_t activeVoices;
        float totalCPULoad;
        float peakCPULoad;
        bool isUnderPressure;
    };

    SystemState getSystemState() const {
        std::lock_guard<std::mutex> lock(mVoiceMutex);
        return {
            static_cast<size_t>(mActiveVoices),
            getCurrentCPULoad(),
            mPeakCPULoad,
            isUnderPressure()
        };
    }

private:
    size_t mMaxVoices;
    std::atomic<int> mActiveVoices;
    float mCPULimit;
    StealingStrategy mStrategy;
    std::vector<std::unique_ptr<GrainCloud>> mVoices;
    std::vector<VoiceState> mVoiceStates;
    mutable std::mutex mVoiceMutex;
    Logger mLogger;
    float mPeakCPULoad;

    void activateVoice(size_t index) {
        auto& state = mVoiceStates[index];
        state.active = true;
        state.startTime = std::chrono::steady_clock::now();
        state.grainCount = 0;
        state.cpuLoad = 0.0f;
        mActiveVoices++;

        if (!mVoices[index]) {
            mVoices[index] = std::make_unique<GrainCloud>();
        }
    }

    void deactivateVoice(size_t index) {
        auto& state = mVoiceStates[index];
        state.active = false;
        mActiveVoices--;
    }

    float calculateVoiceScore(size_t index) const {
        const auto& state = mVoiceStates[index];
        auto age = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - state.startTime).count();

        switch (mStrategy) {
            case StealingStrategy::Oldest:
                return static_cast<float>(age);
            case StealingStrategy::Quietest:
                return -state.amplitude;
            case StealingStrategy::LeastImportant:
                return -state.importance;
            case StealingStrategy::Smart:
                return (age * 0.4f) +
                       (-state.amplitude * 0.3f) +
                       (-state.importance * 0.3f);
            default:
                return 0.0f;
        }
    }

    size_t stealVoice() {
        size_t stealIndex = 0;
        float highestScore = -std::numeric_limits<float>::infinity();

        for (size_t i = 0; i < mMaxVoices; ++i) {
            if (!mVoiceStates[i].active) continue;

            float score = calculateVoiceScore(i);
            if (score > highestScore) {
                highestScore = score;
                stealIndex = i;
            }
        }

        deactivateVoice(stealIndex);
        activateVoice(stealIndex);
        return stealIndex;
    }

    bool isUnderPressure() const {
        return getCurrentCPULoad() > mCPULimit;
    }

    float calculateTotalCPULoad() {
        float total = 0.0f;
        for (const auto& state : mVoiceStates) {
            if (state.active) {
                total += state.cpuLoad;
            }
        }
        mPeakCPULoad = std::max(mPeakCPULoad, total);
        return total;
    }

    float getCurrentCPULoad() const {
        float total = 0.0f;
        for (const auto& state : mVoiceStates) {
            if (state.active) {
                total += state.cpuLoad;
            }
        }
        return total;
    }

    float estimateVoiceCPULoad(size_t index) const {
        const auto& state = mVoiceStates[index];
        return state.grainCount * 0.01f;
    }

    void reduceCPULoad() {
        std::vector<size_t> voices;
        for (size_t i = 0; i < mMaxVoices; ++i) {
            if (mVoiceStates[i].active) {
                voices.push_back(i);
            }
        }

        std::sort(voices.begin(), voices.end(),
            [this](size_t a, size_t b) {
                return mVoiceStates[a].importance < mVoiceStates[b].importance;
            });

        for (size_t index : voices) {
            if (!isUnderPressure()) break;
            deactivateVoice(index);
        }
    }
};

} // namespace GranularPlunderphonics