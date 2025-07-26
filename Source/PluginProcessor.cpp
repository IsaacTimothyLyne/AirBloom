#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================
AirBloomAudioProcessor::AirBloomAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo())
        .withOutput("Output", juce::AudioChannelSet::stereo())),
    parameters(*this, nullptr, "PARAMETERS", {
        std::make_unique<juce::AudioParameterFloat>("bloom",      "Bloom",
             juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f),
       /* std::make_unique<juce::AudioParameterBool>("atmos",      "Atmos",     false),*/
        std::make_unique<juce::AudioParameterFloat>("reverbWet",  "Reverb Wet",
             juce::NormalisableRange<float>(0.0f, 1.0f), 0.3f),

             // ← new gain knobs:
             std::make_unique<juce::AudioParameterFloat>("inputGain",  "Input Gain",
                  juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f),
             std::make_unique<juce::AudioParameterFloat>("outputGain", "Output Gain",
                  juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f),
                  /* ★ NEW ↓ */
    std::make_unique<juce::AudioParameterBool>("bypass", "Bypass", false),
    std::make_unique<juce::AudioParameterBool>("lowCut","Low-Cut",false),
    std::make_unique<juce::AudioParameterChoice>(
        "oversample", "Oversample", juce::StringArray { juce::String::fromUTF8(u8"1×"), juce::String::fromUTF8(u8"2×"), juce::String::fromUTF8(u8"4×") }, 0)
        })
{
    bloomParam = parameters.getRawParameterValue("bloom");
    //atmosParam = parameters.getRawParameterValue("atmos");
    reverbWetParam = parameters.getRawParameterValue("reverbWet");
    // hook up your new ones too:
    inputGainParam = parameters.getRawParameterValue("inputGain");
    outputGainParam = parameters.getRawParameterValue("outputGain");
    /* ★ NEW ↓ */
    bypassParam = parameters.getRawParameterValue("bypass");
    lowCutParam = parameters.getRawParameterValue("lowCut");
    oversampleParam = parameters.getRawParameterValue("oversample");

    presetManager = std::make_unique<PresetManager>(parameters);

    if (parameters.state.getNumChildren() == 0)      // first-ever open
        presetManager->handleSelection("Init");
}



AirBloomAudioProcessor::~AirBloomAudioProcessor() {}

//==============================================================================
void AirBloomAudioProcessor::prepareToPlay(double sampleRate,
    int samplesPerBlock)
{
    // shared spec
    juce::dsp::ProcessSpec spec{
        sampleRate,
        (juce::uint32)samplesPerBlock,
        (juce::uint32)getTotalNumOutputChannels()
    };
    softClipper.prepare(spec);
    softClipper.functionToUse = [](float x)
        {
            // gentle analogue-style clip: 0 dBFS enters soft knee at ~-6 dB
            constexpr float drive = 1.5f;
            return std::tanh(x * drive);
        };

    prevInGain = 1.0f;
    prevOutGain = 1.0f;
    // 1) init shelf + drive ( unity )
    shelfFilter.prepare(spec);
    *shelfFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        sampleRate, 10000.0f, 0.7071f, 1.0f);
    driveGain.prepare(spec);
    driveGain.setGainLinear(1.0f);

    // 2) init HPF + reverb
    reverbHpf.prepare(spec);
    *reverbHpf.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
        sampleRate, 800.0f, 0.7071f);

    reverbProcessor.reset();
    reverbProcessor.prepare(spec);
    juce::Reverb::Parameters rp;
    rp.roomSize = 0.8f;
    rp.damping = 0.2f;
    rp.wetLevel = 1.0f;  // pure wet here
    rp.dryLevel = 0.0f;
    rp.width = 1.0f;
    rp.freezeMode = 0.0f;
    reverbProcessor.setParameters(rp);

    lowCutFilter.prepare(spec);
    *lowCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass
    (sampleRate, 100.0f, 0.7071f);   // 100 Hz, Q ≈ 0.7

    os2x = std::make_unique<juce::dsp::Oversampling<float>>(
        getTotalNumOutputChannels(), /*factor*/ 2,
        juce::dsp::Oversampling<float>::FilterType::filterHalfBandPolyphaseIIR);
    os2x->initProcessing(samplesPerBlock);

    os4x = std::make_unique<juce::dsp::Oversampling<float>>(
        getTotalNumOutputChannels(), 4,
        juce::dsp::Oversampling<float>::FilterType::filterHalfBandPolyphaseIIR);
    os4x->initProcessing(samplesPerBlock);

    constexpr double smoothTimeSec = 0.05;          // 50 ms ramp

    bloomSm.reset(sampleRate, smoothTimeSec);
    wetSm.reset(sampleRate, smoothTimeSec);

    /* start at current param values so there’s no jump on first block */
    bloomSm.setCurrentAndTargetValue(*bloomParam);
    wetSm.setCurrentAndTargetValue(*reverbWetParam);


    // 3) allocate temp buffers
    int numCh = getTotalNumOutputChannels();
    colorBuffer.setSize(numCh, samplesPerBlock, false, true, true);
    reverbBuffer.setSize(numCh, samplesPerBlock, false, true, true);

}

void AirBloomAudioProcessor::releaseResources() {}

//==============================================================================
bool AirBloomAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet()
        && !layouts.getMainInputChannelSet().isDisabled();
}

//==============================================================================
void AirBloomAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& /*midi*/)
{
    juce::ScopedNoDenormals noDenormals;

    const int numCh = buffer.getNumChannels();
    const int numSmp = buffer.getNumSamples();

    // make sure our temps match
    if (colorBuffer.getNumChannels() != numCh || colorBuffer.getNumSamples() != numSmp)
        colorBuffer.setSize(numCh, numSmp, false, true, true);
    if (reverbBuffer.getNumChannels() != numCh || reverbBuffer.getNumSamples() != numSmp)
        reverbBuffer.setSize(numCh, numSmp, false, true, true);

    // 1) read UI
    bloomSm.setTargetValue(*bloomParam);
    wetSm.setTargetValue(*reverbWetParam);
    float bloom = bloomParam->load();
    constexpr bool atmos = true;       // always ON now
    float wetMix = reverbWetParam->load();
    float inG = juce::Decibels::decibelsToGain(inputGainParam->load());
    float outG = juce::Decibels::decibelsToGain(outputGainParam->load());
    bool  doBypass = bypassParam->load() > 0.5f;
    bool  doLowCut = lowCutParam->load() > 0.5f;

    /* ---------- TRUE-BYPASS EARLY-OUT -------------------- */
    if (doBypass)
    {
        prevInGain = 1.0f;   // reset so first un-bypass is clean
        prevOutGain = 1.0f;
        bloomSm.setCurrentAndTargetValue(*bloomParam);
        wetSm.setCurrentAndTargetValue(*reverbWetParam);
        return;
    }
    /* ----------------------------------------------------- */

    {
        const float newGain = inG;
        for (int ch = 0; ch < numCh; ++ch)
            buffer.applyGainRamp(ch, 0, numSmp, prevInGain, newGain);
        prevInGain = newGain;
    }

    /* ---------- OPTIONAL MAIN-PATH HPF ------------------- */
    if (doLowCut)
    {
        juce::dsp::AudioBlock<float> lb(buffer);
        juce::dsp::ProcessContextReplacing<float> ctx(lb);
        lowCutFilter.process(ctx);
    } 
    /* ----------------------------------------------------- */

    // 2) compute & configure shelf+drive
    constexpr float intensity = 4.0f;  // tweak for more/less presence
    constexpr float maxShelfDb = 12.0f;
    constexpr float maxDriveDb = 8.0f;

    float shelfDb = bloom * maxShelfDb * intensity;
    float driveDb = bloom * maxDriveDb * intensity;

    float shelfG = juce::Decibels::decibelsToGain(shelfDb);
    float driveG = juce::Decibels::decibelsToGain(driveDb);

    *shelfFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        getSampleRate(), 10000.0f, 0.7071f, shelfG);
    driveGain.setGainLinear(driveG);

    // 3) run colour stage on a temp copy (oversampled if asked for)
    for (int ch = 0; ch < numCh; ++ch)
        colorBuffer.copyFrom(ch, 0, buffer, ch, 0, numSmp);

    juce::dsp::AudioBlock<float> baseBlock(colorBuffer);
    auto choice = static_cast<int>(oversampleParam->load() + 0.5f);

    if (choice == 1 || choice == 2)            // 2× or 4×
    {
        auto& os = (choice == 1 ? *os2x : *os4x);
        auto upBlock = os.processSamplesUp(baseBlock);          // oversample

        juce::dsp::ProcessContextReplacing<float> hiCtx(upBlock);
        shelfFilter.process(hiCtx);
        driveGain.process(hiCtx);
        softClipper.process(hiCtx);

        os.processSamplesDown(baseBlock);                       // back to 1×
    }
    else                                        // 1× (no OS)
    {
        juce::dsp::ProcessContextReplacing<float> loCtx(baseBlock);
        shelfFilter.process(loCtx);
        driveGain.process(loCtx);
        softClipper.process(loCtx);
    }

    // 4) cross-fade Bloom: dry*(1−bloom) + color*bloom
    for (int ch = 0; ch < numCh; ++ch)
    {
        auto* dry = buffer.getWritePointer(ch);
        auto* col = colorBuffer.getReadPointer(ch);
        for (int i = 0; i < numSmp; ++i)
        {
            const float b = bloomSm.getNextValue();          // ← smooth here
            dry[i] = dry[i] * (1.0f - b) + col[i] * b;
        }
    }


    // 5) optionally reverb that coloured signal
    if (atmos && wetMix > 0.0f)
    {
        // copy the just-coloured audio
        for (int ch = 0; ch < numCh; ++ch)
            reverbBuffer.copyFrom(ch, 0, buffer, ch, 0, numSmp);

        {
            juce::dsp::AudioBlock<float>        rb(reverbBuffer);
            juce::dsp::ProcessContextReplacing<float> rctx(rb);
            reverbHpf.process(rctx);
            reverbProcessor.process(rctx);
        }

        // cross-fade reverb tail in
        for (int ch = 0; ch < numCh; ++ch)
        {
            auto* dry = buffer.getWritePointer(ch);
            auto* wet = reverbBuffer.getReadPointer(ch);
            for (int i = 0; i < numSmp; ++i){
                const float w = wetSm.getNextValue();            // ← smooth here
                dry[i] = dry[i] * (1.0f - w) + wet[i] * w;
            }
        }
    }
    /* ----------------- OUTPUT GAIN (smoothed) --------------- */
    {
        const float newGain = outG * juce::Decibels::decibelsToGain(-5.0f);
        for (int ch = 0; ch < numCh; ++ch)
            buffer.applyGainRamp(ch, 0, numSmp, prevOutGain, newGain);
        prevOutGain = newGain;
    }
}

//==============================================================================
juce::AudioProcessorEditor* AirBloomAudioProcessor::createEditor()
{
    return new AirBloomAudioProcessorEditor(*this);
}

bool AirBloomAudioProcessor::hasEditor() const
{
    return true;
}

//==============================================================================
void AirBloomAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = parameters.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void AirBloomAudioProcessor::setStateInformation(const void* data,
    int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

//==============================================================================
// Factory function the host uses to create instances of your plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AirBloomAudioProcessor();
}
