#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "PresetManager.h"
std::unique_ptr<PresetManager> presetMgr;

AirBloomAudioProcessorEditor::AirBloomAudioProcessorEditor(AirBloomAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // Look & Feel
    lookAndFeel = std::make_unique<AirBloomLookAndFeel>();
    setLookAndFeel(lookAndFeel.get());
    juce::LookAndFeel::setDefaultLookAndFeel(lookAndFeel.get());

    // Background
    backgroundImage = juce::ImageCache::getFromMemory(
        BinaryData::new_background_png, BinaryData::new_background_pngSize);

    // ── heading font ──────────────────────────────────────────
    // ── heading font ──────────────────────────────────────────
    juce::Typeface::Ptr tf =
        juce::Typeface::createSystemTypefaceFor(BinaryData::Super_Vanilla_ttf,
            BinaryData::Super_Vanilla_ttfSize);

    juce::Font panelFont(tf);
    panelFont.setHeight(22.0f);
    //panelFont.setBold(true);

    // ── column titles ─────────────────────────────────────────
    addAndMakeVisible(panelLeft);
    panelLeft.setText("Input", juce::dontSendNotification);
    panelLeft.setFont(panelFont);
    panelLeft.setColour(juce::Label::textColourId, juce::Colours::white);
    panelLeft.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(panelCentre);
    panelCentre.setText("Colour & Space", juce::dontSendNotification);
    panelCentre.setFont(panelFont);
    panelCentre.setColour(juce::Label::textColourId, juce::Colours::white);
    panelCentre.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(panelRight);
    panelRight.setText("Output /\nUtility", juce::dontSendNotification); // line-break
    panelRight.setFont(panelFont);
    panelRight.setColour(juce::Label::textColourId, juce::Colours::white);
    panelRight.setJustificationType(juce::Justification::centred);


    // Preset box
    presetBox.addItem("Flat", 1);
    presetBox.addItem("Subtle", 2);
    presetBox.addItem("Extreme", 3);
    presetBox.setSelectedId(1);
    addAndMakeVisible(presetBox);

    // ── Oversample selector ───────────────────────────────────
    oversampleBox.addItem(juce::String::fromUTF8(u8"1×"), 1);
    oversampleBox.addItem(juce::String::fromUTF8(u8"2×"), 2);
    oversampleBox.addItem(juce::String::fromUTF8(u8"4×"), 3);
    oversampleBox.setSelectedId(1);
    addAndMakeVisible(oversampleBox);

    oversampleAttachment = std::make_unique<ChoiceAtt>(
        processorRef.parameters, "oversample", oversampleBox);


    // Bypass / Oversample / Low-Cut
    bypassButton.setButtonText("Bypass");     addAndMakeVisible(bypassButton);
    lowCutButton.setButtonText("Low-Cut");    addAndMakeVisible(lowCutButton);

    // Bloom knob
    bloomSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    bloomSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(bloomSlider);
    bloomAttachment = std::make_unique<BloomAttachment>(
        processorRef.parameters, "bloom", bloomSlider);

    bloomLabel.setText("Bloom", juce::dontSendNotification);
    bloomLabel.setJustificationType(juce::Justification::centred);
    bloomLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(bloomLabel);

    // Reverb-Wet knob
    reverbWetSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    reverbWetSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(reverbWetSlider);
    reverbWetAttachment = std::make_unique<WetAttachment>(
        processorRef.parameters, "reverbWet", reverbWetSlider);

    reverbLabel.setText("Reverb Wet", juce::dontSendNotification);
    reverbLabel.setJustificationType(juce::Justification::centred);
    reverbLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(reverbLabel);

    // Input-gain knob
    inputGainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    inputGainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(inputGainSlider);
    inputGainAttachment = std::make_unique<GainAttachment>(
        processorRef.parameters, "inputGain", inputGainSlider);

    inputGainLabel.setText("Input Gain", juce::dontSendNotification);
    inputGainLabel.setJustificationType(juce::Justification::centred);
    inputGainLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(inputGainLabel);

    // Output-gain knob
    outputGainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    outputGainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(outputGainSlider);
    outputGainAttachment = std::make_unique<GainAttachment>(
        processorRef.parameters, "outputGain", outputGainSlider);

    outputGainLabel.setText("Output Gain", juce::dontSendNotification);
    outputGainLabel.setJustificationType(juce::Justification::centred);
    outputGainLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(outputGainLabel);

    // PluginEditor.cpp  (inside constructor, just after button creation)
    bypassAttachment = std::make_unique<BtnAtt>(processorRef.parameters,
        "bypass", bypassButton);
    lowCutAttachment = std::make_unique<BtnAtt>(processorRef.parameters,
        "lowCut", lowCutButton);

    presetMgr = std::make_unique<PresetManager>(processorRef.parameters);
    presetMgr->refreshMenu(presetBox);

    presetBox.onChange = [this]
    {
        presetMgr->handleSelection(presetBox.getText());
    };

    auto logo = juce::ImageCache::getFromMemory(BinaryData::ASPIRE_AUDIO_png,
        BinaryData::ASPIRE_AUDIO_pngSize);
    aspireLogo.setImage(logo, juce::RectanglePlacement::centred);
    addAndMakeVisible(aspireLogo);

#if DEV_PRESET_SAVE
    /* -------------------------------------------------------------- */
    /*  TEMPORARY “save preset” button                                */
    /* -------------------------------------------------------------- */
    addAndMakeVisible(saveBtn);
    saveBtn.setClickingTogglesState(false);
    saveBtn.setTooltip("Save current settings as a preset");

    saveBtn.onClick = [this]
        {
            juce::AlertWindow w("Save preset", "Name:", juce::AlertWindow::NoIcon, this);
            w.addTextEditor("name", presetBox.getText());
            w.addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));

            if (w.runModalLoop() == 1)
            {
                auto nm = w.getTextEditor("name")->getText().trim();
                if (nm.isNotEmpty())
                {
                    presetMgr->savePresetAs(nm);
                    presetMgr->refreshMenu(presetBox);
                    presetBox.setText(nm, juce::dontSendNotification);
                }
            }
        };
#endif
    
    setSize(512, 512);
}
AirBloomAudioProcessorEditor::~AirBloomAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void AirBloomAudioProcessorEditor::paint(juce::Graphics& g)
{
    if (backgroundImage.isValid())
        g.drawImage(backgroundImage, getLocalBounds().toFloat(), juce::RectanglePlacement::fillDestination);
    else
        g.fillAll(juce::Colours::darkslategrey);
}

void AirBloomAudioProcessorEditor::resized()
{
    // ── constants ──────────────────────────────────────────
    constexpr int edgePad = 16;
    constexpr int panelGap = 8;
    constexpr int bigBtnW = 100;
    constexpr int bigBtnH = 68;
    constexpr int yShift = 18;

    // ── header layout ──────────────────────────────────────
    auto full = getLocalBounds().reduced(edgePad);
    auto header = full.removeFromTop(topBarHeight);

    constexpr int presetW = 300;     // slimmer preset menu
    constexpr int presetH = 40;
    constexpr int overW = 100;     // oversample selector
    constexpr int overH = 40;
    constexpr int gapW = 12;

    const int groupW = presetW + gapW + overW;
    const int groupX = (getWidth() - groupW) / 2;
    const int groupY = header.getCentreY() - presetH / 2 + 8;

    // centred preset-oversample group
    presetBox.setBounds(groupX,
        groupY,
        presetW, presetH);

    oversampleBox.setBounds(groupX + presetW + gapW,
        groupY,
        overW, overH);

    // ── three columns below header ─────────────────────────
    auto panels = full.reduced(0, 40);           // top cushion
    const int colW = (panels.getWidth() - 2 * panelGap) / 3;

    auto leftCol = panels.removeFromLeft(colW);
    panels.removeFromLeft(panelGap);
    auto centreCol = panels.removeFromLeft(colW);
    panels.removeFromLeft(panelGap);
    auto rightCol = panels;

    auto centreX = [](const juce::Rectangle<int>& r, int w)
        { return r.getCentreX() - w / 2; };

    const int titleH = 22;               // same as font height + padding
    const int titleY = header.getBottom() + 18 + yShift;

    panelLeft.setBounds(leftCol.getX(), titleY, leftCol.getWidth(), titleH);
    panelCentre.setBounds(centreCol.getX(), titleY, centreCol.getWidth(), titleH);
    panelRight.setBounds(rightCol.getX(), titleY, rightCol.getWidth(), titleH + 22);


    // ── LEFT column ────────────────────────────────────────
    {
        int y = leftCol.getY() + 40 + yShift;

        inputGainSlider.setBounds(centreX(leftCol, 80), y, 80, 80);
        inputGainLabel.setBounds(inputGainSlider.getX(),
            inputGainSlider.getBottom(), 80, 20);

        y += 165;
        bypassButton.setBounds(centreX(leftCol, bigBtnW), y,
            bigBtnW, bigBtnH);
    }

    // ── CENTRE column ──────────────────────────────────────
    {
        int y = centreCol.getY() + 10 + yShift;

        bloomSlider.setBounds(centreX(centreCol, 180), y, 180, 180);
        bloomLabel.setBounds(bloomSlider.getX(),
            bloomSlider.getBottom() - 8, // was +0 → +8 px
            180, 20);

        y += 198;                        // adjust for new gap
        reverbWetSlider.setBounds(centreX(centreCol, 0), y, 80, 80);
        reverbLabel.setBounds(reverbWetSlider.getX(),
            reverbWetSlider.getBottom(), 80, 20);
    }

    // ── RIGHT column ───────────────────────────────────────
    {
        int y = rightCol.getY() + 40 + yShift + 22;

        lowCutButton.setBounds(centreX(rightCol, bigBtnW), y,
            bigBtnW, bigBtnH);

        y += 130;
        outputGainSlider.setBounds(centreX(rightCol, 80), y, 80, 80);
        outputGainLabel.setBounds(outputGainSlider.getX(),
            outputGainSlider.getBottom(), 80, 20);
    }

    // BOTTOM LOGO
    constexpr int logoW = 256;   // scale-down width (half of 256)
    constexpr int logoH = 40;    // scale-down height (half of 40)
    aspireLogo.setBounds(getWidth() - edgePad - logoW,
        getHeight() - edgePad - logoH + 20, // +4 to move down
        logoW, logoH);

#if DEV_PRESET_SAVE
    const int  saveW = 60, saveH = 24;
    const int  xSave = oversampleBox.getRight() + 8;
    const int  ySave = oversampleBox.getY() + (oversampleBox.getHeight() - saveH) / 2;
    saveBtn.setBounds(xSave, ySave, saveW, saveH);
#endif
}
