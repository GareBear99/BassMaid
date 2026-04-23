# Changelog

All notable changes to BassMaid are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and the
project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.1.0-beta.1] \u2014 2026-04-23

### Added
- Universal (arm64 + x86_64) macOS VST3 + AU + Standalone CI build
  pipeline on `macos-14` via `.github/workflows/build.yml`.
- MIT LICENSE and CHANGELOG matching the FreeEQ8 release baseline.
- Canonical "Position in the Maid Audio Lineup" section listing the full
  eight-plugin Maid suite (FreeEQ8 anchor + BassMaid, SpaceMaid,
  GlueMaid, MixMaid, MultiMaid, MeterMaid, ChainMaid).

### Notes
- First tagged beta intended to trigger the CI pipeline. Binaries will
  attach to the release automatically when `macos-14` finishes the
  build.
