#pragma once

#include <JuceHeader.h>
#include <clouds/clouds_reverb.h>

struct ReverbPreset {
    juce::String name;
    float amount;
    float inputGain;
    float time;
    float diffusion;
    float lp;
};

class CloudsReverbProcessor : public juce::AudioProcessor,
                               public juce::AudioProcessorValueTreeState::Listener
{
public:
    CloudsReverbProcessor();
    ~CloudsReverbProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    static const std::vector<ReverbPreset>& getFactoryPresets();

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void parameterChanged(const juce::String& parameterID, float newValue) override;

    juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters; }

private:
    juce::AudioProcessorValueTreeState parameters;
    clouds::CloudsReverb reverb;

    std::atomic<float>* amountParam = nullptr;
    std::atomic<float>* inputGainParam = nullptr;
    std::atomic<float>* timeParam = nullptr;
    std::atomic<float>* diffusionParam = nullptr;
    std::atomic<float>* lpParam = nullptr;

    // Smoothed parameter values for zipper-noise-free transitions
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedAmount;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedInputGain;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedTime;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedDiffusion;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedLp;

    static constexpr double kSmoothingTimeSeconds = 0.02;  // 20ms smoothing

    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CloudsReverbProcessor)
};
