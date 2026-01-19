#pragma once

#include "PluginProcessor.h"

class CloudsReverbEditor : public juce::AudioProcessorEditor
{
public:
    explicit CloudsReverbEditor(CloudsReverbProcessor&);
    ~CloudsReverbEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    CloudsReverbProcessor& processorRef;

    juce::Slider amountSlider;
    juce::Slider inputGainSlider;
    juce::Slider timeSlider;
    juce::Slider diffusionSlider;
    juce::Slider lpSlider;

    juce::Label amountLabel;
    juce::Label inputGainLabel;
    juce::Label timeLabel;
    juce::Label diffusionLabel;
    juce::Label lpLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> amountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> timeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> diffusionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lpAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CloudsReverbEditor)
};
