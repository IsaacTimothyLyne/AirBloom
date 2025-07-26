#pragma once
#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>   // already there, but make sure it’s included
#include "PresetManager.h"

class AirBloomAudioProcessor : public juce::AudioProcessor
{
public:
    AirBloomAudioProcessor();
    ~AirBloomAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override { return "AirBloom"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float>* bloomParam = nullptr;
    //std::atomic<float>* atmosParam = nullptr;
    std::atomic<float>* reverbWetParam = nullptr;
    std::atomic<float>* inputGainParam = nullptr;
    std::atomic<float>* outputGainParam = nullptr;
    std::atomic<float>* bypassParam = nullptr;
    std::atomic<float>* lowCutParam = nullptr;


private:
    // — stereo-safe high-shelf filter —
    using Shelf = juce::dsp::ProcessorDuplicator<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>
    >;
    Shelf        shelfFilter;
    juce::dsp::Gain<float> driveGain;

    // — stereo-safe high-pass + reverb chain —
    using HPF = juce::dsp::ProcessorDuplicator<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>
    >;
    HPF          reverbHpf;
    juce::dsp::Reverb reverbProcessor;

    // — temp buffers to avoid per-block allocation —
    juce::AudioBuffer<float> colorBuffer, reverbBuffer;

    /* ★ NEW ↓  12 dB /oct high-pass for the main signal */
    using HPF = juce::dsp::ProcessorDuplicator<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>>;
    HPF          lowCutFilter;

    /* private:  … existing DSP members … */
    juce::dsp::WaveShaper<float> softClipper;   // ★ NEW

    /* ── parameters ───────────────────────────────────────────── */
    std::atomic<float>* oversampleParam = nullptr;          // ★ NEW

    /* ── DSP helpers ──────────────────────────────────────────── */
    std::unique_ptr<juce::dsp::Oversampling<float>> os2x;    // 2 ×
    std::unique_ptr<juce::dsp::Oversampling<float>> os4x;    // 4 ×

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bloomSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> wetSm;

    std::unique_ptr<PresetManager> presetManager;

    float prevInGain = 1.0f;
    float prevOutGain = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AirBloomAudioProcessor)
};
