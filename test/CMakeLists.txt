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
)

target_include_directories(GranularTests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${VST3_SDK_ROOT}
        ${FFTW_ROOT}/include
)

target_link_directories(GranularTests
        PRIVATE
        ${FFTW_ROOT}/lib
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
    )
endif()

include(CTest)
include(Catch)
catch_discover_tests(GranularTests)