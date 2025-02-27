#include "AttractorFactory.h"
#include "ChaoticAttractors.h"
#include "LorenzAttractor.h"
#include "../common/ErrorHandling.h"

namespace GranularPlunderphonics {

    std::unique_ptr<ChaoticAttractor> AttractorFactory::createAttractor(const std::string& type, double sampleRate) {
        if (type == "torus") {
            return std::make_unique<TorusAttractor>(sampleRate);
        }
        else if (type == "lorenz") {
            return std::make_unique<LorenzAttractor>(sampleRate);
        }

        // Instead of returning nullptr, throw an exception with detailed error information
        throw GranularPlunderphonicsException(
            ErrorCodes::kInvalidParameter,
            "Unsupported attractor type: " + type
        );
    }

} // namespace GranularPlunderphonics