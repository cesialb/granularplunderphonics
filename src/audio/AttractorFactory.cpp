#include "AttractorFactory.h"
#include "ChaoticAttractors.h"
#include "LorenzAttractor.h"
#include "../common/ErrorHandling.h"

namespace GranularPlunderphonics {

    std::unique_ptr<ChaoticAttractor> AttractorFactory::createAttractor(const std::string& type, double sampleRate) {
        try {
            if (type == "torus") {
                return std::make_unique<TorusAttractor>(sampleRate);
            }
            else if (type == "lorenz") {
                return std::make_unique<LorenzAttractor>(sampleRate);
            }

            // Return nullptr for unsupported attractor types
            return nullptr;
        }
        catch (const std::exception& e) {
            // Log error and return nullptr
            Logger logger("AttractorFactory");
            logger.error("Failed to create attractor '" + type + "': " + e.what());
            return nullptr;
        }
    }

} // namespace GranularPlunderphonics