#pragma once

#include "ChaoticBase.h"
#include "DifferentialEquationSolver.h"
#include <cmath>

namespace GranularPlunderphonics {

class LorenzAttractor : public ChaoticAttractor {
public:
    struct Parameters {
        double rho{28.0};    // Standard value for chaotic behavior
        double beta{8.0/3.0};// Standard value
        double sigma{10.0};  // Standard value
    };

    explicit LorenzAttractor(double sampleRate)
        : mSolver(3, SolverSettings{
            .initialStepSize = 1.0 / sampleRate,
            .minStepSize = 1e-6,
            .maxStepSize = 1.0 / 100.0,
            .tolerance = 1e-6,
            .normalizationThreshold = 100.0,
            .stabilityThreshold = 1000.0,
            .maxIterations = 100
        })
        , mSampleRate(sampleRate)
        , mState{1.0, 1.0, 1.0}  // Initial conditions
        , mUpdateRate(1.0)
    {
        mSystemFunc = [this](double t, const StateVector& y) -> StateVector {
            std::lock_guard<std::mutex> lock(mParamMutex);
            return {
                mParams.sigma * (y[1] - y[0]),
                y[0] * (mParams.rho - y[2]) - y[1],
                y[0] * y[1] - mParams.beta * y[2]
            };
        };
    }

    float process() override;
    void process(float* buffer, size_t size);
    void reset() override;
    void resetState();
    std::vector<float> getState() const override;
    size_t getDimension() const override;
    Parameters getParameters() const;
    void setParameters(const Parameters& params);
    void setUpdateRate(double rate);
    PatternData analyzePattern() const override;

private:
    float normalizeOutput(double x) const {
        return static_cast<float>(std::tanh(x / 20.0));  // Scale down for audio
    }

    DifferentialEquationSolver mSolver;
    double mSampleRate;
    StateVector mState;
    SystemFunction mSystemFunc;
    Parameters mParams;
    double mUpdateRate;

    mutable std::mutex mMutex;
    mutable std::mutex mParamMutex;
};

} // namespace GranularPlunderphonics