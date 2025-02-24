#include "AttractorFactory.h"
#include "ChaoticAttractors.h"
#include "LorenzAttractor.h"

namespace GranularPlunderphonics {

    std::unique_ptr<ChaoticAttractor> AttractorFactory::createAttractor(const std::string& type, double sampleRate) {
        if (type == "torus") {
            return std::make_unique<TorusAttractor>(sampleRate);
        }
        else if (type == "lorenz") {
            return std::make_unique<LorenzAttractor>(sampleRate);
        }
        return nullptr;
    }

} // namespace GranularPlunderphonics