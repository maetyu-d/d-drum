#pragma once

#include <array>
#include <cstdint>
#include <random>

#include <juce_audio_processors/juce_audio_processors.h>

class BurialDrumPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    enum class DrumType
    {
        kick,
        snare,
        closedHat,
        openHat,
        crash,
        ride,
        clap,
        rim,
        none
    };

    BurialDrumPluginAudioProcessor();
    ~BurialDrumPluginAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.5; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return parameters; }
    void startTestSequence();
    void queueDrumTestHit(DrumType type);

private:
    static constexpr int drumCount = 8;

    struct Voice
    {
        bool active = false;
        DrumType type = DrumType::none;
        float velocity = 0.0f;
        int samplesUntilStart = 0;
        int sampleIndex = 0;
        float phaseA = 0.0f;
        float phaseB = 0.0f;
        float phaseC = 0.0f;
        uint32_t noiseState = 1u;
        float toneState = 0.0f;
    };

    static constexpr int maxVoices = 32;
    std::array<Voice, maxVoices> voices;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    static int drumTypeToIndex(DrumType type);
    static const char* drumIdPrefix(DrumType type);

    void cacheParameterPointers();

    DrumType noteToDrumType(int midiNote) const;
    void triggerDrum(DrumType type, float velocity, int sampleOffset);
    float renderVoiceSample(Voice& v) const;
    int applySwingOffset(int sampleOffset, int blockSize) const;
    void triggerTestSequenceEvents(int blockSize);

    static float nextNoiseSample(uint32_t& state);
    static float softClip(float x);

    double currentSampleRate = 44100.0;

    // Global mellowing to keep the kit dark and lo-fi.
    float lpStateL = 0.0f;
    float lpStateR = 0.0f;
    float punchHPState = 0.0f;

    std::mt19937 rng;
    std::uniform_real_distribution<float> random01 { 0.0f, 1.0f };
    juce::AudioProcessorValueTreeState parameters;

    // Cached raw parameter pointers for per-drum controls.
    std::array<std::atomic<float>*, drumCount> drumLevelParams {};
    std::array<std::atomic<float>*, drumCount> drumTuneParams {};
    std::array<std::atomic<float>*, drumCount> drumDecayParams {};
    std::array<std::atomic<float>*, drumCount> drumToneParams {};
    std::array<std::atomic<float>*, drumCount> drumDriveParams {};

    // Updated at block start from parameters.
    float blockTuneSemitones = 0.0f;
    float blockDecay = 0.9f;
    float blockTone = 0.25f;
    float blockDrive = 0.2f;
    float blockHatLength = 1.0f;
    std::array<float, drumCount> blockDrumLevels { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
    std::array<float, drumCount> blockDrumTuneSemitones { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    std::array<float, drumCount> blockDrumDecay { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
    std::array<float, drumCount> blockDrumTone { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };
    std::array<float, drumCount> blockDrumDrive { 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f };

    std::atomic<bool> testSequenceRequested { false };
    std::atomic<uint32_t> debugDrumTriggerMask { 0u };
    bool testSequencePlaying = false;
    int64_t testSequenceSampleCursor = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BurialDrumPluginAudioProcessor)
};
