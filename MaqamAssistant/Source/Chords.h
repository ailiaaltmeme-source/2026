#pragma once
#include "MaqamDetector.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

struct Chord
{
    juce::String label;
    std::vector<std::vector<int>> bars;   // each bar = list of MIDI notes
};

// MVP: stacked thirds on the maqam's 7 degrees. Quarter-tones are ROUNDED to the
// nearest semitone for now (TODO: pitch-bend / MPE export).
inline std::vector<Chord> suggestChords (const Maqam::Result& r)
{
    const auto& tpl = Maqam::templates()[(size_t) r.templateIndex].cents;

    auto noteAt = [&] (int degree)
    {
        return r.tonicBin * 50 + tpl[(size_t) (degree % 7)] + 1200 * (degree / 7);
    };
    auto triad = [&] (int deg)
    {
        std::vector<int> n;
        for (int k : { 0, 2, 4 })
            n.push_back (60 + (int) std::lround (noteAt (deg + k) / 100.0));
        return n;
    };

    using Bars = std::vector<std::vector<int>>;
    return {
        { "I",  Bars { triad (0) } },
        { "IV", Bars { triad (3) } },
        { "V",  Bars { triad (4) } },
        { "I-IV-V-I", Bars { triad (0), triad (3), triad (4), triad (0) } },
    };
}

inline juce::File writeMidi (const std::vector<std::vector<int>>& bars, const juce::String& name)
{
    const int ticks = 480;
    juce::MidiMessageSequence seq;
    for (size_t b = 0; b < bars.size(); ++b)
        for (int note : bars[b])
        {
            seq.addEvent (juce::MidiMessage::noteOn (1, note, (juce::uint8) 90), (double) (b * 4 * (size_t) ticks));
            seq.addEvent (juce::MidiMessage::noteOff (1, note), (double) ((b + 1) * 4 * (size_t) ticks));
        }
    seq.updateMatchingNotes();

    juce::MidiFile mf;
    mf.setTicksPerQuarterNote (ticks);
    mf.addTrack (seq);

    auto f = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile (name + ".mid");
    f.deleteFile();
    if (auto out = f.createOutputStream())
        mf.writeTo (*out);
    return f;
}
