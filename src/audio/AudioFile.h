/**
 * @file AudioFile.h
 * @brief Audio file management class for handling various audio formats
 */

#pragma once

#include <string>
#include <memory>
#include <vector>
#include "../common/Logger.h"
#include "../common/ErrorHandling.h"

namespace GranularPlunderphonics {

/**
 * @enum AudioFileFormat
 * @brief Supported audio file formats
 */
enum class AudioFileFormat {
    WAV,
    AIFF,
    FLAC,
    Unknown
};

/**
 * @struct AudioFileInfo
 * @brief Contains metadata about an audio file
 */
struct AudioFileInfo {
    size_t numChannels{0};
    double sampleRate{0.0};
    size_t bitDepth{0};
    size_t numFrames{0};
    AudioFileFormat format{AudioFileFormat::Unknown};
    bool isValid() const { return numChannels > 0 && sampleRate > 0 && bitDepth > 0; }
};

/**
 * @class AudioFile
 * @brief Handles loading and managing audio content
 */
class AudioFile {
public:
    AudioFile();
    ~AudioFile();

    /**
     * @brief Load audio file from disk
     * @param path Path to audio file
     * @return true if successful
     */
    bool load(const std::string& path);

    /**
     * @brief Save audio to file
     * @param path Output file path
     * @param format Desired output format
     * @return true if successful
     */
    bool save(const std::string& path, AudioFileFormat format = AudioFileFormat::WAV);

    /**
     * @brief Get audio file information
     * @return AudioFileInfo struct containing file metadata
     */
    const AudioFileInfo& getInfo() const { return mInfo; }

    /**
     * @brief Get audio samples for a channel
     * @param channel Channel index
     * @return Vector of samples, or empty vector if invalid channel
     */
    const std::vector<float>& getChannelData(size_t channel) const;

    /**
     * @brief Set sample rate for the audio file
     * @param newRate Target sample rate
     * @return true if successful
     */
    bool setSampleRate(double newRate);

    /**
     * @brief Set bit depth for the audio file
     * @param newBitDepth Target bit depth
     * @return true if successful
     */
    bool setBitDepth(size_t newBitDepth);

    /**
     * @brief Enable/disable memory mapping
     * @param enable True to enable memory mapping
     * @return true if successful
     */
    bool enableMemoryMapping(bool enable);

    /**
     * @brief Check if file is loaded
     * @return true if file is loaded
     */
    bool isLoaded() const { return mIsLoaded; }

private:
    AudioFileInfo mInfo;
    std::vector<std::vector<float>> mAudioData;
    bool mIsLoaded{false};
    bool mIsMemoryMapped{false};
    Logger mLogger{"AudioFile"};

    /**
     * @brief Clear all loaded audio data
     */
    void clear();

    /**
     * @brief Detect file format from extension
     * @param path File path
     * @return Detected format
     */
    AudioFileFormat detectFormat(const std::string& path) const;
};

} // namespace GranularPlunderphonics
