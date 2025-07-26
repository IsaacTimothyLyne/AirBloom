#pragma once
#include <JuceHeader.h>

class PresetManager
{
public:
    explicit PresetManager(juce::AudioProcessorValueTreeState&);

    /* UI hooks */
    void refreshMenu(juce::ComboBox& box) const;   // just lists .abp files
    void handleSelection(const juce::String& preset);  // loads that file
    void savePresetAs(const juce::String& name);
    void deleteUserPreset(const juce::String& name);

    /* optional helper – prints current params to console */
    juce::String dumpCurrentParams() const;

private:
    juce::AudioProcessorValueTreeState& state;
    const juce::File userDir;

    /* helpers */
    juce::File userFile(const juce::String& name) const;
};
