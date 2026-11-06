#include "PluginEditor.h"
#include "MaqamDetector.h"

MaqamEditor::MaqamEditor (MaqamProcessor& p) : AudioProcessorEditor (&p), proc (p)
{
    setSize (560, 320);

    listenButton.setColour (juce::ToggleButton::textColourId, juce::Colours::white);
    listenButton.onClick = [this]
    {
        const bool on = listenButton.getToggleState();
        if (on) proc.clearCapture();
        proc.listening = on;
    };
    addAndMakeVisible (listenButton);

    analyseButton.onClick = [this] { analyse (proc.getCapture(), proc.getSampleRateHz()); };
    addAndMakeVisible (analyseButton);

    result.setColour (juce::Label::textColourId, juce::Colours::white);
    result.setJustificationType (juce::Justification::topLeft);
    result.setText ("Drop an audio file here (WAV/AIFF/FLAC), or use Listen + Analyse.",
                    juce::dontSendNotification);
    addAndMakeVisible (result);
}

void MaqamEditor::paint (juce::Graphics& g) { g.fillAll (juce::Colour (0xff14181d)); }

void MaqamEditor::resized()
{
    auto r = getLocalBounds().reduced (12);
    listenButton.setBounds (r.removeFromTop (28));
    auto row = r.removeFromTop (36);
    analyseButton.setBounds (row.removeFromLeft (120).reduced (0, 2));
    result.setBounds (r.removeFromTop (110));

    if (tiles.size() > 0)
    {
        const int w = r.getWidth() / tiles.size();
        for (auto* t : tiles) t->setBounds (r.removeFromLeft (w));
    }
}

bool MaqamEditor::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (auto& f : files)
        if (juce::File (f).hasFileExtension ("wav;aiff;aif;flac;ogg")) return true;
    return false;
}

void MaqamEditor::filesDropped (const juce::StringArray& files, int, int)
{
    juce::AudioFormatManager fm;
    fm.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader (fm.createReaderFor (juce::File (files[0])));
    if (reader == nullptr) return;

    const int n = (int) juce::jmin ((juce::int64) reader->lengthInSamples,
                                    (juce::int64) (reader->sampleRate * 60.0));
    juce::AudioBuffer<float> buf ((int) reader->numChannels, n);
    reader->read (&buf, 0, n, 0, true, true);

    std::vector<float> mono ((size_t) n, 0.0f);
    for (int ch = 0; ch < buf.getNumChannels(); ++ch)
        for (int i = 0; i < n; ++i)
            mono[(size_t) i] += buf.getSample (ch, i) / (float) buf.getNumChannels();

    analyse (mono, reader->sampleRate);
}

void MaqamEditor::analyse (const std::vector<float>& mono, double sr)
{
    auto results = Maqam::detect (mono, sr);
    tiles.clear();

    if (results.empty())
    {
        result.setText ("No clear pitch found. Try a longer, cleaner, single-line melody.",
                        juce::dontSendNotification);
        resized();
        return;
    }

    juce::String text;
    int i = 1;
    for (auto& r : results)
        text << i++ << ") " << r.name << " on " << Maqam::binName (r.tonicBin)
             << "   (" << juce::String (r.score, 2) << ")\n";
    result.setText (text, juce::dontSendNotification);

    for (auto& c : suggestChords (results.front()))
        addAndMakeVisible (tiles.add (new ChordTile (c)));

    resized();
}
