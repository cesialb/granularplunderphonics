#pragma once

#include "ChaoticBase.h"
#include "DifferentialEquationSolver.h"
#include <array>
#include <mutex>

namespace GranularPlunderphonics {

    class TorusAttractor : public ChaoticAttractor {
    public:
        struct Parameters {
            double a{0.5};   // First frequency ratio
            double b{0.3};   // Second frequency ratio
            double r{1.0};   // Torus radius
        };

        explicit TorusAttractor(double sampleRate);
        float process() override;
        void reset() override;
        std::vector<float> getState() const override;
        size_t getDimension() const override { return 2; }
        PatternData analyzePattern() const override;

        void setParameters(const Parameters& params);

    private:
        float normalizeOutput(double x) const;

        DifferentialEquationSolver mSolver;
        double mSampleRate;
        Parameters mParams;
        std::array<double, 2> mPhase;
        mutable std::mutex mParamMutex;
    };

} // namespace GranularPlunderphonics