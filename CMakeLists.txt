cmake_minimum_required(VERSION 3.18)
project(AirBloom VERSION 1.0.0 LANGUAGES C CXX)

# --------------------------------------------------------------------------
# Tell CMake where the JUCE sub-module lives and pull in its functions
# --------------------------------------------------------------------------
set(JUCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/JUCE)          # <- matches git submodule path
add_subdirectory(${JUCE_DIR})                                  # this defines juce_add_plugin()

# --------------------------------------------------------------------------
# Your plug-in target
# --------------------------------------------------------------------------
juce_add_plugin(AirBloom
    COMPANY_NAME          "Aspire Audio"
    IS_SYNTH              FALSE
    NEEDS_MIDI_INPUT      FALSE
    NEEDS_MIDI_OUTPUT     FALSE
    IS_MIDI_EFFECT        FALSE
    FORMATS               VST3 AU
    PRODUCT_NAME          "AirBloom"
)

target_sources(AirBloom PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    # ...other .cpp/.h files...
)

target_compile_features(AirBloom PRIVATE cxx_std_20)

target_link_libraries(AirBloom
    PRIVATE
        juce::juce_audio_utils        # add whatever JUCE modules you actually use
)
