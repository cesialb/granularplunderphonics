#pragma once

#include "ChaoticBase.h"
#include <string>
#include <memory>

namespace GranularPlunderphonics {

    class AttractorFactory {
    public:
        static std::unique_ptr<ChaoticAttractor> createAttractor(const std::string& type, double sampleRate);
    };

} // namespace GranularPlunderphonics