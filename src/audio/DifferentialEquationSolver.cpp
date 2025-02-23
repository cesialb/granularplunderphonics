#include "DifferentialEquationSolver.h"
#include <algorithm>

namespace GranularPlunderphonics {

DifferentialEquationSolver::DifferentialEquationSolver(size_t dimensions, 
                                                     const SolverSettings& settings)
    : mSettings(settings)
    , mDimensions(dimensions)
    , mLogger("DiffEqSolver")
{
    // Initialize solver state
    reset();

    // Allocate temporary vectors
    mK1.resize(dimensions);
    mK2.resize(dimensions);
    mK3.resize(dimensions);
    mK4.resize(dimensions);
    mTemp.resize(dimensions);

    mLogger.info(("Created solver with " + std::to_string(dimensions) + " dimensions").c_str());
}

void DifferentialEquationSolver::reset() {
    mState.stable = true;
    mState.currentStepSize = mSettings.initialStepSize;
    mState.maxError = 0.0;
    mState.stepCount = 0;
    mState.normalized = false;
}

void DifferentialEquationSolver::setSettings(const SolverSettings& settings) {
    mSettings = settings;
    reset();
}

bool DifferentialEquationSolver::step(const SystemFunction& system, 
                                    double& time, 
                                    StateVector& state) {
    if (!mState.stable || state.size() != mDimensions) {
        return false;
    }

    // Attempt integration step
    for (size_t iterations = 0; iterations < mSettings.maxIterations; ++iterations) {
        // Estimate error with current step size
        double error = estimateError(system, time, state, mState.currentStepSize);
        
        if (error > mSettings.tolerance) {
            // Reduce step size and try again
            mState.currentStepSize = adaptStepSize(mState.currentStepSize, error);
            if (mState.currentStepSize < mSettings.minStepSize) {
                mLogger.error("Step size below minimum threshold");
                mState.stable = false;
                return false;
            }
            continue;
        }

        // Compute step
        StateVector newState = rungeKutta4Step(system, time, state, mState.currentStepSize);
        
        // Check stability
        if (!checkStability(newState)) {
            mLogger.error("System instability detected");
            mState.stable = false;
            return false;
        }

        // Apply normalization if needed
        mState.normalized = normalizeState(newState);

        // Update state
        state = std::move(newState);
        time += mState.currentStepSize;
        mState.stepCount++;
        
        // Possibly increase step size for next iteration
        if (error < mSettings.tolerance / 2) {
            mState.currentStepSize = std::min(
                adaptStepSize(mState.currentStepSize, error),
                mSettings.maxStepSize
            );
        }

        return true;
    }

    mLogger.error("Maximum iterations exceeded");
    mState.stable = false;
    return false;
}

StateVector DifferentialEquationSolver::rungeKutta4Step(const SystemFunction& system,
                                                       double t,
                                                       const StateVector& y,
                                                       double h) {
    // k1 = f(t, y)
    mK1 = system(t, y);

    // k2 = f(t + h/2, y + h*k1/2)
    for (size_t i = 0; i < mDimensions; ++i) {
        mTemp[i] = y[i] + h * mK1[i] / 2;
    }
    mK2 = system(t + h/2, mTemp);

    // k3 = f(t + h/2, y + h*k2/2)
    for (size_t i = 0; i < mDimensions; ++i) {
        mTemp[i] = y[i] + h * mK2[i] / 2;
    }
    mK3 = system(t + h/2, mTemp);

    // k4 = f(t + h, y + h*k3)
    for (size_t i = 0; i < mDimensions; ++i) {
        mTemp[i] = y[i] + h * mK3[i];
    }
    mK4 = system(t + h, mTemp);

    // y[n+1] = y[n] + h*(k1 + 2k2 + 2k3 + k4)/6
    StateVector result(mDimensions);
    for (size_t i = 0; i < mDimensions; ++i) {
        result[i] = y[i] + h * (mK1[i] + 2*mK2[i] + 2*mK3[i] + mK4[i]) / 6;
    }

    return result;
}

double DifferentialEquationSolver::estimateError(const SystemFunction& system,
                                               double t,
                                               const StateVector& y,
                                               double h) {
    // Compare full step with two half steps
    StateVector fullStep = rungeKutta4Step(system, t, y, h);
    StateVector halfStep = rungeKutta4Step(system, t, y, h/2);
    halfStep = rungeKutta4Step(system, t + h/2, halfStep, h/2);

    // Compute maximum relative error
    double maxError = 0.0;
    for (size_t i = 0; i < mDimensions; ++i) {
        double error = std::abs(fullStep[i] - halfStep[i]) / 
                      (std::abs(halfStep[i]) + 1e-10);
        maxError = std::max(maxError, error);
    }

    mState.maxError = maxError;
    return maxError;
}

double DifferentialEquationSolver::adaptStepSize(double currentH, double error) {
    // Using PI controller for step size adaptation
    const double alpha = 0.7;  // Proportional term
    const double beta = 0.4;   // Integral term
    static double lastError = error;

    double factor = std::pow(1.0/error, alpha) * std::pow(1.0/lastError, beta);
    factor = std::min(std::max(factor, 0.1), 10.0);  // Limit change rate
    
    lastError = error;
    return currentH * factor;
}

bool DifferentialEquationSolver::normalizeState(StateVector& state) {
    double norm = calculateNorm(state);
    if (norm > mSettings.normalizationThreshold) {
        for (auto& val : state) {
            val *= mSettings.normalizationThreshold / norm;
        }
        return true;
    }
    return false;
}

bool DifferentialEquationSolver::checkStability(const StateVector& state) {
    for (const auto& val : state) {
        if (std::isnan(val) || std::isinf(val) || 
            std::abs(val) > mSettings.stabilityThreshold) {
            return false;
        }
    }
    return true;
}

double DifferentialEquationSolver::calculateNorm(const StateVector& v) const {
    double sumSq = 0.0;
    for (const auto& val : v) {
        sumSq += val * val;
    }
    return std::sqrt(sumSq);
}

void DifferentialEquationSolver::addScaledVector(StateVector& result,
                                               const StateVector& v,
                                               double scale) {
    for (size_t i = 0; i < mDimensions; ++i) {
        result[i] += scale * v[i];
    }
}

} // namespace GranularPlunderphonics