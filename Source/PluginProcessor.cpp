#include "PluginProcessor.h"
#include "PluginEditor.h"

AirBloomAudioProcessor::AirBloomAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo())
        .withOutput("Output", juce::AudioChannelSet::stereo())),
    parameters(*this, nullptr, "PARAMETERS",
        {
          std::make_unique<juce::AudioParameterFloat>("bloom", "Bloom",
             juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f),
          std::make_unique<juce::AudioParameterBool>("atmos", "Atmos", false)
        })
{
    bloomParam = parameters.getRawParameterValue("bloom");
    atmosParam = parameters.getRawParameterValue("atmos");
}

AirBloomAudioProcessor::~AirBloomAudioProcessor() {}

void AirBloomAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec{ sampleRate,
                                  (juce::uint32)samplesPerBlock,
                                  (juce::uint32)getTotalNumOutputChannels() };

    dspChain.prepare(spec);

    // 1) EQ @ unity gain
    {
        auto& eq = dspChain.get<ChainIds::Eq>();
        *eq.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            sampleRate, 10000.0f, 0.7071f, 1.0f);
    }

    // 2) Drive @ unity
    dspChain.get<ChainIds::Drive>().setGainLinear(1.0f);

    // 3) Reverb: always pass dry, shimmer only when Atmos is on
    juce::Reverb::Parameters rp;
    rp.roomSize = 0.8f;
    rp.damping = 0.2f;
    rp.wetLevel = 0.0f;  // shimmer off by default
    rp.dryLevel = 1.0f;  // always keep the processed dry signal
    rp.width = 1.0f;
    rp.freezeMode = 0.0f;
    dspChain.get<ChainIds::Rev>().setParameters(rp);
}

void AirBloomAudioProcessor::releaseResources() {}

bool AirBloomAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet()
        && !layouts.getMainInputChannelSet().isDisabled();
}

void AirBloomAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // read the knob + toggle
    float bloom = bloomParam->load();           // [0..1]
    bool  atmos = (atmosParam->load() > 0.5f);

    // sanity check
    jassert(bloom >= 0.0f && bloom <= 1.0f);

    // 1) Drive mapping (1→5)
    float driveGain = 1.0f + bloom * 4.0f;
    dspChain.get<ChainIds::Drive>().setGainLinear(driveGain);

    // 2) EQ shelf mapping (unity→+6dB)
    {
        auto& eq = dspChain.get<ChainIds::Eq>();
        float shelfGain = 1.0f + bloom * 1.0f;
        *eq.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            getSampleRate(), 10000.0f, 0.7071f, shelfGain);
    }

    // 3) Gate shimmer by Atmos
    {
        auto params = dspChain.get<ChainIds::Rev>().getParameters();
        params.wetLevel = atmos ? (0.3f * bloom) : 0.0f;
        dspChain.get<ChainIds::Rev>().setParameters(params);
    }

    // 4) Build wetBuffer & run full chain on it
    juce::AudioBuffer<float> wetBuffer(buffer);
    {
        juce::dsp::AudioBlock<float>              wetBlock(wetBuffer);
        juce::dsp::ProcessContextReplacing<float> wetCtx(wetBlock);
        dspChain.process(wetCtx);
    }

    // 5) Manual dry/wet mix
    const int numCh = buffer.getNumChannels();
    const int numSamp = buffer.getNumSamples();
    float dryLevel = 1.0f - bloom;

    for (int ch = 0; ch < numCh; ++ch)
    {
        float* dryData = buffer.getWritePointer(ch);
        auto* wetData = wetBuffer.getReadPointer(ch);

        for (int i = 0; i < numSamp; ++i)
            dryData[i] = dryData[i] * dryLevel + wetData[i] * bloom;
    }

    // 6) COMPENSATE MAKE-UP GAIN so the peak stays ~0 dB
    buffer.applyGain(1.0f / driveGain);

    // debug print// Option A: String concatenation
    DBG("Bloom=" + juce::String(bloom, 2)
        + "  Atmos=" + juce::String(atmos ? 1 : 0)
        + "  DriveGain=" + juce::String(driveGain, 2));

    // — or —

     // Option B: formatted String
    DBG(juce::String::formatted(
        "Bloom=%.2f  Atmos=%d  DriveGain=%.2f",
        bloom, atmos ? 1 : 0, driveGain));

}

juce::AudioProcessorEditor* AirBloomAudioProcessor::createEditor() { return new AirBloomAudioProcessorEditor(*this); }
void AirBloomAudioProcessor::getStateInformation(juce::MemoryBlock& d) { if (auto xml = parameters.copyState().createXml()) copyXmlToBinary(*xml, d); }
void AirBloomAudioProcessor::setStateInformation(const void* d, int s) { if (auto xml = getXmlFromBinary(d, s)) parameters.replaceState(juce::ValueTree::fromXml(*xml)); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new AirBloomAudioProcessor(); }
