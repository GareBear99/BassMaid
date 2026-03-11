BassMaid (JUCE) — Housekeeping for Your Low End — Sub / Harmonics / Tighten

Build (CMake):
1) Install JUCE locally (JUCE 7 recommended).
2) Configure:
   Option A:
     cmake -B build -S . -DJUCE_DIR=/path/to/JUCE (folder containing JUCEConfig.cmake)
   Option B:
     clone JUCE next to this project and run:
     cmake -B build -S . -DJUCE_PATH=../JUCE
3) Build:
   cmake --build build --config Release

Plugin formats: AU + VST3 + Standalone
Notes:
- RT-safe: no allocations in processBlock, no coefficient rebuilds mid-block.
- Crossover: Linkwitz-Riley 4th order (juce::dsp::LinkwitzRileyFilter)
- Sub: envelope-followed sine reinforcement around Sub Tune
- Harmonics: tone tilt -> waveshaper -> LPF
- Tighten: one-knob low-band compressor
- Output protection: optional limiter + ceiling
