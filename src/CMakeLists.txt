# Source directory CMakeLists.txt
set(PLUGIN_SOURCES
        plugin/GranularPlunderphonicsProcessor.cpp
        plugin/GranularPlunderphonicsController.cpp
        plugin/GranularPlunderphonicsEntry.cpp
        common/Logger.cpp
        common/ErrorHandling.cpp
)

set(PLUGIN_HEADERS
        plugin/GranularPlunderphonicsProcessor.h
        plugin/GranularPlunderphonicsController.h
        plugin/GranularPlunderphonicsIDs.h
        plugin/version.h
        common/Logger.h
        common/ErrorHandling.h
)

# VST3 plugin target
smtg_add_vst3plugin(GranularPlunderphonics
        ${PLUGIN_SOURCES}
        ${PLUGIN_HEADERS}
)

# Include directories
target_include_directories(GranularPlunderphonics
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${VST3_SDK_ROOT}
)

# Link dependencies
target_link_libraries(GranularPlunderphonics
        PRIVATE
        sdk
        sdk_common
        sdk_vstgui
        spdlog::spdlog
)

# Compiler options
target_compile_options(GranularPlunderphonics
        PRIVATE
        $<$<CONFIG:Debug>:-Wall -Wextra -Wpedantic -g>
        $<$<CONFIG:Release>:-O3>
)

# Define preprocessor macros
target_compile_definitions(GranularPlunderphonics
        PRIVATE
        PLUGIN_VERSION="${PROJECT_VERSION}"
        $<$<CONFIG:Debug>:DEBUG_BUILD>
)

# Install plugin
smtg_install_vst3_plugin(GranularPlunderphonics)

# Plugin validator - runs the VST3 validator on the plugin
if(SMTG_RUN_VST_VALIDATOR)
    smtg_run_vst3_validator(GranularPlunderphonics)
endif()