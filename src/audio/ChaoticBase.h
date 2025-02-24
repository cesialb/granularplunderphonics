#pragma once

#include <vector>
#include <memory>

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

} // namespace GranularPlunderphonics