#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "../src/audio/AudioFile.h"
#include "../src/audio/AudioBuffer.h"
#include <sndfile.h>
#include <cmath>
#include <vector>
#include <random>

using namespace GranularPlunderphonics;

namespace {
    // Helper to create test audio data
    std::vector<float> createTestTone(float frequency, float sampleRate, size_t numSamples) {
        std::vector<float> data(numSamples);
        for (size_t i = 0; i < numSamples; ++i) {
            float t = static_cast<float>(i) / sampleRate;
            data[i] = std::sin(2.0f * M_PI * frequency * t);
        }
        return data;
    }

    // Helper to write test audio file
    bool writeTestFile(const std::string& path,
                  const std::vector<float>& data,
                  float sampleRate,
                  AudioFileFormat format) {
        SF_INFO sfinfo;
        memset(&sfinfo, 0, sizeof(sfinfo));

        sfinfo.samplerate = static_cast<int>(sampleRate);
        sfinfo.channels = 1;

        // Set format based on requested format
        switch (format) {
            case AudioFileFormat::WAV:
                sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
            break;
            case AudioFileFormat::AIFF:
                sfinfo.format = SF_FORMAT_AIFF | SF_FORMAT_FLOAT;
            break;
            case AudioFileFormat::FLAC:
                sfinfo.format = SF_FORMAT_FLAC | SF_FORMAT_PCM_24;
            break;
            default:
                return false;
        }

        SNDFILE* file = sf_open(path.c_str(), SFM_WRITE, &sfinfo);
        if (!file) {
            return false;
        }

        // Write data
        sf_count_t written = sf_write_float(file, data.data(), data.size());
        sf_close(file);

        return written == static_cast<sf_count_t>(data.size());
    }
}

TEST_CASE("Audio File Management Tests", "[audiofile]") {
    const float sampleRate = 44100.0f;

    SECTION("Short Samples (<1s)") {
        // Create 0.5s test file
        const float sampleRate = 44100.0f;
        std::vector<float> shortData = createTestTone(440.0f, sampleRate,
                                                     static_cast<size_t>(sampleRate * 0.5f));

        Logger logger("AudioFileTest");
        logger.info("Creating test file...");
        bool writeResult = writeTestFile("test_short.wav", shortData, sampleRate, AudioFileFormat::WAV);
        logger.info(writeResult ? "Test file created successfully" : "Failed to create test file");

        AudioFile file;
        logger.info("Attempting to load test file...");
        bool loadResult = file.load("test_short.wav");
        logger.info(loadResult ? "File loaded successfully" : "Failed to load file");

        REQUIRE(writeResult);  // First verify write succeeded
        REQUIRE(loadResult);   // Then verify load succeeded
        REQUIRE(file.getInfo().numFrames == shortData.size());
        REQUIRE(file.getInfo().sampleRate == sampleRate);
    }

    SECTION("Medium Length Samples (10-30s)") {
        // Create 15s test file
        std::vector<float> mediumData = createTestTone(440.0f, sampleRate,
                                                      static_cast<size_t>(sampleRate * 15.0f));
        writeTestFile("test_medium.wav", mediumData, sampleRate, AudioFileFormat::WAV);

        AudioFile file;
        REQUIRE(file.load("test_medium.wav"));
        REQUIRE(file.getInfo().numFrames == mediumData.size());
        
        // Test streaming capability
        bool streamingEnabled = file.enableStreaming(true);
        REQUIRE(streamingEnabled);
        
        // Test buffer reading in chunks
        const size_t chunkSize = 4096;
        std::vector<float> buffer(chunkSize);
        size_t totalRead = 0;
        while (totalRead < file.getInfo().numFrames) {
            size_t toRead = std::min(chunkSize, file.getInfo().numFrames - totalRead);
            REQUIRE(file.readBuffer(buffer.data(), toRead) == toRead);
            totalRead += toRead;
        }
    }

    SECTION("Long Samples (>2min)") {
        // Create 130s test file
        std::vector<float> longData = createTestTone(440.0f, sampleRate,
                                                    static_cast<size_t>(sampleRate * 130.0f));
        writeTestFile("test_long.wav", longData, sampleRate, AudioFileFormat::WAV);

        AudioFile file;
        REQUIRE(file.load("test_long.wav"));
        
        // Test memory-mapped access
        REQUIRE(file.enableMemoryMapping(true));
        
        // Verify memory-mapped reading
        const auto& channelData = file.getChannelData(0);
        REQUIRE(channelData.size() == longData.size());
        
        // Test random access performance
        const size_t numAccesses = 1000;
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<size_t> dist(0, longData.size() - 1);
        
        for (size_t i = 0; i < numAccesses; ++i) {
            size_t pos = dist(rng);
            REQUIRE(std::abs(channelData[pos] - longData[pos]) < 1e-6f);
        }
    }

    SECTION("Different Sample Rates") {
        std::vector<float> sampleRates = {22050.0f, 44100.0f, 48000.0f, 96000.0f};
        
        for (float rate : sampleRates) {
            // Create test file at current rate
            std::vector<float> data = createTestTone(440.0f, rate,
                                                    static_cast<size_t>(rate));
            writeTestFile("test_rate.wav", data, rate, AudioFileFormat::WAV);

            AudioFile file;
            REQUIRE(file.load("test_rate.wav"));
            REQUIRE(file.getInfo().sampleRate == rate);

            // Test sample rate conversion
            float newRate = rate * 2.0f;
            REQUIRE(file.setSampleRate(newRate));
            REQUIRE(file.getInfo().sampleRate == newRate);
        }
    }

    SECTION("Different Bit Depths") {
        std::vector<int> bitDepths = {16, 24, 32};

        for (int depth : bitDepths) {
            // Create a simple audio file for testing
            AudioFile file;

            // Create test tone
            std::vector<float> data = createTestTone(440.0f, sampleRate,
                                                   static_cast<size_t>(sampleRate));

            // We need to create audio data since we can't modify getInfo() directly
            std::vector<std::vector<float>> channels;
            channels.push_back(data); // One channel

            // Set up internal data using reflection or directly
            file.setBitDepth(depth);

            // Use the file path for this depth
            std::string filename = "test_depth_" + std::to_string(depth) + ".wav";

            // Simply write the test tone to the file directly
            SF_INFO sfInfo;
            std::memset(&sfInfo, 0, sizeof(SF_INFO));
            sfInfo.channels = 1;
            sfInfo.samplerate = sampleRate;

            // Set appropriate format with requested bit depth
            if (depth == 16) {
                sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
            } else if (depth == 24) {
                sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_24;
            } else {
                sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
            }

            // Write directly using libsndfile
            SNDFILE* sndfile = sf_open(filename.c_str(), SFM_WRITE, &sfInfo);
            sf_writef_float(sndfile, data.data(), data.size());
            sf_close(sndfile);

            // Now verify using our AudioFile class
            AudioFile loadedFile;
            REQUIRE(loadedFile.load(filename));

            // Test bit depth conversion
            int newDepth = (depth == 32) ? 24 : 32;
            REQUIRE(loadedFile.setBitDepth(newDepth));
            REQUIRE(loadedFile.getInfo().bitDepth == newDepth);
        }
    }

    SECTION("Format Support") {
        std::vector<AudioFileFormat> formats = {
            AudioFileFormat::WAV,
            AudioFileFormat::AIFF,
            AudioFileFormat::FLAC
        };

        std::vector<float> data = createTestTone(440.0f, sampleRate,
                                                static_cast<size_t>(sampleRate));

        for (auto format : formats) {
            std::string extension;
            switch (format) {
                case AudioFileFormat::WAV: extension = ".wav"; break;
                case AudioFileFormat::AIFF: extension = ".aiff"; break;
                case AudioFileFormat::FLAC: extension = ".flac"; break;
                default: continue;
            }

            std::string filename = "test_format" + extension;
            REQUIRE(writeTestFile(filename, data, sampleRate, format));

            AudioFile file;
            REQUIRE(file.load(filename));
            REQUIRE(file.getInfo().format == format);
        }
    }

    SECTION("Thread Safety") {
        // Create test file
        std::vector<float> data = createTestTone(440.0f, sampleRate,
                                                static_cast<size_t>(sampleRate * 5.0f));
        writeTestFile("test_thread.wav", data, sampleRate, AudioFileFormat::WAV);

        AudioFile file;
        REQUIRE(file.load("test_thread.wav"));

        const int numThreads = 4;
        const int iterationsPerThread = 100;
        std::atomic<bool> foundError{false};
        std::vector<std::thread> threads;

        // Create multiple threads accessing the file simultaneously
        for (int t = 0; t < numThreads; ++t) {
            threads.emplace_back([&]() {
                std::vector<float> buffer(1024);
                for (int i = 0; i < iterationsPerThread && !foundError; ++i) {
                    size_t randomPos = rand() % (data.size() - buffer.size());
                    if (!file.readBuffer(buffer.data(), buffer.size(), randomPos)) {
                        foundError = true;
                    }
                }
            });
        }

        // Wait for all threads
        for (auto& thread : threads) {
            thread.join();
        }

        REQUIRE_FALSE(foundError);
    }
}