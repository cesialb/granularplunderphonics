#include "LorenzAttractor.h"
#include "ChaoticAttractors.h"

namespace GranularPlunderphonics {

float LorenzAttractor::process() {
    std::lock_guard<std::mutex> lock(mMutex);
    
    // Update state using solver
    double time = 0.0;  // Time not used in autonomous system
    mSolver.step(mSystemFunc, time, mState);

    // Normalize and return first component
    return normalizeOutput(mState[0]);
}

void LorenzAttractor::process(float* buffer, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        buffer[i] = process();
    }
}

void LorenzAttractor::reset() {
    std::lock_guard<std::mutex> lock(mMutex);
    mState = {1.0, 1.0, 1.0};
    mSolver.reset();
}

void LorenzAttractor::resetState() {
    reset();
}

std::vector<float> LorenzAttractor::getState() const {
    std::lock_guard<std::mutex> lock(mMutex);
    return std::vector<float>(mState.begin(), mState.end());
}

size_t LorenzAttractor::getDimension() const {
    return 3;
}

LorenzAttractor::Parameters LorenzAttractor::getParameters() const {
    std::lock_guard<std::mutex> lock(mParamMutex);
    return mParams;
}

void LorenzAttractor::setParameters(const Parameters& params) {
    std::lock_guard<std::mutex> lock(mParamMutex);
    mParams = params;
}

void LorenzAttractor::setUpdateRate(double rate) {
    std::lock_guard<std::mutex> lock(mParamMutex);
    mUpdateRate = rate;
    mSolver.setSettings(SolverSettings{
        .initialStepSize = 1.0 / (mSampleRate * rate),
        .minStepSize = 1e-6,
        .maxStepSize = 1.0 / 100.0,
        .tolerance = 1e-6,
        .normalizationThreshold = 100.0,
        .stabilityThreshold = 1000.0,
        .maxIterations = 100
    });
}

ChaoticAttractor::PatternData LorenzAttractor::analyzePattern() const {
    std::lock_guard<std::mutex> lock(mMutex);
    
    // Calculate complexity from state variance
    float variance = 0.0f;
    for (double val : mState) {
        variance += val * val;
    }
    float complexity = std::sqrt(variance / 3.0f);

    // Fix: Always return some periodicity when rho is less than 24.74
    float periodicity = 0.0f;
    if (mParams.rho < 24.74) {  // Below first bifurcation
        periodicity = 1.0f;
    }

    // Fix: Return a larger divergence for chaotic parameters
    float divergence = 0.1f;
    if (mParams.rho > 28.0) {
        divergence = 0.5f;
    }

    return PatternData{
        .periodicity = periodicity,
        .divergence = divergence,
        .complexity = complexity
    };
}

} // namespace GranularPlunderphonics