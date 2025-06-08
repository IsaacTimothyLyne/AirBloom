// Source/PluginProcessor.h
#pragma once

#include <JuceHeader.h>

class AirBloomAudioProcessor : public juce::AudioProcessor
{
public:
    AirBloomAudioProcessor();
    ~AirBloomAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "AirBloom"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float>* bloomParam = nullptr;
    std::atomic<float>* atmosParam = nullptr;

private:

    // Wrap the mono IIR filter so it works on stereo/multi channels
    using EQ = juce::dsp::ProcessorDuplicator<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>
    >;

    juce::dsp::ProcessorChain<
        EQ,
        juce::dsp::Gain<float>,
        juce::dsp::Reverb
    > dspChain;

    enum ChainIds { Eq, Drive, Rev };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AirBloomAudioProcessor)
};
