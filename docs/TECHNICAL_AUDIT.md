# BassMaid — Technical Audit

Scope: BassMaid release baseline. This audit mirrors the FreeEQ8 / MeterMaid
structure so readers can compare plugins apples-to-astructure sSource layout
- `BassMaid*/Source/PluginProcessor.{h,cpp}` — audio processor + parameter layout
- `BassMaid*/Source/PluginEditor.{h,cpp}` — custom-painted UI
- `BassMaid*/CMakeLists.txt` — JUCE plugin build

## Build
`.github/workflows/build.yml` runs on `macos-14`, clones JUCE 8.0.0,
configures with `-DCMAKE_OSX_ARCHITECTURES=\"arm64;x86_64\"` and builds
VST3 + AU + Standalone universal binaries, verified with `lipo -info`,
zipped, and attached to the GitHub Release on any `v*` tag push.

## Standards
BassMaid is built with JUCE 8.0.0 (C++20) and targets VST3 / AU / Standalone
on macOS 11.0+. The DSP is self-contained and uses `juce::dsp` where
applicable.

## Remaining release blockers
See `RELEASE_GATES.md`.
