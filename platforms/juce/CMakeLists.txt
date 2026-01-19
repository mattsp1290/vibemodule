cmake_minimum_required(VERSION 3.21)

# Linux-specific: disable JUCE features that require system dependencies
if(UNIX AND NOT APPLE)
    set(JUCE_NEEDS_CURL OFF CACHE BOOL "" FORCE)
    set(JUCE_NEEDS_WEB_BROWSER OFF CACHE BOOL "" FORCE)
endif()

# Fetch JUCE via CPM
CPMAddPackage(
    NAME JUCE
    GIT_TAG 8.0.1
    GITHUB_REPOSITORY juce-framework/JUCE
    OPTIONS
        "JUCE_BUILD_EXTRAS OFF"
        "JUCE_BUILD_EXAMPLES OFF"
)

# Define the plugin
juce_add_plugin(CloudsReverb
    COMPANY_NAME "vibemodule"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Vibe
    PLUGIN_CODE CldR
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "Clouds Reverb"
)

# Generate JUCE header
juce_generate_juce_header(CloudsReverb)

# Add source files
target_sources(CloudsReverb
    PRIVATE
        PluginProcessor.cpp
        PluginProcessor.h
        PluginEditor.cpp
        PluginEditor.h
)

# Compile definitions
target_compile_definitions(CloudsReverb
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# Link libraries
target_link_libraries(CloudsReverb
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        clouds::dsp
)
