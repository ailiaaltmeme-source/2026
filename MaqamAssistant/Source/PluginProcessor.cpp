#include "PluginProcessor.h"
#include "PluginEditor.h"

void MaqamProcessor::prepareToPlay (double sr, int)
{
    sampleRate = sr;
    numCaptured = 0;
    captured.assign ((size_t) (sr * 30.0), 0.0f);
}

void MaqamProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    if (! listening.load() || buffer.getNumChannels() == 0) return;

    const float* in = buffer.getReadPointer (0);
    int n = numCaptured.load();
    const int cap = (int) captured.size();
    for (int i = 0; i < buffer.getNumSamples() && n < cap; ++i)
        captured[(size_t) n++] = in[i];
    numCaptured.store (n);
}

juce::AudioProcessorEditor* MaqamProcessor::createEditor() { return new MaqamEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MaqamProcessor(); }
