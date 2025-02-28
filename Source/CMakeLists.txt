juce_add_plugin(GranularPlunderphonics
        COMPANY_NAME "YourCompany"
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        PLUGIN_MANUFACTURER_CODE Grpl
        PLUGIN_CODE Grph
        FORMATS VST3 Standalone
        PRODUCT_NAME "Granular Plunderphonics")

# Add JUCE modules
target_compile_definitions(GranularPlunderphonics
        PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_sources(GranularPlunderphonics
        PRIVATE
        PluginProcessor.cpp
        PluginEditor.cpp)

# Link against JUCE modules
target_link_libraries(GranularPlunderphonics
        PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_audio_plugin_client
        juce::juce_dsp
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)