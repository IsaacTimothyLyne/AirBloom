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
        menuBg = juce::ImageCache::getFromMemory(BinaryData::menu_background_png, BinaryData::menu_background_pngSize);
        alertBg = juce::ImageCache::getFromMemory(BinaryData::articulations_back_png, BinaryData::articulations_back_pngSize);
    }

    // draw your preset combo background and text as before...
    void drawComboBox(juce::Graphics& g,
        int width, int height,
        bool /*isButtonDown*/,
        int arrowX, int arrowY,
        int arrowW, int arrowH,
        juce::ComboBox& box) override
    {
        if (menuBg.isValid())
        {
            g.drawImage(menuBg,
                0, 0, width, height,
                0, 0, menuBg.getWidth(), menuBg.getHeight(),
                false);
        }
        else
        {
            g.fillAll(juce::Colours::darkgrey);
        }

        g.setColour(juce::Colours::white);
        g.setFont(14.0f);
    }

    // film-strip knob drawing unchanged...
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
            *(juce::Slider*) nullptr);
    }
    void AirBloomLookAndFeel::drawAlertBox(juce::Graphics& g,
        juce::AlertWindow& box,
        const juce::Rectangle<int>& textAreaConst,
        juce::TextLayout& layout)
    {
        // ---- background ----
        if (alertBg.isValid())
            g.drawImage(alertBg, box.getLocalBounds().toFloat(),
                juce::RectanglePlacement::fillDestination);
        else
            g.fillAll(juce::Colours::darkgrey);

        // ---- border ----
        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.drawRect(box.getLocalBounds(), 1);

        // ---- message (e.g. Name:) ----
        juce::Rectangle<int> area(textAreaConst);
        area.removeFromTop(6);                          // tiny gap
        layout.draw(g, area.toFloat());
    }
    void fillTextEditorBackground(juce::Graphics& g,
        int w, int h,
        juce::TextEditor&) override
    {
        if (menuBg.isValid())
            g.drawImage(menuBg, 0, 0, w, h,
                0, 0, menuBg.getWidth(), menuBg.getHeight());
        else
            g.fillAll(juce::Colour::fromRGB(25, 25, 25));   // fallback colour
    }

    void drawPopupMenuBackground(juce::Graphics& g,
        int w, int h) override
    {
        static auto bg = alertBg;

        if (bg.isValid())
            g.drawImage(bg, 0, 0, w, h, 0, 0, bg.getWidth(), bg.getHeight());
        else
            g.fillAll(juce::Colours::darkgrey);
    }
    void drawButtonBackground(juce::Graphics& g, juce::Button& b,
        const juce::Colour&, bool, bool isDown) override
    {
        // Only skin plain TextButtons (ToggleButtons already handled)
        if (dynamic_cast<juce::TextButton*> (&b) != nullptr)
        {
            const auto& img = isDown ? buttonOnImage : buttonOffImage;
            g.drawImage(img, b.getLocalBounds().toFloat(),
                juce::RectanglePlacement::fillDestination);
            return;                                     // done
        }

        LookAndFeel_V4::drawButtonBackground(g, b,
            b.findColour(juce::TextButton::buttonColourId), false, isDown);
    }
    // new: stretch your on/off art to fill the toggle-button component area
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& btn,
        bool /*isMouseOver*/, bool /*isButtonDown*/) override
    {
        auto bounds = btn.getLocalBounds().toFloat();
        const auto& img = btn.getToggleState() ? buttonOnImage : buttonOffImage;

        if (img.isValid())
        {
            g.drawImage(img,
                bounds,
                juce::RectanglePlacement::fillDestination);
        }
        else
        {
            LookAndFeel_V4::drawToggleButton(g, btn, false, false);
        }

        // then overlay the label text
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f, juce::Font::bold));

        // trim bottom so text sits in upper portion
        auto textArea = bounds.withTrimmedBottom(bounds.getHeight() * 0.15f);
        g.drawFittedText(btn.getButtonText(),
            textArea.getSmallestIntegerContainer(),
            juce::Justification::centred, 1);
    }

private:
    juce::Image knobStrip, buttonOnImage, buttonOffImage, menuBg, alertBg;
};
