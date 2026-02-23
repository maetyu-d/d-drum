#pragma once

#include <array>
#include <memory>
#include <vector>

#include <juce_gui_extra/juce_gui_extra.h>

#include "PluginProcessor.h"

class BurialDrumPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit BurialDrumPluginAudioProcessorEditor(BurialDrumPluginAudioProcessor&);
    ~BurialDrumPluginAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    static constexpr int drumCount = 8;

    struct RetroLookAndFeel final : juce::LookAndFeel_V4
    {
        RetroLookAndFeel();
    };

    void configureSlider(juce::Slider& slider, juce::Label& label, const juce::String& text, bool compact = false);
    void applyPreset(int presetIndex);
    void setParameterValue(const juce::String& paramId, float plainValue);
    void stepPreset(int delta);

    BurialDrumPluginAudioProcessor& audioProcessor;
    RetroLookAndFeel retroLookAndFeel;

    juce::Label presetLabel;
    juce::ComboBox presetBox;
    juce::TextButton prevPresetButton { "Prev" };
    juce::TextButton nextPresetButton { "Next" };
    juce::TextButton testSequenceButton { "Play Test Sequence" };
    juce::Label infoLabel;

    juce::Slider tuneSlider;
    juce::Slider decaySlider;
    juce::Slider toneSlider;
    juce::Slider driveSlider;
    juce::Slider hatLengthSlider;
    juce::Slider swingSlider;

    juce::Label tuneLabel;
    juce::Label decayLabel;
    juce::Label toneLabel;
    juce::Label driveLabel;
    juce::Label hatLengthLabel;
    juce::Label swingLabel;

    std::array<juce::Label, drumCount> drumNameLabels;
    std::array<juce::TextButton, drumCount> drumTestButtons;
    std::array<juce::Slider, drumCount> drumLevelSliders;
    std::array<juce::Slider, drumCount> drumTuneSliders;
    std::array<juce::Slider, drumCount> drumDecaySliders;
    std::array<juce::Slider, drumCount> drumToneSliders;
    std::array<juce::Slider, drumCount> drumDriveSliders;
    std::array<juce::Label, drumCount> drumLevelLabels;
    std::array<juce::Label, drumCount> drumTuneLabels;
    std::array<juce::Label, drumCount> drumDecayLabels;
    std::array<juce::Label, drumCount> drumToneLabels;
    std::array<juce::Label, drumCount> drumDriveLabels;

    std::unique_ptr<SliderAttachment> tuneAttachment;
    std::unique_ptr<SliderAttachment> decayAttachment;
    std::unique_ptr<SliderAttachment> toneAttachment;
    std::unique_ptr<SliderAttachment> driveAttachment;
    std::unique_ptr<SliderAttachment> hatLengthAttachment;
    std::unique_ptr<SliderAttachment> swingAttachment;

    std::array<std::unique_ptr<SliderAttachment>, drumCount> drumLevelAttachments;
    std::array<std::unique_ptr<SliderAttachment>, drumCount> drumTuneAttachments;
    std::array<std::unique_ptr<SliderAttachment>, drumCount> drumDecayAttachments;
    std::array<std::unique_ptr<SliderAttachment>, drumCount> drumToneAttachments;
    std::array<std::unique_ptr<SliderAttachment>, drumCount> drumDriveAttachments;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BurialDrumPluginAudioProcessorEditor)
};
