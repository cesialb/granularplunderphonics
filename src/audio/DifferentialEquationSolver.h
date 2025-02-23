#pragma once

#include <vector>
#include <functional>
#include <cmath>
#include "../common/Logger.h"

namespace GranularPlunderphonics {

// State vector type for multi-dimensional systems
using StateVector = std::vector<double>;
// System function type (takes time and state, returns derivatives)
using SystemFunction = std::function<StateVector(double, const StateVector&)>;

struct SolverSettings {
    double initialStepSize{0.01};      // Initial integration step
    double minStepSize{1e-6};          // Minimum allowed step size
    double maxStepSize{0.1};           // Maximum allowed step size
    double tolerance{1e-6};            // Error tolerance for adaptive stepping
    double normalizationThreshold{1e3}; // Threshold for value normalization
    double stabilityThreshold{1e6};     // Threshold for divergence detection
    size_t maxIterations{1000};        // Maximum iterations before timeout
};

struct SolverState {
    bool stable{true};         // Current stability status
    double currentStepSize;    // Current adaptive step size
    double maxError{0.0};      // Maximum error in last step
    size_t stepCount{0};       // Number of steps taken
    bool normalized{false};    // Whether normalization was applied
};

class DifferentialEquationSolver {
public:
    DifferentialEquationSolver(size_t dimensions, const SolverSettings& settings = SolverSettings());

    // Run one step of the solver
    bool step(const SystemFunction& system, double& time, StateVector& state);

    // Reset solver state
    void reset();

    // Set solver parameters
    void setSettings(const SolverSettings& settings);

    // Get current solver state
    const SolverState& getState() const { return mState; }

    // Check system stability
    bool isStable() const { return mState.stable; }

private:
    // Perform single RK4 step
    StateVector rungeKutta4Step(const SystemFunction& system, double t, 
                               const StateVector& y, double h);

    // Estimate local truncation error
    double estimateError(const SystemFunction& system, double t,
                        const StateVector& y, double h);

    // Adapt step size based on error
    double adaptStepSize(double currentH, double error);

    // Normalize state vector if needed
    bool normalizeState(StateVector& state);

    // Check for instability
    bool checkStability(const StateVector& state);

    // Utilities
    double calculateNorm(const StateVector& v) const;
    void addScaledVector(StateVector& result, const StateVector& v, double scale);

    SolverSettings mSettings;
    SolverState mState;
    size_t mDimensions;
    Logger mLogger;

    // Temp vectors for RK4 calculations
    StateVector mK1, mK2, mK3, mK4, mTemp;
};

// Predefined system functions
namespace Systems {
    // Simple harmonic oscillator: d²x/dt² + x = 0
    inline SystemFunction harmonicOscillator() {
        return [](double t, const StateVector& y) -> StateVector {
            return {y[1], -y[0]};
        };
    }

    // Van der Pol oscillator: d²x/dt² - μ(1-x²)dx/dt + x = 0
    inline SystemFunction vanDerPol(double mu = 1.0) {
        return [mu](double t, const StateVector& y) -> StateVector {
            return {y[1], mu * (1.0 - y[0] * y[0]) * y[1] - y[0]};
        };
    }

    // Lorenz system: dx/dt = σ(y-x), dy/dt = x(ρ-z)-y, dz/dt = xy-βz
    inline SystemFunction lorenz(double sigma = 10.0, double rho = 28.0, double beta = 8.0/3.0) {
        return [sigma, rho, beta](double t, const StateVector& y) -> StateVector {
            return {
                sigma * (y[1] - y[0]),
                y[0] * (rho - y[2]) - y[1],
                y[0] * y[1] - beta * y[2]
            };
        };
    }

    // Rossler system: dx/dt = -(y+z), dy/dt = x+ay, dz/dt = b+z(x-c)
    inline SystemFunction rossler(double a = 0.2, double b = 0.2, double c = 5.7) {
        return [a, b, c](double t, const StateVector& y) -> StateVector {
            return {
                -(y[1] + y[2]),
                y[0] + a * y[1],
                b + y[2] * (y[0] - c)
            };
        };
    }
}

} // namespace GranularPlunderphonics