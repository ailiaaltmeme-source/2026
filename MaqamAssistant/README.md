# Maqam Assistant (JUCE / VST3) — starter skeleton

Build (needs CMake 3.22+, Git, and VS 2022 on Windows or Xcode on macOS):

    cmake -B build
    cmake --build build --config Release

The VST3 appears under build/MaqamAssistant_artefacts/Release/VST3/.
Copy it to C:\Program Files\Common Files\VST3 (Windows) or ~/Library/Audio/Plug-Ins/VST3 (macOS),
then rescan plugins in Studio One. The Standalone app is built next to it.

Use: drop a WAV on the window (or put it on an audio track, tick Listen, play, press Analyse).
Drag a chord tile onto a MIDI track.

Status: UNTESTED first draft. Maqam templates are starting values only; quarter-tones are
rounded to semitones in the exported MIDI (TODO: pitch bend / MPE).

## Build without installing anything (GitHub Actions)
Upload this folder to a new GitHub repository (include the hidden .github folder).
Open the Actions tab, run "Build plugin", then download the artifact for your system.
It contains the .vst3 and the Standalone app.
