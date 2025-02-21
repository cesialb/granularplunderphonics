# test/CMakeLists.txt
find_package(Catch2 3 REQUIRED)

add_executable(GranularTests
        GrainCloudTests.cpp
        # ... other test files ...
)

target_link_libraries(GranularTests
        PRIVATE
        Catch2::Catch2WithMain
        GranularPlunderphonics
)

include(CTest)
include(Catch)
catch_discover_tests(GranularTests)
