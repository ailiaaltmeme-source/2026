#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include <vector>

class MaqamProcessor : public juce::AudioProcessor
{
public:
    MaqamProcessor()
        : AudioProcessor (BusesProperties()
              .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
              .withOutput ("Output", juce::AudioChannelSet::stereo(), true)) {}

    void prepareToPlay (double sampleRate, int) override;
    void releaseResources() override {}
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Maqam Assistant"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override {}
    void setStateInformation (const void*, int) override {}

    // "Listen" mode: records up to 30 s of the track's audio (audio passes through untouched)
    std::atomic<bool> listening { false };
    void clearCapture() { numCaptured = 0; }
    std::vector<float> getCapture() const
    {
        return std::vector<float> (captured.begin(), captured.begin() + numCaptured.load());
    }
    double getSampleRateHz() const { return sampleRate; }

private:
    std::vector<float> captured;
    std::atomic<int> numCaptured { 0 };
    double sampleRate = 44100.0;
};
