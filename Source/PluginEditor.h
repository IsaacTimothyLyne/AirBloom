#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "AirBloomLookAndFeel.h"

class AirBloomAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    AirBloomAudioProcessorEditor(AirBloomAudioProcessor&);
    ~AirBloomAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    AirBloomAudioProcessor& processorRef;

    std::unique_ptr<AirBloomLookAndFeel> lookAndFeel;
    juce::Image                          backgroundImage;

    // ── Top‐bar controls ────────────────────────────────────────────────────────
    static constexpr int topBarHeight = 60;
    juce::ComboBox     presetBox;
    juce::ToggleButton bypassButton, lowCutButton;

    // ── Main knobs ──────────────────────────────────────────────────────────────
    juce::Slider bloomSlider;
    using BloomAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<BloomAttachment> bloomAttachment;

    juce::Slider reverbWetSlider;
    using WetAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<WetAttachment> reverbWetAttachment;

    juce::Slider inputGainSlider, outputGainSlider;
    using GainAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<GainAttachment> inputGainAttachment, outputGainAttachment;

    // PluginEditor.h   (member additions)
    using BtnAtt = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<BtnAtt> bypassAttachment, lowCutAttachment;

    juce::ComboBox oversampleBox;                             // replaces ToggleButton
    using ChoiceAtt = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<ChoiceAtt> oversampleAttachment;

    // ── Labels ─────────────────────────────────────────────────────────────────
    juce::Label bloomLabel, reverbLabel, inputGainLabel, outputGainLabel, panelLeft, panelCentre, panelRight;

    juce::Font titleFont{ 28.0f, juce::Font::bold };

    // LOGO
    juce::ImageComponent aspireLogo;

#if DEV_PRESET_SAVE
    juce::TextButton saveBtn{ "Save" };
#endif


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AirBloomAudioProcessorEditor)
};
