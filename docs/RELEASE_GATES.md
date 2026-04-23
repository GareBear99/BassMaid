# Release Gates

A release is only green when every gate below has evidence attached.

## Gate 1 — Build trust
- [x] Universal macOS build (arm64 + x86_64) via `.github/workflows/build.yml`
- [ ] Windows build
- [ ] Linux build

## Gate 2 — Host trust (DAW load + state save/restore)
- [ ] Reaper
- [ ] Ableton Live
- [ ] Logic Pro (AU)
- [ ] FL Studio
- [ ] Cubase

## Gate 3 — RT / performance trust
- [ ] 32-sample buffer stable
- [ ] 192 kHz stable
- [ ] 30-minute session without dropouts

## Gate 4 — UX trust
- [ ] Resize behaviour final
- [ ] Tooltips + readouts polished
- [ ] Preset management

## Gate 5 — Distribution trust
- [x] MIT LICENSE shipped
- [ ] macOS code signing / notarization
- [ ] Windows signing

## Release promotion rule
A `v*` tag without `-beta` / `-alpha` / `-rc` must only be cut when
Gates 1–5 are all green. Until then, builds ship as pre-release.
