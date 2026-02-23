#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <memory>
#include <vector>

namespace
{
constexpr float twoPi = juce::MathConstants<float>::twoPi;
constexpr double testSequenceBpm = 168.0;
constexpr int testSequenceSteps = 32;

constexpr std::array<const char*, 8> drumIdPrefixes {
    "kick", "snare", "closedHat", "openHat", "crash", "ride", "clap", "rim"
};

constexpr std::array<const char*, 8> drumNames {
    "Kick", "Snare", "Closed Hat", "Open Hat", "Crash", "Ride", "Clap", "Rim"
};

struct SequenceHit
{
    int step;
    BurialDrumPluginAudioProcessor::DrumType type;
    float velocity;
};

constexpr std::array<SequenceHit, 30> testPattern {
    SequenceHit { 0,  BurialDrumPluginAudioProcessor::DrumType::kick,      0.96f },
    SequenceHit { 0,  BurialDrumPluginAudioProcessor::DrumType::closedHat, 0.52f },
    SequenceHit { 2,  BurialDrumPluginAudioProcessor::DrumType::closedHat, 0.44f },
    SequenceHit { 3,  BurialDrumPluginAudioProcessor::DrumType::rim,       0.48f },
    SequenceHit { 4,  BurialDrumPluginAudioProcessor::DrumType::snare,     0.90f },
    SequenceHit { 4,  BurialDrumPluginAudioProcessor::DrumType::closedHat, 0.45f },
    SequenceHit { 6,  BurialDrumPluginAudioProcessor::DrumType::closedHat, 0.42f },
    SequenceHit { 7,  BurialDrumPluginAudioProcessor::DrumType::clap,      0.66f },
    SequenceHit { 8,  BurialDrumPluginAudioProcessor::DrumType::kick,      0.88f },
    SequenceHit { 8,  BurialDrumPluginAudioProcessor::DrumType::openHat,   0.48f },
    SequenceHit { 10, BurialDrumPluginAudioProcessor::DrumType::closedHat, 0.39f },
    SequenceHit { 12, BurialDrumPluginAudioProcessor::DrumType::snare,     0.86f },
    SequenceHit { 12, BurialDrumPluginAudioProcessor::DrumType::closedHat, 0.46f },
    SequenceHit { 14, BurialDrumPluginAudioProcessor::DrumType::ride,      0.36f },
    SequenceHit { 15, BurialDrumPluginAudioProcessor::DrumType::rim,       0.42f },

    SequenceHit { 16, BurialDrumPluginAudioProcessor::DrumType::kick,      0.94f },
    SequenceHit { 16, BurialDrumPluginAudioProcessor::DrumType::closedHat, 0.50f },
    SequenceHit { 18, BurialDrumPluginAudioProcessor::DrumType::closedHat, 0.40f },
    SequenceHit { 19, BurialDrumPluginAudioProcessor::DrumType::clap,      0.59f },
    SequenceHit { 20, BurialDrumPluginAudioProcessor::DrumType::snare,     0.90f },
    SequenceHit { 20, BurialDrumPluginAudioProcessor::DrumType::openHat,   0.50f },
    SequenceHit { 22, BurialDrumPluginAudioProcessor::DrumType::closedHat, 0.41f },
    SequenceHit { 23, BurialDrumPluginAudioProcessor::DrumType::rim,       0.40f },
    SequenceHit { 24, BurialDrumPluginAudioProcessor::DrumType::kick,      0.89f },
    SequenceHit { 24, BurialDrumPluginAudioProcessor::DrumType::closedHat, 0.45f },
    SequenceHit { 26, BurialDrumPluginAudioProcessor::DrumType::closedHat, 0.37f },
    SequenceHit { 28, BurialDrumPluginAudioProcessor::DrumType::snare,     0.92f },
    SequenceHit { 28, BurialDrumPluginAudioProcessor::DrumType::crash,     0.46f },
    SequenceHit { 30, BurialDrumPluginAudioProcessor::DrumType::ride,      0.32f },
    SequenceHit { 31, BurialDrumPluginAudioProcessor::DrumType::clap,      0.50f }
};

float expDecay(float t, float tau)
{
    return std::exp(-t / juce::jmax(tau, 1.0e-5f));
}

float smoothAttack(float t, float attack)
{
    if (attack <= 0.0f)
        return 1.0f;

    return juce::jlimit(0.0f, 1.0f, t / attack);
}

} // namespace

BurialDrumPluginAudioProcessor::BurialDrumPluginAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    cacheParameterPointers();
}

juce::AudioProcessorValueTreeState::ParameterLayout BurialDrumPluginAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> layout;
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("tune", "Tune", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.01f), 0.0f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("decay", "Decay", juce::NormalisableRange<float>(0.2f, 1.8f, 0.001f), 0.78f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("tone", "Tone", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.25f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("drive", "Drive", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.28f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("hatLength", "Hat Length", juce::NormalisableRange<float>(0.2f, 2.0f, 0.001f), 0.82f));
    layout.push_back(std::make_unique<juce::AudioParameterFloat>("swing", "Swing", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));

    for (size_t i = 0; i < drumIdPrefixes.size(); ++i)
    {
        const juce::String prefix(drumIdPrefixes[i]);
        const juce::String name(drumNames[i]);

        layout.push_back(std::make_unique<juce::AudioParameterFloat>(
            prefix + "Level",
            name + " Level",
            juce::NormalisableRange<float>(0.0f, 1.5f, 0.001f),
            (i == 0 ? 1.12f : (i == 1 ? 1.08f : 1.0f))));

        layout.push_back(std::make_unique<juce::AudioParameterFloat>(
            prefix + "Tune",
            name + " Tune",
            juce::NormalisableRange<float>(-12.0f, 12.0f, 0.01f),
            0.0f));

        layout.push_back(std::make_unique<juce::AudioParameterFloat>(
            prefix + "Decay",
            name + " Decay",
            juce::NormalisableRange<float>(0.3f, 2.0f, 0.001f),
            1.0f));

        layout.push_back(std::make_unique<juce::AudioParameterFloat>(
            prefix + "Tone",
            name + " Tone",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
            0.5f));

        layout.push_back(std::make_unique<juce::AudioParameterFloat>(
            prefix + "Drive",
            name + " Drive",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
            (i < 2 ? 0.34f : 0.22f)));
    }

    return { layout.begin(), layout.end() };
}

int BurialDrumPluginAudioProcessor::drumTypeToIndex(DrumType type)
{
    switch (type)
    {
        case DrumType::kick: return 0;
        case DrumType::snare: return 1;
        case DrumType::closedHat: return 2;
        case DrumType::openHat: return 3;
        case DrumType::crash: return 4;
        case DrumType::ride: return 5;
        case DrumType::clap: return 6;
        case DrumType::rim: return 7;
        case DrumType::none: return -1;
    }

    return -1;
}

const char* BurialDrumPluginAudioProcessor::drumIdPrefix(DrumType type)
{
    const int index = drumTypeToIndex(type);
    if (index < 0)
        return "";

    return drumIdPrefixes[static_cast<size_t>(index)];
}

void BurialDrumPluginAudioProcessor::cacheParameterPointers()
{
    const std::array<DrumType, drumCount> types {
        DrumType::kick,
        DrumType::snare,
        DrumType::closedHat,
        DrumType::openHat,
        DrumType::crash,
        DrumType::ride,
        DrumType::clap,
        DrumType::rim
    };

    for (size_t i = 0; i < types.size(); ++i)
    {
        const juce::String prefix(drumIdPrefix(types[i]));
        drumLevelParams[i] = parameters.getRawParameterValue(prefix + "Level");
        drumTuneParams[i] = parameters.getRawParameterValue(prefix + "Tune");
        drumDecayParams[i] = parameters.getRawParameterValue(prefix + "Decay");
        drumToneParams[i] = parameters.getRawParameterValue(prefix + "Tone");
        drumDriveParams[i] = parameters.getRawParameterValue(prefix + "Drive");
    }
}

void BurialDrumPluginAudioProcessor::prepareToPlay(double sampleRate, int)
{
    currentSampleRate = juce::jmax(8000.0, sampleRate);
    lpStateL = 0.0f;
    lpStateR = 0.0f;
    punchHPState = 0.0f;

    for (auto& v : voices)
        v = {};

    testSequencePlaying = false;
    testSequenceSampleCursor = 0;
}

void BurialDrumPluginAudioProcessor::releaseResources()
{
}

bool BurialDrumPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

BurialDrumPluginAudioProcessor::DrumType BurialDrumPluginAudioProcessor::noteToDrumType(int midiNote) const
{
    switch (midiNote)
    {
        case 36: return DrumType::kick;
        case 38: return DrumType::snare;
        case 42: return DrumType::closedHat;
        case 46: return DrumType::openHat;
        case 49: return DrumType::crash;
        case 51: return DrumType::ride;
        case 39: return DrumType::clap;
        case 37: return DrumType::rim;
        default: return DrumType::none;
    }
}

void BurialDrumPluginAudioProcessor::triggerDrum(DrumType type, float velocity, int sampleOffset)
{
    auto it = std::find_if(voices.begin(), voices.end(), [](const Voice& v) { return !v.active; });

    if (it == voices.end())
        it = std::min_element(voices.begin(), voices.end(), [](const Voice& a, const Voice& b) { return a.sampleIndex > b.sampleIndex; });

    it->active = true;
    it->type = type;
    it->velocity = juce::jlimit(0.0f, 1.0f, velocity);
    it->samplesUntilStart = juce::jmax(0, sampleOffset);
    it->sampleIndex = 0;
    it->phaseA = random01(rng) * twoPi;
    it->phaseB = random01(rng) * twoPi;
    it->phaseC = random01(rng) * twoPi;
    it->noiseState = static_cast<uint32_t>(rng()) | 1u;
    it->toneState = 0.0f;
}

float BurialDrumPluginAudioProcessor::nextNoiseSample(uint32_t& state)
{
    // Xorshift32: fast decorrelated noise without periodic tonal artifacts.
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    const float normalized = static_cast<float>(state & 0x00ffffffu) / static_cast<float>(0x01000000u);
    return normalized * 2.0f - 1.0f;
}

float BurialDrumPluginAudioProcessor::softClip(float x)
{
    return std::tanh(x);
}

float BurialDrumPluginAudioProcessor::renderVoiceSample(Voice& v) const
{
    const int drumIndex = drumTypeToIndex(v.type);
    if (drumIndex < 0)
    {
        v.active = false;
        return 0.0f;
    }

    const float drumLevel = blockDrumLevels[static_cast<size_t>(drumIndex)];
    const float drumTune = blockDrumTuneSemitones[static_cast<size_t>(drumIndex)];
    const float drumDecay = blockDrumDecay[static_cast<size_t>(drumIndex)];
    const float drumTone = blockDrumTone[static_cast<size_t>(drumIndex)];
    const float drumDrive = blockDrumDrive[static_cast<size_t>(drumIndex)];

    const float sr = static_cast<float>(currentSampleRate);
    const float t = static_cast<float>(v.sampleIndex) / sr;
    const float vel = 0.35f + 0.65f * v.velocity;
    const float tuneMul = std::pow(2.0f, (blockTuneSemitones + drumTune) / 12.0f);
    const float decayMul = blockDecay * drumDecay;
    const float hatMul = blockHatLength;

    float out = 0.0f;

    switch (v.type)
    {
        case DrumType::kick:
        {
            const float freq = (46.0f + 300.0f * expDecay(t, 0.010f * decayMul)) * tuneMul;
            const float amp = expDecay(t, 0.14f * decayMul);
            const float click = expDecay(t, 0.0019f) * nextNoiseSample(v.noiseState);
            v.phaseA += twoPi * freq / sr;
            v.phaseB += twoPi * (freq * 0.5f) / sr;
            const float thump = std::sin(v.phaseA) * amp;
            const float sub = std::sin(v.phaseB) * expDecay(t, 0.16f * decayMul);
            out = (thump * 1.02f) + (sub * 0.60f) + 0.66f * click;
            if (t > 0.52f * decayMul)
                v.active = false;
            break;
        }
        case DrumType::snare:
        {
            const float bodyAmp = expDecay(t, 0.082f * decayMul);
            const float noiseAmp = expDecay(t, 0.058f * decayMul);
            const float bodyFreq = (238.0f + 130.0f * expDecay(t, 0.009f * decayMul)) * tuneMul;
            v.phaseA += twoPi * bodyFreq / sr;
            const float body = std::sin(v.phaseA) * bodyAmp;
            const float noise = nextNoiseSample(v.noiseState) * noiseAmp;
            const float crack = expDecay(t, 0.0024f) * nextNoiseSample(v.noiseState);
            out = 0.90f * body + 0.66f * noise + 0.94f * crack;
            if (t > 0.34f * decayMul)
                v.active = false;
            break;
        }
        case DrumType::closedHat:
        {
            const float env = expDecay(t, 0.018f * decayMul * hatMul);
            const float n = nextNoiseSample(v.noiseState);
            const float metal = std::sin(v.phaseA) + std::sin(v.phaseB * 1.733f);
            v.phaseA += twoPi * (7340.0f * tuneMul) / sr;
            v.phaseB += twoPi * (9170.0f * tuneMul) / sr;
            out = (0.62f * n + 0.38f * metal) * env;
            if (t > 0.10f * decayMul * hatMul)
                v.active = false;
            break;
        }
        case DrumType::openHat:
        {
            const float env = expDecay(t, 0.045f * decayMul * hatMul);
            const float n = nextNoiseSample(v.noiseState);
            const float metal = std::sin(v.phaseA) + 0.7f * std::sin(v.phaseB * 1.91f) + 0.4f * std::sin(v.phaseC * 2.27f);
            v.phaseA += twoPi * (6100.0f * tuneMul) / sr;
            v.phaseB += twoPi * (7420.0f * tuneMul) / sr;
            v.phaseC += twoPi * (9030.0f * tuneMul) / sr;
            out = (0.42f * n + 0.58f * metal) * env;
            if (t > 0.24f * decayMul * hatMul)
                v.active = false;
            break;
        }
        case DrumType::crash:
        {
            const float env = expDecay(t, 0.18f * decayMul * hatMul) * smoothAttack(t, 0.002f);
            const float n = nextNoiseSample(v.noiseState) * expDecay(t, 0.095f * decayMul * hatMul);
            v.phaseA += twoPi * (4540.0f * tuneMul) / sr;
            v.phaseB += twoPi * (5920.0f * tuneMul) / sr;
            v.phaseC += twoPi * (7440.0f * tuneMul) / sr;
            const float partials = 0.64f * std::sin(v.phaseA) + 0.38f * std::sin(v.phaseB) + 0.18f * std::sin(v.phaseC);
            out = (0.22f * n + 0.78f * partials) * env;
            if (t > 0.30f * decayMul * hatMul)
                v.active = false;
            break;
        }
        case DrumType::ride:
        {
            const float env = expDecay(t, 0.19f * decayMul * hatMul) * smoothAttack(t, 0.0018f);
            const float n = nextNoiseSample(v.noiseState) * expDecay(t, 0.085f * decayMul * hatMul);
            v.phaseA += twoPi * (3890.0f * tuneMul) / sr;
            v.phaseB += twoPi * (5280.0f * tuneMul) / sr;
            const float ping = std::sin(v.phaseA) * expDecay(t, 0.10f * decayMul);
            const float tail = 0.20f * std::sin(v.phaseB) * expDecay(t, 0.15f * decayMul);
            out = (0.20f * n + ping + tail) * env;
            if (t > 0.34f * decayMul * hatMul)
                v.active = false;
            break;
        }
        case DrumType::clap:
        {
            const float burst1 = expDecay(juce::jmax(0.0f, t - 0.000f), 0.015f * decayMul);
            const float burst2 = expDecay(juce::jmax(0.0f, t - 0.012f * decayMul), 0.013f * decayMul);
            const float burst3 = expDecay(juce::jmax(0.0f, t - 0.022f * decayMul), 0.028f * decayMul);
            const float env = juce::jmin(1.0f, burst1 + burst2 + burst3);
            const float n = nextNoiseSample(v.noiseState);
            out = n * env;
            if (t > 0.34f * decayMul)
                v.active = false;
            break;
        }
        case DrumType::rim:
        {
            const float env = expDecay(t, 0.050f * decayMul);
            v.phaseA += twoPi * (940.0f * tuneMul) / sr;
            v.phaseB += twoPi * (1490.0f * tuneMul) / sr;
            const float tone = std::sin(v.phaseA) + 0.6f * std::sin(v.phaseB);
            const float tick = expDecay(t, 0.0032f) * nextNoiseSample(v.noiseState);
            out = (0.78f * tone + 0.50f * tick) * env;
            if (t > 0.18f * decayMul)
                v.active = false;
            break;
        }
        case DrumType::none:
            v.active = false;
            break;
    }

    const float toneCoeff = juce::jmap(drumTone, 0.02f, 0.62f);
    v.toneState += toneCoeff * (out - v.toneState);
    const float toneBlend = juce::jlimit(0.0f, 1.0f, drumTone);
    const float toned = juce::jmap(toneBlend, v.toneState, out);

    const float drumDriveGain = 1.0f + 6.6f * drumDrive;
    const float drumDriven = softClip(toned * drumDriveGain) / std::sqrt(drumDriveGain);

    return softClip(drumDriven * vel * drumLevel);
}

void BurialDrumPluginAudioProcessor::startTestSequence()
{
    testSequenceRequested.store(true);
}

void BurialDrumPluginAudioProcessor::queueDrumTestHit(DrumType type)
{
    const int drumIndex = drumTypeToIndex(type);
    if (drumIndex < 0)
        return;

    const auto bit = static_cast<uint32_t>(1u << static_cast<uint32_t>(drumIndex));
    debugDrumTriggerMask.fetch_or(bit);
}

void BurialDrumPluginAudioProcessor::triggerTestSequenceEvents(int blockSize)
{
    if (testSequenceRequested.exchange(false))
    {
        testSequencePlaying = true;
        testSequenceSampleCursor = 0;
    }

    if (!testSequencePlaying)
        return;

    const int64_t samplesPerStep = static_cast<int64_t>(std::round((currentSampleRate * 60.0 / testSequenceBpm) * 0.25));
    const int64_t totalSamples = samplesPerStep * testSequenceSteps;
    const int64_t blockStart = testSequenceSampleCursor;
    const int64_t blockEnd = blockStart + blockSize;

    for (const auto& hit : testPattern)
    {
        const int64_t hitSample = static_cast<int64_t>(hit.step) * samplesPerStep;
        if (hitSample >= blockStart && hitSample < blockEnd)
            triggerDrum(hit.type, hit.velocity, static_cast<int>(hitSample - blockStart));
    }

    testSequenceSampleCursor += blockSize;
    if (testSequenceSampleCursor >= totalSamples)
        testSequencePlaying = false;
}

int BurialDrumPluginAudioProcessor::applySwingOffset(int sampleOffset, int blockSize) const
{
    const auto* swingParam = parameters.getRawParameterValue("swing");
    if (swingParam == nullptr)
        return sampleOffset;

    const float swing = *swingParam;
    if (swing <= 0.001f)
        return sampleOffset;

    auto* playHead = getPlayHead();
    if (playHead == nullptr)
        return sampleOffset;

    const auto position = playHead->getPosition();
    if (!position.hasValue())
        return sampleOffset;

    const auto bpmOptional = position->getBpm();
    const auto ppqOptional = position->getPpqPosition();
    if (!bpmOptional.hasValue() || !ppqOptional.hasValue())
        return sampleOffset;

    const double bpm = *bpmOptional;
    if (bpm <= 1.0)
        return sampleOffset;

    const double quartersPerSample = bpm / (60.0 * currentSampleRate);
    const double ppqAtEvent = *ppqOptional + static_cast<double>(sampleOffset) * quartersPerSample;
    const int eighthIndex = static_cast<int>(std::floor(ppqAtEvent * 2.0));
    const bool isOffbeatEighth = (eighthIndex & 1) != 0;

    if (!isOffbeatEighth)
        return sampleOffset;

    const int samplesPerEighth = static_cast<int>(std::round((60.0 / bpm) * currentSampleRate * 0.5));
    const int maxSwingSamples = static_cast<int>(std::round(samplesPerEighth * 0.33));
    const int delay = static_cast<int>(std::round(static_cast<float>(maxSwingSamples) * swing));
    return juce::jlimit(0, blockSize + maxSwingSamples, sampleOffset + delay);
}

void BurialDrumPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    const auto numSamples = buffer.getNumSamples();
    const auto numChannels = buffer.getNumChannels();

    blockTuneSemitones = *parameters.getRawParameterValue("tune");
    blockDecay = *parameters.getRawParameterValue("decay");
    blockTone = *parameters.getRawParameterValue("tone");
    blockDrive = *parameters.getRawParameterValue("drive");
    blockHatLength = *parameters.getRawParameterValue("hatLength");

    for (size_t i = 0; i < drumCount; ++i)
    {
        if (drumLevelParams[i] != nullptr)
            blockDrumLevels[i] = *drumLevelParams[i];
        if (drumTuneParams[i] != nullptr)
            blockDrumTuneSemitones[i] = *drumTuneParams[i];
        if (drumDecayParams[i] != nullptr)
            blockDrumDecay[i] = *drumDecayParams[i];
        if (drumToneParams[i] != nullptr)
            blockDrumTone[i] = *drumToneParams[i];
        if (drumDriveParams[i] != nullptr)
            blockDrumDrive[i] = *drumDriveParams[i];
    }

    triggerTestSequenceEvents(numSamples);

    const uint32_t debugMask = debugDrumTriggerMask.exchange(0u);
    if (debugMask != 0u)
    {
        for (size_t i = 0; i < drumCount; ++i)
        {
            const uint32_t bit = static_cast<uint32_t>(1u << static_cast<uint32_t>(i));
            if ((debugMask & bit) == 0u)
                continue;

            triggerDrum(static_cast<DrumType>(static_cast<int>(i)), 0.95f, 0);
        }
    }

    for (const auto metadata : midiMessages)
    {
        const auto message = metadata.getMessage();
        if (message.isNoteOn())
        {
            const auto type = noteToDrumType(message.getNoteNumber());
            if (type != DrumType::none)
                triggerDrum(type, message.getFloatVelocity(), applySwingOffset(metadata.samplePosition, numSamples));
        }
    }

    midiMessages.clear();
    buffer.clear();

    const float lpCoeff = juce::jmap(blockTone, 0.14f, 0.52f);
    const float driveGain = 1.0f + 6.4f * blockDrive;
    const float driveTrim = 1.0f / std::sqrt(driveGain);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float mono = 0.0f;
        bool hasStartedVoice = false;

        for (auto& v : voices)
        {
            if (!v.active)
                continue;

            if (v.samplesUntilStart > 0)
            {
                --v.samplesUntilStart;
                continue;
            }

            hasStartedVoice = true;
            mono += renderVoiceSample(v);
            ++v.sampleIndex;
        }

        if (!hasStartedVoice && std::abs(mono) < 1.0e-7f)
        {
            punchHPState = 0.0f;
            lpStateL = 0.0f;
            lpStateR = 0.0f;
        }

        punchHPState += 0.11f * (mono - punchHPState);
        const float transient = mono - punchHPState;
        mono += transient * 0.95f;
        mono = softClip(mono * 0.62f * driveGain) * driveTrim;

        // Dark one-pole filtering and a tiny channel offset for texture.
        lpStateL += lpCoeff * (mono - lpStateL);
        lpStateR += lpCoeff * ((mono * 0.997f) - lpStateR);

        if (numChannels > 0)
            buffer.setSample(0, sample, lpStateL);
        if (numChannels > 1)
            buffer.setSample(1, sample, lpStateR);
    }

}

void BurialDrumPluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (const auto state = parameters.copyState(); state.isValid())
    {
        if (const auto xml = state.createXml())
            copyXmlToBinary(*xml, destData);
    }
}

void BurialDrumPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    const std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState == nullptr)
        return;

    if (!xmlState->hasTagName(parameters.state.getType()))
        return;

    parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
    cacheParameterPointers();
}

juce::AudioProcessorEditor* BurialDrumPluginAudioProcessor::createEditor()
{
    return new BurialDrumPluginAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BurialDrumPluginAudioProcessor();
}
