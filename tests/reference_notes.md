# Reference Notes

Suggested validation targets for BassMaid:

- static-signal sanity (silence / DC / 1 kHz sine) — verify no NaNs,
  consistent output levels, clean state-reset on `prepareToPlay`.
- parameter sweep — every parameter from min to max, no clicks.
- preset round-trip — save state, reload, assert equality.
- large-buffer test — run at 4096-sample block sizes for 60 s, no drift.

A full Catch2 / GoogleTest / JUCE UnitTest harness will land in a
later release pass alongside real automated asserts (tracked in
`docs/RELEASE_GATES.md` Gate 3).
