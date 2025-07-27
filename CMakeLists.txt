# --- top of CMakeLists.txt -----------------------------------
cmake_minimum_required (VERSION 3.21)

# enable everything JUCE might need on each platform
project (
    AirBloom
    VERSION 1.0.0
    LANGUAGES C CXX OBJC OBJCXX   #  ← add C + Obj-C(+ +)
)


project(AirBloom LANGUAGES CXX)

# -----------------------------------------------------------------
# 1. Bring in JUCE (we added it as git sub-module in extern/JUCE)
# -----------------------------------------------------------------
add_subdirectory(extern/JUCE)

# -----------------------------------------------------------------
# 2. Define the plug-in target
# -----------------------------------------------------------------
set(COMAPANY "AspireAudio")              # no spaces
set(PLUGIN_VERSION "1.0.0")              # change whenever you tag

juce_add_plugin(AirBloom
    COMPANY_NAME            "${COMAPANY}"
    BUNDLE_ID               "com.${COMAPANY}.AirBloom"  # explicit, no spaces
    VERSION                 "${PLUGIN_VERSION}"
    PLUGIN_MANUFACTURER_CODE AspA
    PLUGIN_CODE              Ablm
    FORMATS                  VST3 AU
    PRODUCT_NAME             "AirBloom"
    SOURCE_DIRS              Source
)

# <-- add this -----------------------------------------------
juce_generate_juce_header (AirBloom)
# ------------------------------------------------------------

# -----------------------------------------------------------------
# 3. C++ standard + extra compiler flags (optional)
# -----------------------------------------------------------------
target_compile_features(AirBloom PRIVATE cxx_std_17)
# target_compile_definitions(AirBloom PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)

# -----------------------------------------------------------------
# 4. Link JUCE modules you use
# -----------------------------------------------------------------
target_link_libraries(AirBloom
    PRIVATE
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
)
