// Source/LookAndFeel.h
#pragma once
#include <JuceHeader.h>

class AirBloomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AirBloomLookAndFeel()
    {
        knobStrip = juce::ImageCache::getFromMemory(BinaryData::knob_big1_png, BinaryData::knob_big1_pngSize);
        buttonOnImage = juce::ImageCache::getFromMemory(BinaryData::button_small_on_png, BinaryData::button_small_on_pngSize);
        buttonOffImage = juce::ImageCache::getFromMemory(BinaryData::button_small_off_png, BinaryData::button_small_off_pngSize);
    }

    // film-strip knob (unchanged)
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPosProportional, float, float, juce::Slider&) override
    {
        if (knobStrip.isValid())
        {
            constexpr int numFrames = 257;
            int frameW = knobStrip.getWidth();
            int frameH = knobStrip.getHeight() / numFrames;
            int idx = juce::jlimit(0, numFrames - 1,
                int(std::floor(sliderPosProportional * (numFrames - 1) + 0.5f)));

            g.drawImage(knobStrip,
                x, y, width, height,
                0, idx * frameH, frameW, frameH,
                false);
            return;
        }
        LookAndFeel_V4::drawRotarySlider(g, x, y, width, height,
            sliderPosProportional,
            juce::MathConstants<float>::pi * 1.2f,
            juce::MathConstants<float>::twoPi,
            *(juce::Slider*)nullptr);
    }

    // draw Atmos button + text

    void drawToggleButton(juce::Graphics& g,
        juce::ToggleButton& btn,
        bool, bool) override
    {
        auto bounds = btn.getLocalBounds();
        const auto& img = btn.getToggleState() ? buttonOnImage : buttonOffImage;

        if (img.isValid())
            g.drawImage(img, bounds.toFloat(), false);
        else
            LookAndFeel_V4::drawToggleButton(g, btn, false, false);

        // Now draw bold “ATMOS” text a bit higher
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f, juce::Font::bold));

        // Trim off the bottom 25% so the text sits up inside the top 75% of the button
        auto textArea = bounds.withTrimmedBottom(bounds.getHeight() * 0.15f);

        g.drawFittedText(btn.getButtonText(),
            textArea,
            juce::Justification::centred,
            1);
    }


private:
    juce::Image knobStrip, buttonOnImage, buttonOffImage;
};
