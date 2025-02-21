find_package(SndFile REQUIRED)

set(PLUGIN_SOURCES
        ${PLUGIN_SOURCES}
        audio/AudioFile.cpp
)

set(PLUGIN_HEADERS
        ${PLUGIN_HEADERS}
        audio/AudioFile.h
)

set(PLUGIN_SOURCES
        ${PLUGIN_SOURCES}
        common/ResourceManager.cpp
)

set(PLUGIN_HEADERS
        ${PLUGIN_HEADERS}
        common/ResourceManager.h
)

set(PLUGIN_SOURCES
        plugin/GranularPlunderphonicsProcessor.cpp
        plugin/GranularPlunderphonicsController.cpp
        plugin/GranularPlunderphonicsEntry.cpp
        plugin/ParameterManager.cpp
        common/Logger.cpp
        common/ErrorHandling.cpp
)

set(PLUGIN_HEADERS
        plugin/GranularPlunderphonicsProcessor.h
        plugin/GranularPlunderphonicsController.h
        plugin/GranularPlunderphonicsIDs.h
        plugin/GranularParameters.h
        plugin/ParameterManager.h
        plugin/version.h
        common/Logger.h
        common/ErrorHandling.h
)

add_library(GranularPlunderphonics MODULE
        ${PLUGIN_SOURCES}
        ${PLUGIN_HEADERS}
)

target_include_directories(GranularPlunderphonics
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${VST3_SDK_ROOT}
)

target_link_libraries(GranularPlunderphonics
        PRIVATE
        sdk
        base
        pluginterfaces
        spdlog::spdlog
        SndFile::sndfile
)
