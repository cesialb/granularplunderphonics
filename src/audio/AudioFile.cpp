/**
 * @file AudioFile.cpp
 * @brief Implementation of AudioFile class
 */

#include "AudioFile.h"
#include <filesystem>
#include <algorithm>
#include <sndfile.h>

namespace GranularPlunderphonics {

AudioFile::AudioFile() {
    mLogger.info("Creating AudioFile instance");
}

AudioFile::~AudioFile() {
    clear();
}

bool AudioFile::load(const std::string& path) {
    mLogger.info(("Loading audio file: " + path).c_str());
    // Clear any existing data
    clear();

    // Check if file exists
    if (!std::filesystem::exists(path)) {
        mLogger.error(("File does not exist: " + path).c_str());        return false;
    }

    // Detect format
    mInfo.format = detectFormat(path);
    if (mInfo.format == AudioFileFormat::Unknown) {
        mLogger.error(("Unsupported file format: " + path).c_str());        return false;
    }

    // Open file using libsndfile
    SF_INFO sfInfo;
    std::memset(&sfInfo, 0, sizeof(SF_INFO));

    SNDFILE* file = sf_open(path.c_str(), SFM_READ, &sfInfo);
    if (!file) {
        mLogger.error(("Failed to open file: " + std::string(sf_strerror(nullptr))).c_str());
        return false;
    }

    // Store file info
    mInfo.numChannels = sfInfo.channels;
    mInfo.sampleRate = sfInfo.samplerate;
    mInfo.numFrames = sfInfo.frames;
    mInfo.bitDepth = 32; // libsndfile converts to float internally

    // Allocate buffers
    mAudioData.resize(mInfo.numChannels);
    for (auto& channel : mAudioData) {
        channel.resize(mInfo.numFrames);
    }

    // Read interleaved data
    std::vector<float> interleavedBuffer(mInfo.numFrames * mInfo.numChannels);
    sf_count_t framesRead = sf_readf_float(file, interleavedBuffer.data(), mInfo.numFrames);

    if (framesRead != static_cast<sf_count_t>(mInfo.numFrames)) {
        mLogger.error("Failed to read all frames");
        sf_close(file);
        clear();
        return false;
    }

    // Deinterleave into channels
    for (size_t frame = 0; frame < mInfo.numFrames; ++frame) {
        for (size_t channel = 0; channel < mInfo.numChannels; ++channel) {
            mAudioData[channel][frame] = interleavedBuffer[frame * mInfo.numChannels + channel];
        }
    }

    sf_close(file);
    mIsLoaded = true;
    mLogger.info("Successfully loaded audio file");
    return true;
}

bool AudioFile::save(const std::string& path, AudioFileFormat format) {
    if (!mIsLoaded) {
        mLogger.error("No audio data to save");
        return false;
    }

    // Setup format info for libsndfile
    SF_INFO sfInfo;
    std::memset(&sfInfo, 0, sizeof(SF_INFO));

    sfInfo.channels = mInfo.numChannels;
    sfInfo.samplerate = mInfo.sampleRate;

    // Set format based on requested format and extension
    switch (format) {
        case AudioFileFormat::WAV:
            sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
            break;
        case AudioFileFormat::AIFF:
            sfInfo.format = SF_FORMAT_AIFF | SF_FORMAT_FLOAT;
            break;
        case AudioFileFormat::FLAC:
            sfInfo.format = SF_FORMAT_FLAC | SF_FORMAT_PCM_24;
            break;
        default:
            mLogger.error("Unsupported save format");
            return false;
    }

    // Create interleaved buffer
    std::vector<float> interleavedBuffer(mInfo.numFrames * mInfo.numChannels);

    // Interleave channels
    for (size_t frame = 0; frame < mInfo.numFrames; ++frame) {
        for (size_t channel = 0; channel < mInfo.numChannels; ++channel) {
            interleavedBuffer[frame * mInfo.numChannels + channel] = mAudioData[channel][frame];
        }
    }

    // Open file for writing
    SNDFILE* file = sf_open(path.c_str(), SFM_WRITE, &sfInfo);
    if (!file) {
        mLogger.error(("Failed to create output file: " + std::string(sf_strerror(nullptr))).c_str());
        return false;
    }

    // Write data
    sf_count_t framesWritten = sf_writef_float(file, interleavedBuffer.data(), mInfo.numFrames);

    sf_close(file);

    if (framesWritten != static_cast<sf_count_t>(mInfo.numFrames)) {
        mLogger.error("Failed to write all frames");
        return false;
    }

    mLogger.info("Successfully saved audio file");
    return true;
}

const std::vector<float>& AudioFile::getChannelData(size_t channel) const {
    static const std::vector<float> empty;
    if (channel >= mInfo.numChannels) {
        return empty;
    }
    return mAudioData[channel];
}

void AudioFile::clear() {
    mAudioData.clear();
    mInfo = AudioFileInfo();
    mIsLoaded = false;
    mIsMemoryMapped = false;
}

AudioFileFormat AudioFile::detectFormat(const std::string& path) const {
    std::string extension = std::filesystem::path(path).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    if (extension == ".wav") return AudioFileFormat::WAV;
    if (extension == ".aiff" || extension == ".aif") return AudioFileFormat::AIFF;
    if (extension == ".flac") return AudioFileFormat::FLAC;

    return AudioFileFormat::Unknown;
}

bool AudioFile::setSampleRate(double newRate) {
    // Implementation will be added in the next step with resampling
    return false;
}

bool AudioFile::setBitDepth(size_t newBitDepth) {
    // Implementation will be added in the next step
    return false;
}

bool AudioFile::enableMemoryMapping(bool enable) {
    // Implementation will be added in a future step
    return false;
}

} // namespace GranularPlunderphonics
