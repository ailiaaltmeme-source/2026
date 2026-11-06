#pragma once
#include <juce_core/juce_core.h>
#include <array>
#include <vector>

namespace Maqam
{
    struct MaqamTemplate { const char* name; std::array<int, 7> cents; };

    // Starting templates (cents above the tonic). REVIEW BY EAR / with a reference:
    // intervals differ between Iraqi, Egyptian and Turkish schools.
    const std::vector<MaqamTemplate>& templates();

    struct Result
    {
        juce::String name;
        int templateIndex = 0;
        int tonicBin = 0;     // tonic = tonicBin * 50 cents above C (24 bins per octave)
        float score = 0.f;
    };

    // Returns the 3 best (maqam, tonic) matches, best first. Empty if no pitch found.
    std::vector<Result> detect (const std::vector<float>& mono, double sampleRate);

    juce::String binName (int bin);   // "D", "Eb+" (+ = quarter-tone up)
}
