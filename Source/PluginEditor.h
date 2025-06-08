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

    juce::Slider bloomSlider;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> bloomAttachment;

    juce::ToggleButton atmosToggle;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<ButtonAttachment> atmosAttachment;

    juce::Font titleFont{ 28.0f, juce::Font::bold };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AirBloomAudioProcessorEditor)
};
