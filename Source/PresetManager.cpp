#include "PresetManager.h"

//===========================================================================
// location:  ~/Documents/AirBloom Presets/
PresetManager::PresetManager(juce::AudioProcessorValueTreeState& s)
    : state(s),
    userDir(juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("AirBloom Presets"))
{
    userDir.createDirectory();   // make sure it exists
}

//===========================================================================
void PresetManager::refreshMenu(juce::ComboBox& box) const
{
    box.clear();
    int id = 1;

    for (auto f : userDir.findChildFiles(juce::File::findFiles, false, "*.abp"))
        box.addItem(f.getFileNameWithoutExtension(), id++);
}

// ────────────────────────────────────────────────────────────────────────
// ‣ 3.  LOAD: disk only
void PresetManager::handleSelection(const juce::String& name)
{
    if (auto xml = juce::XmlDocument::parse(userFile(name)))
        state.replaceState(juce::ValueTree::fromXml(*xml));
}

//===========================================================================
// save
void PresetManager::savePresetAs(const juce::String& name)
{
    if (name.isEmpty()) return;

    if (auto xml = state.copyState().createXml())
        xml->writeTo(userFile(name));
}

//===========================================================================
// delete
void PresetManager::deleteUserPreset(const juce::String& name)
{
    userFile(name).deleteFile();
}

//===========================================================================
// helpers
juce::File PresetManager::userFile(const juce::String& nm) const
{
    return userDir.getChildFile(nm + ".abp");
}
