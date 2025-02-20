# Source directory CMakeLists.txt

# Define source files
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

# Create the plugin target
add_library(GranularPlunderphonics MODULE
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

# Set output directory
set_target_properties(GranularPlunderphonics PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${VST3_OUTPUT_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${VST3_OUTPUT_DIR}"
)

# Installation
install(TARGETS GranularPlunderphonics
        LIBRARY DESTINATION "${VST3_OUTPUT_DIR}"
        RUNTIME DESTINATION "${VST3_OUTPUT_DIR}"
)
