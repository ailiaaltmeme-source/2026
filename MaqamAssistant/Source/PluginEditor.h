#pragma once
#include "PluginProcessor.h"
#include "Chords.h"
#include <juce_audio_utils/juce_audio_utils.h>

// A tile you can drag onto a MIDI track in the DAW (drags a temporary .mid file)
class ChordTile : public juce::Component
{
public:
    explicit ChordTile (Chord c) : chord (std::move (c)) {}

    void paint (juce::Graphics& g) override
    {
        g.setColour (juce::Colour (0xff2a3a4a));
        g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (3.f), 8.f);
        g.setColour (juce::Colours::white);
        g.drawText (chord.label, getLocalBounds(), juce::Justification::centred);
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (dragging || e.getDistanceFromDragStart() < 8) return;
        dragging = true;
        auto f = writeMidi (chord.bars, "Maqam_" + chord.label);
        juce::Component::SafePointer<ChordTile> safe (this);
        juce::DragAndDropContainer::performExternalDragDropOfFiles (
            juce::StringArray { f.getFullPathName() }, false, this,
            [safe] { if (safe != nullptr) safe->dragging = false; });
    }
    void mouseUp (const juce::MouseEvent&) override { dragging = false; }

private:
    Chord chord;
    bool dragging = false;
};

class MaqamEditor : public juce::AudioProcessorEditor,
                    public juce::FileDragAndDropTarget
{
public:
    explicit MaqamEditor (MaqamProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;
    bool isInterestedInFileDrag (const juce::StringArray&) override;
    void filesDropped (const juce::StringArray&, int, int) override;

private:
    void analyse (const std::vector<float>& mono, double sampleRate);

    MaqamProcessor& proc;
    juce::ToggleButton listenButton { "Listen (insert on audio track, then press play)" };
    juce::TextButton analyseButton { "Analyse" };
    juce::Label result;
    juce::OwnedArray<ChordTile> tiles;
};
