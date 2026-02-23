#include "PluginEditor.h"

namespace
{
const juce::Colour uiBg { 0xff030502 };
const juce::Colour uiPhosphor { 0xff8dff67 };
const juce::Colour uiPhosphorDim { 0xff4f9f3a };
const juce::Colour uiPanel { 0xff050a03 };

constexpr std::array<const char*, 8> drumNames {
    "Kick", "Snare", "Closed Hat", "Open Hat", "Crash", "Ride", "Clap", "Rim"
};

constexpr std::array<const char*, 8> drumPrefixes {
    "kick", "snare", "closedHat", "openHat", "crash", "ride", "clap", "rim"
};

constexpr std::array<BurialDrumPluginAudioProcessor::DrumType, 8> drumTypes {
    BurialDrumPluginAudioProcessor::DrumType::kick,
    BurialDrumPluginAudioProcessor::DrumType::snare,
    BurialDrumPluginAudioProcessor::DrumType::closedHat,
    BurialDrumPluginAudioProcessor::DrumType::openHat,
    BurialDrumPluginAudioProcessor::DrumType::crash,
    BurialDrumPluginAudioProcessor::DrumType::ride,
    BurialDrumPluginAudioProcessor::DrumType::clap,
    BurialDrumPluginAudioProcessor::DrumType::rim
};

struct PresetData
{
    const char* name = "";
    float globalTune = 0.0f;
    float globalDecay = 1.0f;
    float globalTone = 0.5f;
    float globalDrive = 0.25f;
    float globalHatLength = 1.0f;
    float globalSwing = 0.0f;
    std::array<float, 8> level { 1, 1, 1, 1, 1, 1, 1, 1 };
    std::array<float, 8> tune { 0, 0, 0, 0, 0, 0, 0, 0 };
    std::array<float, 8> decay { 1, 1, 1, 1, 1, 1, 1, 1 };
    std::array<float, 8> tone { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };
    std::array<float, 8> drive { 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f };
};

const std::array<PresetData, 18> presets {{
    { "Go Plastic",      1.2f, 0.54f, 0.86f, 0.66f, 0.44f, 0.30f, { 1.42f, 1.26f, 1.06f, 0.96f, 0.30f, 0.30f, 1.20f, 1.14f }, { -2.2f, 4.6f, 8.2f, 7.4f, 1.0f, 0.8f, 4.8f, 7.0f }, { 0.56f, 0.44f, 0.40f, 0.40f, 0.34f, 0.34f, 0.48f, 0.42f }, { 0.62f, 0.86f, 0.90f, 0.86f, 0.56f, 0.54f, 0.80f, 0.84f }, { 0.88f, 0.74f, 0.40f, 0.34f, 0.18f, 0.18f, 0.66f, 0.72f } },
    { "Amen Raze",      6.8f, 0.34f, 1.00f, 0.48f, 0.22f, 0.78f, { 1.08f, 1.18f, 1.24f, 1.10f, 0.24f, 0.22f, 1.00f, 1.30f }, { 4.2f, 7.8f, 11.6f, 10.8f, 2.6f, 2.2f, 7.4f, 11.8f }, { 0.42f, 0.34f, 0.30f, 0.30f, 0.32f, 0.32f, 0.36f, 0.34f }, { 0.86f, 1.00f, 1.00f, 1.00f, 0.74f, 0.70f, 0.92f, 1.00f }, { 0.54f, 0.62f, 0.32f, 0.26f, 0.14f, 0.14f, 0.58f, 0.64f } },
    { "FM Melt",       -4.4f, 1.54f, 0.14f, 0.74f, 1.54f, 0.46f, { 1.26f, 1.04f, 1.08f, 1.14f, 0.42f, 0.40f, 0.92f, 0.82f }, { -6.0f, -3.2f, 3.0f, 3.6f, -2.2f, -2.6f, -1.2f, -1.8f }, { 1.52f, 1.10f, 1.30f, 1.44f, 0.90f, 0.86f, 1.02f, 0.90f }, { 0.14f, 0.28f, 0.46f, 0.52f, 0.34f, 0.30f, 0.24f, 0.20f }, { 0.84f, 0.58f, 0.34f, 0.36f, 0.18f, 0.16f, 0.46f, 0.38f } },
    { "Plaid Razor",   -7.6f, 1.20f, 0.06f, 0.96f, 0.28f, 0.90f, { 1.50f, 1.16f, 0.56f, 0.50f, 0.18f, 0.18f, 0.90f, 1.18f }, { -10.4f, -5.0f, -3.0f, -3.8f, -8.8f, -8.2f, -2.6f, 5.2f }, { 1.74f, 1.26f, 0.54f, 0.50f, 0.34f, 0.34f, 0.92f, 0.84f }, { 0.04f, 0.12f, 0.24f, 0.22f, 0.14f, 0.12f, 0.18f, 0.52f }, { 1.00f, 0.84f, 0.20f, 0.18f, 0.10f, 0.10f, 0.52f, 0.66f } },
    { "Venetian Sub",  -2.6f, 0.72f, 0.56f, 1.00f, 0.60f, 0.18f, { 1.50f, 1.28f, 0.86f, 0.80f, 0.24f, 0.24f, 1.18f, 1.24f }, { -4.0f, 5.4f, -2.8f, 6.0f, -7.4f, 8.0f, -1.8f, 7.8f }, { 0.78f, 0.66f, 0.46f, 0.44f, 0.32f, 0.32f, 0.58f, 0.52f }, { 0.26f, 0.78f, 0.34f, 0.84f, 0.32f, 0.76f, 0.40f, 0.88f }, { 1.00f, 0.84f, 0.36f, 0.34f, 0.12f, 0.12f, 0.78f, 0.84f } },
    { "Hard Sync",      4.8f, 0.44f, 0.94f, 0.38f, 0.30f, 0.62f, { 0.96f, 1.12f, 1.18f, 1.02f, 0.26f, 0.24f, 1.04f, 1.18f }, { 2.6f, 6.2f, 10.2f, 9.6f, 1.2f, 1.0f, 6.6f, 9.4f }, { 0.50f, 0.40f, 0.34f, 0.34f, 0.30f, 0.30f, 0.40f, 0.38f }, { 0.78f, 0.96f, 1.00f, 0.96f, 0.62f, 0.58f, 0.88f, 0.96f }, { 0.46f, 0.56f, 0.34f, 0.30f, 0.14f, 0.14f, 0.56f, 0.60f } },
    { "Micro Edit",     0.2f, 0.66f, 0.72f, 0.58f, 0.36f, 1.00f, { 1.18f, 1.22f, 0.96f, 0.86f, 0.20f, 0.20f, 1.14f, 1.20f }, { -1.6f, 4.2f, 8.4f, 7.8f, -6.8f, 7.0f, 4.2f, 8.8f }, { 0.62f, 0.50f, 0.40f, 0.38f, 0.30f, 0.30f, 0.44f, 0.40f }, { 0.46f, 0.88f, 0.94f, 0.90f, 0.34f, 0.72f, 0.82f, 0.92f }, { 0.74f, 0.70f, 0.36f, 0.30f, 0.12f, 0.14f, 0.70f, 0.76f } },
    { "Data Swerve",   -0.8f, 0.92f, 0.42f, 0.84f, 0.82f, 0.50f, { 1.34f, 1.14f, 1.06f, 0.96f, 0.32f, 0.30f, 1.04f, 1.06f }, { -2.8f, 3.4f, 6.2f, 6.6f, -3.6f, 3.6f, 1.2f, 5.8f }, { 0.94f, 0.82f, 0.72f, 0.76f, 0.40f, 0.40f, 0.72f, 0.68f }, { 0.24f, 0.64f, 0.86f, 0.84f, 0.44f, 0.62f, 0.56f, 0.70f }, { 0.68f, 0.56f, 0.28f, 0.26f, 0.12f, 0.12f, 0.48f, 0.56f } },
    { "Servo Funk",     2.2f, 0.48f, 0.90f, 0.56f, 0.30f, 0.54f, { 1.20f, 1.24f, 1.12f, 1.00f, 0.28f, 0.26f, 1.10f, 1.16f }, { 0.6f, 5.8f, 10.0f, 9.2f, 0.8f, 0.4f, 6.0f, 8.6f }, { 0.54f, 0.44f, 0.36f, 0.36f, 0.30f, 0.30f, 0.42f, 0.38f }, { 0.70f, 0.92f, 1.00f, 0.94f, 0.54f, 0.50f, 0.86f, 0.90f }, { 0.62f, 0.68f, 0.38f, 0.32f, 0.14f, 0.14f, 0.64f, 0.68f } },
    { "Acid Ghost",    -5.6f, 1.64f, 0.08f, 0.62f, 1.72f, 0.36f, { 1.14f, 0.92f, 1.24f, 1.32f, 0.62f, 0.58f, 0.88f, 0.78f }, { -6.8f, -2.6f, 1.6f, 2.0f, -0.8f, -1.0f, -1.2f, -1.4f }, { 1.46f, 1.04f, 1.82f, 1.96f, 1.20f, 1.14f, 1.12f, 1.02f }, { 0.08f, 0.22f, 0.40f, 0.44f, 0.36f, 0.32f, 0.24f, 0.20f }, { 0.66f, 0.44f, 0.30f, 0.32f, 0.16f, 0.16f, 0.36f, 0.32f } },
    { "Granular Rush",  7.4f, 0.30f, 1.00f, 0.30f, 0.20f, 0.88f, { 0.86f, 1.02f, 1.30f, 1.22f, 0.34f, 0.32f, 0.92f, 1.28f }, { 5.0f, 9.4f, 12.0f, 11.4f, 3.4f, 2.8f, 8.0f, 12.0f }, { 0.38f, 0.32f, 0.30f, 0.30f, 0.30f, 0.30f, 0.34f, 0.30f }, { 0.94f, 1.00f, 1.00f, 1.00f, 0.82f, 0.78f, 0.96f, 1.00f }, { 0.34f, 0.48f, 0.32f, 0.28f, 0.12f, 0.12f, 0.46f, 0.56f } },
    { "Live Core",     -1.2f, 0.82f, 0.64f, 0.92f, 0.50f, 0.70f, { 1.36f, 1.22f, 0.94f, 0.86f, 0.22f, 0.22f, 1.16f, 1.22f }, { -2.6f, 5.8f, -5.8f, 6.8f, -9.0f, 9.8f, -3.2f, 9.2f }, { 0.82f, 0.70f, 0.50f, 0.46f, 0.30f, 0.30f, 0.60f, 0.54f }, { 0.24f, 0.84f, 0.22f, 0.88f, 0.26f, 0.82f, 0.34f, 0.90f }, { 1.00f, 0.82f, 0.40f, 0.36f, 0.10f, 0.10f, 0.84f, 0.88f } },
    { "Session Whip",   0.8f, 0.46f, 0.92f, 0.62f, 0.30f, 0.18f, { 1.50f, 1.34f, 1.00f, 0.90f, 0.18f, 0.18f, 1.08f, 1.10f }, { -1.2f, 4.2f, 8.8f, 8.2f, 0.4f, 0.2f, 4.4f, 6.2f }, { 0.44f, 0.34f, 0.30f, 0.30f, 0.30f, 0.30f, 0.36f, 0.34f }, { 0.78f, 0.96f, 0.94f, 0.90f, 0.58f, 0.56f, 0.84f, 0.88f }, { 0.92f, 0.82f, 0.36f, 0.30f, 0.12f, 0.12f, 0.62f, 0.66f } },
    { "Tight Pocket",  -0.6f, 0.52f, 0.84f, 0.56f, 0.34f, 0.26f, { 1.44f, 1.26f, 0.96f, 0.88f, 0.20f, 0.20f, 1.04f, 1.08f }, { -2.4f, 3.8f, 7.6f, 7.0f, -0.2f, -0.2f, 3.8f, 5.6f }, { 0.48f, 0.36f, 0.32f, 0.32f, 0.30f, 0.30f, 0.38f, 0.34f }, { 0.70f, 0.90f, 0.88f, 0.84f, 0.52f, 0.50f, 0.78f, 0.84f }, { 0.84f, 0.74f, 0.34f, 0.28f, 0.12f, 0.12f, 0.56f, 0.62f } },
    { "Crack Driver",   2.0f, 0.40f, 1.00f, 0.78f, 0.26f, 0.36f, { 1.36f, 1.42f, 1.08f, 0.98f, 0.22f, 0.22f, 1.16f, 1.24f }, { -0.8f, 6.0f, 10.2f, 9.6f, 1.0f, 0.8f, 6.4f, 8.8f }, { 0.40f, 0.32f, 0.30f, 0.30f, 0.30f, 0.30f, 0.34f, 0.32f }, { 0.88f, 1.00f, 1.00f, 0.96f, 0.62f, 0.60f, 0.92f, 0.98f }, { 1.00f, 0.92f, 0.38f, 0.32f, 0.14f, 0.14f, 0.72f, 0.78f } },
    { "Dry Room Kit",  -1.4f, 0.58f, 0.72f, 0.48f, 0.28f, 0.10f, { 1.32f, 1.20f, 0.92f, 0.82f, 0.16f, 0.16f, 1.00f, 1.04f }, { -2.8f, 3.2f, 6.8f, 6.0f, -0.8f, -1.0f, 3.0f, 4.8f }, { 0.54f, 0.42f, 0.34f, 0.34f, 0.30f, 0.30f, 0.42f, 0.38f }, { 0.58f, 0.82f, 0.80f, 0.76f, 0.46f, 0.44f, 0.70f, 0.76f }, { 0.70f, 0.62f, 0.30f, 0.24f, 0.10f, 0.10f, 0.46f, 0.52f } },
    { "Punchline",      1.4f, 0.42f, 0.96f, 0.70f, 0.24f, 0.48f, { 1.50f, 1.36f, 1.04f, 0.92f, 0.20f, 0.20f, 1.14f, 1.18f }, { -0.4f, 5.2f, 9.6f, 9.0f, 0.8f, 0.6f, 5.8f, 7.8f }, { 0.42f, 0.32f, 0.30f, 0.30f, 0.30f, 0.30f, 0.34f, 0.32f }, { 0.84f, 1.00f, 0.98f, 0.94f, 0.60f, 0.58f, 0.90f, 0.94f }, { 0.96f, 0.86f, 0.36f, 0.30f, 0.12f, 0.12f, 0.68f, 0.74f } },
    { "Metal Sticks",   3.6f, 0.36f, 0.98f, 0.60f, 0.22f, 0.64f, { 1.26f, 1.30f, 1.24f, 1.08f, 0.26f, 0.24f, 1.06f, 1.26f }, { 1.0f, 6.8f, 11.4f, 10.8f, 1.6f, 1.4f, 6.8f, 10.0f }, { 0.38f, 0.30f, 0.30f, 0.30f, 0.30f, 0.30f, 0.32f, 0.30f }, { 0.90f, 1.00f, 1.00f, 1.00f, 0.68f, 0.66f, 0.94f, 1.00f }, { 0.78f, 0.74f, 0.34f, 0.28f, 0.14f, 0.14f, 0.60f, 0.72f } }
}};
} // namespace

BurialDrumPluginAudioProcessorEditor::RetroLookAndFeel::RetroLookAndFeel()
{
    setDefaultSansSerifTypefaceName("Menlo");
}

BurialDrumPluginAudioProcessorEditor::BurialDrumPluginAudioProcessorEditor(BurialDrumPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&retroLookAndFeel);
    setSize(1040, 640);

    presetLabel.setText("PRESET", juce::dontSendNotification);
    presetLabel.setColour(juce::Label::textColourId, uiPhosphor);
    presetLabel.setFont(juce::Font(juce::FontOptions(12.0f).withStyle("Bold")));
    addAndMakeVisible(presetLabel);

    presetBox.setColour(juce::ComboBox::backgroundColourId, uiPanel);
    presetBox.setColour(juce::ComboBox::textColourId, uiPhosphor);
    presetBox.setColour(juce::ComboBox::outlineColourId, uiPhosphorDim);
    presetBox.setColour(juce::ComboBox::arrowColourId, uiPhosphor);
    for (size_t i = 0; i < presets.size(); ++i)
        presetBox.addItem(presets[i].name, static_cast<int>(i + 1));
    presetBox.onChange = [this]
    {
        const int selected = presetBox.getSelectedId();
        if (selected > 0)
            applyPreset(selected - 1);
    };
    addAndMakeVisible(presetBox);

    prevPresetButton.onClick = [this] { stepPreset(-1); };
    nextPresetButton.onClick = [this] { stepPreset(1); };
    prevPresetButton.setButtonText("<");
    nextPresetButton.setButtonText(">");
    prevPresetButton.setColour(juce::TextButton::buttonColourId, uiPanel);
    nextPresetButton.setColour(juce::TextButton::buttonColourId, uiPanel);
    prevPresetButton.setColour(juce::TextButton::textColourOffId, uiPhosphor);
    nextPresetButton.setColour(juce::TextButton::textColourOffId, uiPhosphor);
    addAndMakeVisible(prevPresetButton);
    addAndMakeVisible(nextPresetButton);

    testSequenceButton.setButtonText("TEST");
    testSequenceButton.setColour(juce::TextButton::buttonColourId, uiPanel);
    testSequenceButton.setColour(juce::TextButton::buttonOnColourId, uiPanel);
    testSequenceButton.setColour(juce::TextButton::textColourOffId, uiPhosphor);
    testSequenceButton.setColour(juce::TextButton::textColourOnId, uiPhosphor);
    testSequenceButton.onClick = [this] { audioProcessor.startTestSequence(); };
    addAndMakeVisible(testSequenceButton);

    infoLabel.setJustificationType(juce::Justification::topLeft);
    infoLabel.setFont(juce::Font(juce::FontOptions(12.0f)));
    infoLabel.setColour(juce::Label::textColourId, uiPhosphorDim);
    infoLabel.setText(
        "MIDI map: 36 Kick, 38 Snare, 42 CH, 46 OH, 49 Crash, 51 Ride, 39 Clap, 37 Rim",
        juce::dontSendNotification);
    addAndMakeVisible(infoLabel);

    configureSlider(tuneSlider, tuneLabel, "Tune");
    configureSlider(decaySlider, decayLabel, "Decay");
    configureSlider(toneSlider, toneLabel, "Tone");
    configureSlider(driveSlider, driveLabel, "Drive");
    configureSlider(hatLengthSlider, hatLengthLabel, "Hat Len");
    configureSlider(swingSlider, swingLabel, "Swing");

    auto& apvts = audioProcessor.getAPVTS();
    tuneAttachment = std::make_unique<SliderAttachment>(apvts, "tune", tuneSlider);
    decayAttachment = std::make_unique<SliderAttachment>(apvts, "decay", decaySlider);
    toneAttachment = std::make_unique<SliderAttachment>(apvts, "tone", toneSlider);
    driveAttachment = std::make_unique<SliderAttachment>(apvts, "drive", driveSlider);
    hatLengthAttachment = std::make_unique<SliderAttachment>(apvts, "hatLength", hatLengthSlider);
    swingAttachment = std::make_unique<SliderAttachment>(apvts, "swing", swingSlider);

    for (size_t i = 0; i < drumCount; ++i)
    {
        drumNameLabels[i].setText(drumNames[i], juce::dontSendNotification);
        drumNameLabels[i].setJustificationType(juce::Justification::centred);
        drumNameLabels[i].setColour(juce::Label::textColourId, uiPhosphor);
        drumNameLabels[i].setFont(juce::Font(juce::FontOptions(12.0f).withStyle("Bold")));
        addAndMakeVisible(drumNameLabels[i]);

        drumTestButtons[i].setButtonText("T");
        drumTestButtons[i].setColour(juce::TextButton::buttonColourId, uiPanel);
        drumTestButtons[i].setColour(juce::TextButton::buttonOnColourId, uiPanel);
        drumTestButtons[i].setColour(juce::TextButton::textColourOffId, uiPhosphor);
        drumTestButtons[i].onClick = [this, i] { audioProcessor.queueDrumTestHit(drumTypes[i]); };
        addAndMakeVisible(drumTestButtons[i]);

        configureSlider(drumLevelSliders[i], drumLevelLabels[i], "Level", true);
        configureSlider(drumTuneSliders[i], drumTuneLabels[i], "Tune", true);
        configureSlider(drumDecaySliders[i], drumDecayLabels[i], "Decay", true);
        configureSlider(drumToneSliders[i], drumToneLabels[i], "Tone", true);
        configureSlider(drumDriveSliders[i], drumDriveLabels[i], "Drive", true);

        const juce::String prefix(drumPrefixes[i]);
        drumLevelAttachments[i] = std::make_unique<SliderAttachment>(apvts, prefix + "Level", drumLevelSliders[i]);
        drumTuneAttachments[i] = std::make_unique<SliderAttachment>(apvts, prefix + "Tune", drumTuneSliders[i]);
        drumDecayAttachments[i] = std::make_unique<SliderAttachment>(apvts, prefix + "Decay", drumDecaySliders[i]);
        drumToneAttachments[i] = std::make_unique<SliderAttachment>(apvts, prefix + "Tone", drumToneSliders[i]);
        drumDriveAttachments[i] = std::make_unique<SliderAttachment>(apvts, prefix + "Drive", drumDriveSliders[i]);
    }

    presetBox.setSelectedId(1, juce::sendNotificationSync);
}

BurialDrumPluginAudioProcessorEditor::~BurialDrumPluginAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void BurialDrumPluginAudioProcessorEditor::configureSlider(juce::Slider& slider, juce::Label& label, const juce::String& text, bool compact)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, compact ? 48 : 58, compact ? 16 : 18);
    slider.setColour(juce::Slider::rotarySliderFillColourId, uiPhosphor);
    slider.setColour(juce::Slider::thumbColourId, uiPhosphor);
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, uiPhosphorDim);
    slider.setColour(juce::Slider::trackColourId, uiPhosphorDim);
    slider.setColour(juce::Slider::textBoxTextColourId, uiPhosphor);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, uiPanel);
    slider.setColour(juce::Slider::textBoxOutlineColourId, uiPhosphorDim);
    addAndMakeVisible(slider);

    label.setText(text.toUpperCase(), juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, compact ? uiPhosphorDim : uiPhosphor);
    label.setFont(juce::Font(juce::FontOptions(compact ? 11.0f : 12.0f)));
    addAndMakeVisible(label);
}

void BurialDrumPluginAudioProcessorEditor::setParameterValue(const juce::String& paramId, float plainValue)
{
    auto* parameter = audioProcessor.getAPVTS().getParameter(paramId);
    auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(parameter);
    if (ranged == nullptr)
        return;

    const float normalized = ranged->convertTo0to1(plainValue);
    ranged->beginChangeGesture();
    ranged->setValueNotifyingHost(normalized);
    ranged->endChangeGesture();
}

void BurialDrumPluginAudioProcessorEditor::applyPreset(int presetIndex)
{
    if (presetIndex < 0 || presetIndex >= static_cast<int>(presets.size()))
        return;

    const auto& preset = presets[static_cast<size_t>(presetIndex)];
    setParameterValue("tune", preset.globalTune);
    setParameterValue("decay", preset.globalDecay);
    setParameterValue("tone", preset.globalTone);
    setParameterValue("drive", preset.globalDrive);
    setParameterValue("hatLength", preset.globalHatLength);
    setParameterValue("swing", preset.globalSwing);

    for (size_t i = 0; i < drumCount; ++i)
    {
        const juce::String prefix(drumPrefixes[i]);
        setParameterValue(prefix + "Level", preset.level[i]);
        setParameterValue(prefix + "Tune", preset.tune[i]);
        setParameterValue(prefix + "Decay", preset.decay[i]);
        setParameterValue(prefix + "Tone", preset.tone[i]);
        setParameterValue(prefix + "Drive", preset.drive[i]);
    }
}

void BurialDrumPluginAudioProcessorEditor::stepPreset(int delta)
{
    const int count = static_cast<int>(presets.size());
    int current = presetBox.getSelectedId() - 1;
    if (current < 0)
        current = 0;

    current = (current + delta + count) % count;
    presetBox.setSelectedId(current + 1, juce::sendNotificationSync);
}

void BurialDrumPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(uiBg);

    // Subtle horizontal scanline texture.
    g.setColour(uiPhosphorDim.withAlpha(0.08f));
    for (int y = 0; y < getHeight(); y += 3)
        g.drawHorizontalLine(y, 0.0f, static_cast<float>(getWidth()));

    auto area = getLocalBounds().reduced(12);

    g.setColour(uiPanel);
    g.fillRect(area);
    g.setColour(uiPhosphorDim);
    g.drawRect(area, 1);

    auto topBand = area.removeFromTop(58);
    g.setColour(uiPhosphor);
    g.setFont(juce::Font(juce::FontOptions(22.0f).withStyle("Bold")));
    g.drawText("D-DRUM MACHINE", topBand.removeFromLeft(388).translated(10, 0), juce::Justification::centredLeft, false);
    g.setColour(uiPhosphorDim);
    g.drawLine(static_cast<float>(area.getX()), static_cast<float>(topBand.getBottom() + 1), static_cast<float>(area.getRight()), static_cast<float>(topBand.getBottom() + 1), 1.0f);

    auto globalPanel = area.removeFromTop(144);
    g.setColour(uiPanel.brighter(0.03f));
    g.fillRect(globalPanel);
    g.setColour(uiPhosphorDim);
    g.drawRect(globalPanel, 1);

    g.setColour(uiPhosphor);
    g.setFont(juce::Font(juce::FontOptions(15.0f).withStyle("Bold")));
    g.drawText("GLOBAL", globalPanel.removeFromTop(24).withTrimmedLeft(10), juce::Justification::centredLeft, false);

    area.removeFromTop(8);

    auto drumsArea = area;
    const int cardGap = 8;
    const int cardWidth = (drumsArea.getWidth() - (cardGap * 3)) / 4;
    const int cardHeight = (drumsArea.getHeight() - cardGap) / 2;

    for (int row = 0; row < 2; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            const int index = row * 4 + col;
            auto card = juce::Rectangle<int>(
                drumsArea.getX() + col * (cardWidth + cardGap),
                drumsArea.getY() + row * (cardHeight + cardGap),
                cardWidth,
                cardHeight);

            g.setColour(uiPanel.brighter(0.02f + 0.01f * static_cast<float>(index / 4)));
            g.fillRect(card);
            g.setColour(uiPhosphorDim.withAlpha(0.9f));
            g.drawRect(card, 1);
        }
    }
}

void BurialDrumPluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(12);

    auto topBar = bounds.removeFromTop(58);
    auto controls = topBar.removeFromRight(382);
    controls = controls.withTrimmedTop(3).withTrimmedBottom(3);
    presetLabel.setBounds(controls.removeFromLeft(50));
    controls.removeFromLeft(4);
    prevPresetButton.setBounds(controls.removeFromLeft(56));
    controls.removeFromLeft(6);
    presetBox.setBounds(controls.removeFromLeft(214));
    controls.removeFromLeft(6);
    nextPresetButton.setBounds(controls.removeFromLeft(56));
    controls.removeFromLeft(10);
    testSequenceButton.setBounds(controls.removeFromLeft(110));

    auto globalArea = bounds.removeFromTop(144);
    globalArea.removeFromTop(24);

    const int globalKnobWidth = 82;
    const int globalGap = 8;
    std::array<juce::Slider*, 6> globalSliders { &tuneSlider, &decaySlider, &toneSlider, &driveSlider, &hatLengthSlider, &swingSlider };
    std::array<juce::Label*, 6> globalLabels { &tuneLabel, &decayLabel, &toneLabel, &driveLabel, &hatLengthLabel, &swingLabel };

    auto globalKnobs = globalArea.removeFromLeft((globalKnobWidth * 6) + (globalGap * 5));
    for (size_t i = 0; i < globalSliders.size(); ++i)
    {
        auto cell = globalKnobs.removeFromLeft(globalKnobWidth);
        globalLabels[i]->setBounds(cell.removeFromTop(18));
        globalSliders[i]->setBounds(cell.removeFromTop(86));
        globalKnobs.removeFromLeft(globalGap);
    }

    infoLabel.setBounds(globalArea.reduced(10, 10));

    bounds.removeFromTop(8);

    const int cardGap = 8;
    const int cardWidth = (bounds.getWidth() - (cardGap * 3)) / 4;
    const int cardHeight = (bounds.getHeight() - cardGap) / 2;

    for (int row = 0; row < 2; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            const size_t index = static_cast<size_t>(row * 4 + col);
            auto card = juce::Rectangle<int>(
                bounds.getX() + col * (cardWidth + cardGap),
                bounds.getY() + row * (cardHeight + cardGap),
                cardWidth,
                cardHeight).reduced(6);

            auto header = card.removeFromTop(20);
            drumTestButtons[index].setBounds(header.removeFromRight(42).reduced(0, 1));
            drumNameLabels[index].setBounds(header);

            const int knobGap = 4;
            const int knobWidth = (card.getWidth() - (knobGap * 4)) / 5;
            const int labelHeight = 13;
            const int sliderHeight = 100;

            std::array<juce::Label*, 5> labels {
                &drumLevelLabels[index],
                &drumTuneLabels[index],
                &drumDecayLabels[index],
                &drumToneLabels[index],
                &drumDriveLabels[index]
            };
            std::array<juce::Slider*, 5> sliders {
                &drumLevelSliders[index],
                &drumTuneSliders[index],
                &drumDecaySliders[index],
                &drumToneSliders[index],
                &drumDriveSliders[index]
            };

            for (size_t i = 0; i < 5; ++i)
            {
                auto column = card.removeFromLeft(knobWidth);
                labels[i]->setBounds(column.removeFromTop(labelHeight));
                column.removeFromTop(2);
                sliders[i]->setBounds(column.removeFromTop(sliderHeight));
                card.removeFromLeft(knobGap);
            }
        }
    }
}
