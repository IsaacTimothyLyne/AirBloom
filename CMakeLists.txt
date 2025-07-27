cmake_minimum_required(VERSION 3.18)
project (AirBloom VERSION 1.0.0)

# Add JUCE
add_subdirectory (extern/JUCE)

juce_add_plugin (AirBloom
    COMPANY_NAME "Aspire Audio"
    IS_SYNTH FALSE
    BUNDLE_ID com.aspireaudio.airbloom
    FORMATS VST3 AU
    PRODUCT_NAME "AirBloom")

target_sources (AirBloom
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/PresetManager.cpp)

target_link_libraries (AirBloom
    PRIVATE
        juce::juce_audio_utils)   # plus any other JUCE modules you use
