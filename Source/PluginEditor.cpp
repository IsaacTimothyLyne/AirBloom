// Source/PluginEditor.cpp
#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "AirBloomLookAndFeel.h"

AirBloomAudioProcessorEditor::AirBloomAudioProcessorEditor(AirBloomAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // 1) Look & Feel
    lookAndFeel = std::make_unique<AirBloomLookAndFeel>();
    setLookAndFeel(lookAndFeel.get());

    // 2) Background
    backgroundImage = juce::ImageCache::getFromMemory(
        BinaryData::background_png, BinaryData::background_pngSize);

    // 3) Bloom knob
    bloomSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    bloomSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(bloomSlider);
    bloomAttachment = std::make_unique<SliderAttachment>(
        processorRef.parameters, "bloom", bloomSlider);

    // 4) ATMOS toggle
    atmosToggle.setButtonText("ATMOS");
    atmosToggle.setClickingTogglesState(true);
    addAndMakeVisible(atmosToggle);
    atmosAttachment = std::make_unique<ButtonAttachment>(
        processorRef.parameters, "atmos", atmosToggle);

    // 5) New half-size window
    setSize(333, 600);
}

AirBloomAudioProcessorEditor::~AirBloomAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void AirBloomAudioProcessorEditor::paint(juce::Graphics& g)
{
    if (backgroundImage.isValid())
        g.drawImage(backgroundImage, getLocalBounds().toFloat(), false);
    else
        g.fillAll(juce::Colours::darkslategrey);

    g.setColour(juce::Colours::white.withAlpha(0.9f));
    g.setFont(titleFont);
    g.drawFittedText("AirBloom", 0, 10, getWidth(), 40,
        juce::Justification::centred, 1);
}

void AirBloomAudioProcessorEditor::resized()
{
    // knob centered
    int knobSize = 140;
    int cx = getWidth() / 2;
    int cy = getHeight() / 2;
    bloomSlider.setBounds(cx - knobSize / 2,
        cy - knobSize / 2,
        knobSize, knobSize);

    // Atmos button 150×102, centered under knob
    int bw = 150, bh = 102;
    atmosToggle.setBounds(cx - bw / 2,
        cy + knobSize / 2 + 20,
        bw, bh);
}
