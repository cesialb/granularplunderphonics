#pragma once

#include "ChaoticBase.h"

namespace GranularPlunderphonics {

    class AttractorFactory {
    public:
        static std::unique_ptr<ChaoticAttractor> createAttractor(const std::string& type, double sampleRate);
    };

} // namespace GranularPlunderphonics