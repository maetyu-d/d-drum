# D-Drum Machine

A drum machine synthesizer plugin, created using JUCE (AU/VST3/standalone), that generates lo-fi, Burial-inspired drum sounds from fixed MIDI notes. Intended to be used as part of my split suite, but can be used standalone too.

## MIDI note map

- `36` = kick
- `38` = snare
- `42` = closed hat
- `46` = open hat
- `49` = crash
- `51` = ride
- `39` = clap
- `37` = rim/side

## Build

You need JUCE available in one of these ways:

1. Set `JUCE_DIR` to a JUCE checkout or install path containing `JUCEConfig.cmake`
2. Put JUCE source at `./JUCE`
3. Put JUCE source at `../JUCE`

Then:

```bash
cmake -S . -B build -DJUCE_DIR=/absolute/path/to/JUCE
cmake --build build --config Release
```

Built plugin targets (from `CMakeLists.txt`): AU, VST3, Standalone.

## Sound design notes

The engine is synthesized (no samples):

- Kick: pitch-dropping sine + transient click
- Snare: tonal body + decaying noise
- Hats/cymbals: metallic partials + filtered noise
- Clap: multi-burst noise envelope
- Rim: short resonant tone + tick
- Global tone shaping: soft clipping and dark one-pole low-pass

## Controls

- Global:
  - `Tune`: global pitch offset (-12 to +12 semitones)
  - `Decay`: scales envelope length for all drums
  - `Tone`: dark/bright global low-pass voicing
  - `Drive`: saturation amount
  - `Hat Len`: extra decay scaling for hats/cymbals
  - `Swing`: delays off-beat 8th notes using host tempo/PPQ
- Per drum (Kick, Snare, Closed Hat, Open Hat, Crash, Ride, Clap, Rim):
  - `Level`: per-drum output trim
  - `Tune`: per-drum pitch offset (-12 to +12 semitones)
  - `Decay`: per-drum envelope scale
  - `Tone`: per-drum dark/bright filtering
  - `Drive`: per-drum saturation amount

## Test sequence

- Click `Play Test Sequence` in the plugin UI to trigger a built-in 2-bar preview groove.

## Preset browser

- In the top bar, use `Prev`, preset dropdown, and `Next` for an in-UI browser.
- Includes 12 varied kits:
  - Burial Base
  - Night Bus
  - Ghost Step
  - Crushed Rain
  - Dry 2-Step
  - Fog Garage
  - Punch Tunnel
  - Snap Skitter
  - Sub Pressure
  - Bright Spray
  - Washed Metal
  - Tape Dust

Build artifacts are written under:

- `build/BurialDrumPlugin_artefacts/Standalone/`
- `build/BurialDrumPlugin_artefacts/AU/`
- `build/BurialDrumPlugin_artefacts/VST3/`
