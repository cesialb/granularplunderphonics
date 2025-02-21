/**
 * @file GrainGenerator.cpp
 * @brief Implementation of grain generation functionality
 */

#include "GrainGenerator.h"
#include <cmath>
#include <algorithm>

namespace GranularPlunderphonics {

GrainGenerator::GrainGenerator(double sampleRate)
    : mSampleRate(sampleRate)
    , mLogger("GrainGenerator")
{
    std::string msg = "Creating GrainGenerator instance with sample rate " +
                     std::to_string(sampleRate);
    mLogger.info(msg.c_str());
}

std::shared_ptr<AudioBuffer> GrainGenerator::generateGrain(
    const AudioBuffer& source, const GrainConfig& config)
{
    if (config.duration == 0) {
        mLogger.error("Invalid grain duration: 0");
        return nullptr;
    }

    // Create grain buffer
    auto grain = std::make_shared<AudioBuffer>(
        source.getNumChannels(),
        config.duration
    );

    // Get window function
    auto window = getWindow(config.shape, config.duration);
    if (window.empty()) {
        mLogger.error("Failed to get window function");
        return nullptr;
    }

    // Calculate sample read positions
    float positionIncrement = config.reverse ? -1.0f : 1.0f;
    if (config.pitchShift != 1.0f) {
        positionIncrement /= config.pitchShift;
    }

    // Extract samples for each channel
    for (size_t channel = 0; channel < source.getNumChannels(); ++channel) {
        std::vector<float> grainData(config.duration);
        float readPosition = config.reverse ?
            config.position + config.duration - 1 :
            config.position;

        // Extract and interpolate samples
        for (size_t i = 0; i < config.duration; ++i) {
            float sample = interpolateSample(source, channel, readPosition);
            grainData[i] = sample * window[i] * config.amplitude;
            readPosition += positionIncrement;
        }

        // Write to grain buffer
        grain->write(channel, grainData.data(), config.duration, 0);
    }

    return grain;
}

void GrainGenerator::precalculateWindows(size_t minSize, size_t maxSize) {
    std::string msg = "Precalculating windows from " + std::to_string(minSize) +
                     " to " + std::to_string(maxSize) + " samples";
    mLogger.info(msg.c_str());

    // Calculate windows for common sizes
    std::vector<size_t> sizes;
    for (size_t size = minSize; size <= maxSize; size *= 2) {
        sizes.push_back(size);
    }

    // Calculate for each shape and size
    for (auto size : sizes) {
        for (int shape = 0; shape < 4; ++shape) {
            auto shapeType = static_cast<GrainShapeType>(shape);
            WindowKey key{shapeType, size};
            if (mWindowCache.find(key) == mWindowCache.end()) {
                mWindowCache[key] = calculateWindow(shapeType, size);
            }
        }
    }
}

std::vector<float> GrainGenerator::getWindow(GrainShapeType shape, size_t size) {
    WindowKey key{shape, size};
    auto it = mWindowCache.find(key);

    if (it != mWindowCache.end()) {
        return it->second;
    }

    // Calculate window if not in cache
    auto window = calculateWindow(shape, size);
    mWindowCache[key] = window;
    return window;
}

std::vector<float> GrainGenerator::calculateWindow(GrainShapeType shape, size_t size) {
    std::vector<float> window(size);
    const double pi = 3.14159265358979323846;

    switch (shape) {
        case GrainShapeType::Sine: {
            for (size_t i = 0; i < size; ++i) {
                double phase = (static_cast<double>(i) / (size - 1)) * pi;
                window[i] = static_cast<float>(std::sin(phase));
            }
            break;
        }

        case GrainShapeType::Triangle: {
            size_t halfSize = size / 2;
            float increment = 1.0f / halfSize;

            // Rising edge
            for (size_t i = 0; i < halfSize; ++i) {
                window[i] = i * increment;
            }
            // Falling edge
            for (size_t i = halfSize; i < size; ++i) {
                window[i] = 1.0f - ((i - halfSize) * increment);
            }
            break;
        }

        case GrainShapeType::Rectangle: {
            std::fill(window.begin(), window.end(), 1.0f);
            break;
        }

        case GrainShapeType::Gaussian: {
            double sigma = 0.4;  // Controls the width of the Gaussian
            double center = (size - 1) / 2.0;

            for (size_t i = 0; i < size; ++i) {
                double x = (i - center) / (size * sigma);
                window[i] = static_cast<float>(std::exp(-0.5 * x * x));
            }
            break;
        }
    }

    return window;
}

void GrainGenerator::applyWindow(AudioBuffer& grain, const std::vector<float>& window) {
    for (size_t channel = 0; channel < grain.getNumChannels(); ++channel) {
        std::vector<float> buffer(window.size());

        // Get samples
        for (size_t i = 0; i < buffer.size(); ++i) {
            buffer[i] = grain.getSample(channel, i) * window[i];
        }

        // Write back to grain
        grain.write(channel, buffer.data(), buffer.size(), 0);
    }
}

float GrainGenerator::interpolateSample(const AudioBuffer& buffer, size_t channel, float position) {
    // Bounds checking
    if (position < 0 || position >= buffer.getNumSamples() - 1) {
        return 0.0f;
    }

    // Get integer and fractional parts of the position
    size_t pos0 = static_cast<size_t>(position);
    size_t pos1 = pos0 + 1;
    float frac = position - pos0;

    // Linear interpolation
    float sample0 = buffer.getSample(channel, pos0);
    float sample1 = buffer.getSample(channel, pos1);
    return sample0 + frac * (sample1 - sample0);
}

} // namespace GranularPlunderphonics