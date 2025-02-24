# test/CMakeLists.txt

# Find Catch2
find_package(Catch2 3 REQUIRED)

# Find other dependencies
find_package(SndFile REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFTW REQUIRED fftw3f)

add_executable(GranularTests
        TestMain.cpp
        BasicTests.cpp
        AudioFileTests.cpp
        GrainCloudTests.cpp
        GrainGeneratorTests.cpp
        GrainProcessorTests.cpp
        LorenzAttractorTests.cpp
        ParameterTests.cpp
        ProcessorTests.cpp
        ResourceManagerTests.cpp
        # Add the processor source files directly to the test executable
        ${CMAKE_SOURCE_DIR}/src/plugin/GranularPlunderphonicsProcessor.cpp
        ${CMAKE_SOURCE_DIR}/src/plugin/GranularPlunderphonicsController.cpp
)

target_include_directories(GranularTests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${VST3_SDK_ROOT}
        ${FFTW_ROOT}/include
        ${SAMPLERATE_INCLUDE_DIRS}
)

target_link_directories(GranularTests
        PRIVATE
        ${FFTW_ROOT}/lib
        ${SAMPLERATE_ROOT}/lib
)

if(APPLE)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    target_link_libraries(GranularTests
            PRIVATE
            Catch2::Catch2WithMain
            GranularPlunderphonicsCore
            sdk
            base
            pluginterfaces
            SndFile::sndfile
            "${FFTW_ROOT}/lib/libfftw3f.dylib"
            "${SAMPLERATE_ROOT}/lib/libsamplerate.dylib"
            "-framework CoreFoundation"
    )
else()
    target_link_libraries(GranularTests
            PRIVATE
            Catch2::Catch2WithMain
            GranularPlunderphonicsCore
            sdk
            base
            pluginterfaces
            SndFile::sndfile
            "${FFTW_ROOT}/lib/libfftw3f.dylib"
            "${SAMPLERATE_ROOT}/lib/libsamplerate.dylib"
    )
endif()

include(CTest)
include(Catch)
catch_discover_tests(GranularTests)