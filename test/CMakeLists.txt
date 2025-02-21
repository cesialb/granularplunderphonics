# test/CMakeLists.txt

# Find Catch2
find_package(Catch2 3 REQUIRED)

add_executable(GranularTests
        AudioFileTests.cpp
        GrainCloudTests.cpp
        GrainGeneratorTests.cpp
        GrainProcessorTests.cpp
        ParameterTests.cpp
        ProcessorTests.cpp
        ResourceManagerTests.cpp
        TestMain.cpp
)

target_link_libraries(GranularTests
        PRIVATE
        Catch2::Catch2WithMain
        GranularPlunderphonicsCore
)

include(CTest)
include(Catch)
catch_discover_tests(GranularTests)
