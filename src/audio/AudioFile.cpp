/**
 * @file AudioFile.cpp
 * @brief Implementation of AudioFile class
 */

#include "AudioFile.h"
#include <filesystem>
#include <algorithm>
#include <sndfile.h>
#include <cmath>
#include <memory>
#include <random>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/mman.h>  // for mmap, munmap
    #include <sys/stat.h>  // for struct stat
    #include <fcntl.h>     // for O_RDONLY
    #include <unistd.h>    // for close
#endif




#include "Resampler.h"

namespace GranularPlunderphonics {


    AudioFile::AudioFile()
        : mLogger("AudioFile")
        , mInfo{}
    , mIsLoaded(false)
    , mIsMemoryMapped(false)
    , mMappedData(nullptr)
    , mMappedSize(0)

{
    mLogger.info("Creating AudioFile instance");
}

AudioFile::~AudioFile() {
    clear();
}

bool AudioFile::load(const std::string& path) {
    mLogger.info(("Loading audio file: " + path).c_str());

    if (path.empty()) {
        mLogger.error("Empty file path provided");
        return false;
    }

    // Clear any existing data
    clear();
    mFilePath = path;

    // Open file and get info
    SF_INFO sfInfo;
    std::memset(&sfInfo, 0, sizeof(SF_INFO));

    SNDFILE* file = sf_open(path.c_str(), SFM_READ, &sfInfo);
    if (!file) {
        mLogger.error(("Failed to open file: " + std::string(sf_strerror(nullptr))).c_str());
        return false;
    }

    // Store file info
    mInfo.numChannels = sfInfo.channels;
    mInfo.numFrames = sfInfo.frames;
    mInfo.sampleRate = sfInfo.samplerate;
    mInfo.format = detectFormat(path);
    mInfo.bitDepth = 32; // Default to 32-bit float

    // Allocate buffers
    mAudioData.resize(mInfo.numChannels);
    for (auto& channel : mAudioData) {
        channel.resize(mInfo.numFrames);
    }

    // Read interleaved data
    std::vector<float> interleavedBuffer(mInfo.numFrames * mInfo.numChannels);
    sf_count_t framesRead = sf_readf_float(file, interleavedBuffer.data(), mInfo.numFrames);

    if (framesRead != static_cast<sf_count_t>(mInfo.numFrames)) {
        mLogger.error("Failed to read all frames from file");
        sf_close(file);
        clear();
        return false;
    }

    // Deinterleave into channel buffers
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

    SNDFILE* file = sf_open(path.c_str(), SFM_WRITE, &sfInfo);
    if (!file) {
        mLogger.error(("Failed to create output file: " + std::string(sf_strerror(nullptr))).c_str());
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

    // Write to file
    sf_count_t framesWritten = sf_writef_float(file, interleavedBuffer.data(), mInfo.numFrames);

    sf_close(file);

    if (framesWritten != static_cast<sf_count_t>(mInfo.numFrames)) {
        mLogger.error("Failed to write all frames");
        return false;
    }

    mLogger.info("Successfully saved audio file");
    return true;
}

bool AudioFile::setSampleRate(double newRate) {
    if (!mIsLoaded || newRate <= 0.0 || newRate == mInfo.sampleRate) {
        mLogger.error("Invalid sample rate conversion request");
        return false;
    }

    GranularPlunderphonics::Resampler resampler;
    std::vector<std::vector<float>> newData;
    newData.reserve(mInfo.numChannels);

    for (size_t channel = 0; channel < mInfo.numChannels; ++channel) {
        auto resampled = resampler.process(mAudioData[channel], mInfo.sampleRate, newRate);
        if (resampled.empty()) {
            mLogger.error(("Failed to resample channel " + std::to_string(channel)).c_str());
            return false;
        }
        newData.push_back(std::move(resampled));
    }

    mAudioData = std::move(newData);
    mInfo.sampleRate = newRate;
    mInfo.numFrames = mAudioData[0].size();

    mLogger.info(("Sample rate converted to " + std::to_string(newRate)).c_str());
    return true;
}

bool AudioFile::setBitDepth(int newBitDepth) {
    if (!mIsLoaded) {
        mLogger.error("No audio data loaded");
        return false;
    }

    if (newBitDepth != 16 && newBitDepth != 24 && newBitDepth != 32) {
        mLogger.error(("Unsupported bit depth: " + std::to_string(newBitDepth)).c_str());
        return false;
    }

    if (newBitDepth == mInfo.bitDepth) {
        return true;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // Calculate scaling factors
    float oldMax = std::pow(2.0f, mInfo.bitDepth - 1) - 1.0f;
    float newMax = std::pow(2.0f, newBitDepth - 1) - 1.0f;
    float scaleFactor = newMax / oldMax;

    // Process each channel
    for (auto& channel : mAudioData) {
        for (auto& sample : channel) {
            sample *= scaleFactor;

            // Apply dithering if reducing bit depth
            if (newBitDepth < mInfo.bitDepth) {
                float r1 = dist(gen);
                float r2 = dist(gen);
                sample += (r1 - r2) / newMax;
            }

            // Clamp to new range
            sample = std::clamp(sample, -1.0f, 1.0f);
        }
    }

    mInfo.bitDepth = newBitDepth;
    mLogger.info(("Bit depth converted to " + std::to_string(newBitDepth)).c_str());
    return true;
}

bool AudioFile::enableMemoryMapping(bool enable) {
    if (enable == mIsMemoryMapped) {
        return true;
    }

    if (!mIsLoaded) {
        mLogger.error("No file loaded for memory mapping");
        return false;
    }

    try {
        if (enable) {
#ifdef _WIN32
            // Windows implementation
            mFileHandle = CreateFileA(mFilePath.c_str(),
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);

            if (mFileHandle == INVALID_HANDLE_VALUE) {
                mLogger.error("Failed to open file for memory mapping");
                return false;
            }

            mMapHandle = CreateFileMapping(mFileHandle,
                NULL,
                PAGE_READONLY,
                0,
                0,
                NULL);

            if (!mMapHandle) {
                CloseHandle(mFileHandle);
                mFileHandle = INVALID_HANDLE_VALUE;
                mLogger.error("Failed to create file mapping");
                return false;
            }

            mMappedData = MapViewOfFile(mMapHandle,
                FILE_MAP_READ,
                0,
                0,
                0);

            if (!mMappedData) {
                CloseHandle(mMapHandle);
                CloseHandle(mFileHandle);
                mMapHandle = NULL;
                mFileHandle = INVALID_HANDLE_VALUE;
                mLogger.error("Failed to map view of file");
                return false;
            }
#else
            // POSIX implementation
            mFileDescriptor = open(mFilePath.c_str(), O_RDONLY);
            if (mFileDescriptor == -1) {
                mLogger.error("Failed to open file for memory mapping");
                return false;
            }

            struct stat sb;
            if (fstat(mFileDescriptor, &sb) == -1) {
                close(mFileDescriptor);
                mFileDescriptor = -1;
                mLogger.error("Failed to get file size");
                return false;
            }

            mMappedSize = sb.st_size;
            mMappedData = mmap(NULL, mMappedSize, PROT_READ, MAP_PRIVATE, mFileDescriptor, 0);

            if (mMappedData == MAP_FAILED) {
                close(mFileDescriptor);
                mFileDescriptor = -1;
                mMappedData = nullptr;
                mLogger.error("Failed to map file into memory");
                return false;
            }
#endif
            mIsMemoryMapped = true;
            mLogger.info("Memory mapping enabled");
        } else {
            // Cleanup memory mapping
#ifdef _WIN32
            if (mMappedData) {
                UnmapViewOfFile(mMappedData);
                mMappedData = nullptr;
            }
            if (mMapHandle) {
                CloseHandle(mMapHandle);
                mMapHandle = NULL;
            }
            if (mFileHandle != INVALID_HANDLE_VALUE) {
                CloseHandle(mFileHandle);
                mFileHandle = INVALID_HANDLE_VALUE;
            }
#else
            if (mMappedData) {
                munmap(mMappedData, mMappedSize);
                mMappedData = nullptr;
            }
            if (mFileDescriptor != -1) {
                close(mFileDescriptor);
                mFileDescriptor = -1;
            }
#endif
            mIsMemoryMapped = false;
            mLogger.info("Memory mapping disabled");
        }
        return true;
    }
    catch (const std::exception& e) {
        mLogger.error(("Memory mapping operation failed: " + std::string(e.what())).c_str());
        return false;
    }
}

const std::vector<float>& AudioFile::getChannelData(size_t channel) const {
    static const std::vector<float> empty;
    if (channel >= mInfo.numChannels) {
        return empty;
    }
    return mAudioData[channel];
}

void AudioFile::clear() {
    enableMemoryMapping(false);  // Cleanup memory mapping if active
    mAudioData.clear();
    mInfo = AudioFileInfo();
    mIsLoaded = false;
    mFilePath.clear();
}

AudioFileFormat AudioFile::detectFormat(const std::string& path) const {
    std::string extension = std::filesystem::path(path).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    if (extension == ".wav") return AudioFileFormat::WAV;
    if (extension == ".aiff" || extension == ".aif") return AudioFileFormat::AIFF;
    if (extension == ".flac") return AudioFileFormat::FLAC;

    return AudioFileFormat::Unknown;
}

} // namespace GranularPlunderphonics
