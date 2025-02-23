/**
 * @file ChaoticAttractors.h
 * @brief Collection of chaotic attractor implementations
 */

#pragma once

#include "DifferentialEquationSolver.h"
#include "LorenzAttractor.h"
#include <array>
#include <memory>
#include <vector>
#include <functional>

namespace GranularPlunderphonics {

/**
 * @brief Base class for all chaotic attractors
 */
class ChaoticAttractor {
public:
    // Virtual destructor for proper cleanup
    virtual ~ChaoticAttractor() = default;

    // Process one sample of output
    virtual float process() = 0;

    // Reset attractor to initial state
    virtual void reset() = 0;

    // Get current state vector for visualization
    virtual std::vector<float> getState() const = 0;

    // Get attractor dimension
    virtual size_t getDimension() const = 0;

    // Get pattern analysis data
    struct PatternData {
        float periodicity{0.0f};    // 0.0 = chaotic, >0.0 = periodic
        float divergence{0.0f};     // Rate of state space expansion
        float complexity{0.0f};     // Measure of system complexity
    };
    virtual PatternData analyzePattern() const = 0;
};

/**
 * @brief Implementation of a 2D torus attractor
 */
class TorusAttractor : public ChaoticAttractor {
public:
    struct Parameters {
        double a{0.5};   // First frequency ratio
        double b{0.3};   // Second frequency ratio
        double r{1.0};   // Torus radius
    };

    explicit TorusAttractor(double sampleRate)
        : mSolver(2, SolverSettings{
            .initialStepSize = 1.0 / sampleRate,
            .minStepSize = 1e-6,
            .maxStepSize = 1.0 / 100.0,
            .tolerance = 1e-6,
            .normalizationThreshold = 2.0,
            .stabilityThreshold = 10.0,
            .maxIterations = 100
        })
        , mSampleRate(sampleRate)
        , mPhase{0.0, 0.0}
    {
        reset();
    }

    void setParameters(const Parameters& params) {
        std::lock_guard<std::mutex> lock(mParamMutex);
        mParams = params;
    }

    float process() override {
        std::lock_guard<std::mutex> lock(mParamMutex);

        // Update phases
        mPhase[0] += 2.0 * M_PI * mParams.a / mSampleRate;
        mPhase[1] += 2.0 * M_PI * mParams.b / mSampleRate;

        // Keep phases in [0, 2π]
        mPhase[0] = std::fmod(mPhase[0], 2.0 * M_PI);
        mPhase[1] = std::fmod(mPhase[1], 2.0 * M_PI);

        // Calculate output
        double x = (mParams.r + std::cos(mPhase[1])) * std::cos(mPhase[0]);
        return normalizeOutput(x);
    }

    void reset() override {
        mPhase = {0.0, 0.0};
    }

    std::vector<float> getState() const override {
        double x = (mParams.r + std::cos(mPhase[1])) * std::cos(mPhase[0]);
        double y = (mParams.r + std::cos(mPhase[1])) * std::sin(mPhase[0]);
        double z = std::sin(mPhase[1]);
        return {static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)};
    }

    size_t getDimension() const override { return 2; }

    PatternData analyzePattern() const override {
        // For torus, we can determine periodicity from frequency ratios
        double ratio = mParams.a / mParams.b;
        bool rational = std::abs(ratio - std::round(ratio)) < 1e-6;

        return PatternData{
            .periodicity = rational ? 1.0f : 0.0f,
            .divergence = 0.0f,  // Torus is always bounded
            .complexity = static_cast<float>(std::abs(ratio))
        };
    }

private:
    float normalizeOutput(double x) const {
        return static_cast<float>(std::tanh(x));
    }

    DifferentialEquationSolver mSolver;
    double mSampleRate;
    Parameters mParams;
    std::array<double, 2> mPhase;
    mutable std::mutex mParamMutex;
};

/**
 * @brief Higher-dimensional attractor with configurable equations
 */
class CustomAttractor : public ChaoticAttractor {
public:
    using StateFunction = std::function<StateVector(double, const StateVector&)>;

    CustomAttractor(size_t dimension, double sampleRate)
        : mSolver(dimension, SolverSettings{
            .initialStepSize = 1.0 / sampleRate,
            .minStepSize = 1e-6,
            .maxStepSize = 1.0 / 100.0,
            .tolerance = 1e-6,
            .normalizationThreshold = 100.0,
            .stabilityThreshold = 1000.0,
            .maxIterations = 100
        })
        , mSampleRate(sampleRate)
        , mDimension(dimension)
        , mState(dimension, 0.1)  // Small non-zero initial conditions
        , mCurrentTime(0.0)
    {
    }

    void setSystemFunction(StateFunction func) {
        std::lock_guard<std::mutex> lock(mFuncMutex);
        mSystemFunc = func;
    }

    float process() override {
        if (!mSystemFunc) return 0.0f;

        std::lock_guard<std::mutex> lock(mFuncMutex);

        // Update state
        mSolver.step(mSystemFunc, mCurrentTime, mState);
        mCurrentTime += 1.0 / mSampleRate;

        // Return normalized first component
        return normalizeOutput(mState[0]);
    }

    void reset() override {
        std::fill(mState.begin(), mState.end(), 0.1);
        mCurrentTime = 0.0;
        mSolver.reset();
    }

    std::vector<float> getState() const override {
        return std::vector<float>(mState.begin(), mState.end());
    }

    size_t getDimension() const override { return mDimension; }

    PatternData analyzePattern() const override {
        // Analyze recent state history to detect patterns
        PatternData data;

        // Calculate complexity from state variance
        float variance = 0.0f;
        for (double val : mState) {
            variance += static_cast<float>(val * val);
        }
        data.complexity = std::sqrt(variance / mDimension);

        // Estimate divergence from solver statistics
        data.divergence = static_cast<float>(mSolver.getState().maxError);

        // Periodicity detection would require more state history
        data.periodicity = 0.0f;

        return data;
    }

private:
    float normalizeOutput(double x) const {
        return static_cast<float>(std::tanh(x / 10.0));
    }

    DifferentialEquationSolver mSolver;
    double mSampleRate;
    size_t mDimension;
    StateVector mState;
    double mCurrentTime;
    StateFunction mSystemFunc;
    mutable std::mutex mFuncMutex;
};

} // namespace GranularPlunderphonics