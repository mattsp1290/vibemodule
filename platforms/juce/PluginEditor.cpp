#include "PluginEditor.h"

CloudsReverbEditor::CloudsReverbEditor(CloudsReverbProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    auto setupSlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& text)
    {
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        addAndMakeVisible(slider);

        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);
    };

    setupSlider(amountSlider, amountLabel, "Amount");
    setupSlider(inputGainSlider, inputGainLabel, "Input Gain");
    setupSlider(timeSlider, timeLabel, "Time");
    setupSlider(diffusionSlider, diffusionLabel, "Diffusion");
    setupSlider(lpSlider, lpLabel, "LP Filter");

    auto& vts = processorRef.getValueTreeState();
    amountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        vts, "amount", amountSlider);
    inputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        vts, "input_gain", inputGainSlider);
    timeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        vts, "time", timeSlider);
    diffusionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        vts, "diffusion", diffusionSlider);
    lpAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        vts, "lp", lpSlider);

    setSize(500, 200);
}

CloudsReverbEditor::~CloudsReverbEditor() = default;

void CloudsReverbEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2d2d2d));
    g.setColour(juce::Colours::white);
    g.setFont(18.0f);
    g.drawFittedText("Clouds Reverb", getLocalBounds().removeFromTop(30), juce::Justification::centred, 1);
}

void CloudsReverbEditor::resized()
{
    auto area = getLocalBounds().reduced(10);
    area.removeFromTop(30);

    const int knobWidth = area.getWidth() / 5;
    const int knobHeight = area.getHeight();
    const int labelHeight = 20;

    auto setupKnob = [&](juce::Slider& slider, juce::Label& label)
    {
        auto knobArea = area.removeFromLeft(knobWidth);
        label.setBounds(knobArea.removeFromTop(labelHeight));
        slider.setBounds(knobArea);
    };

    setupKnob(amountSlider, amountLabel);
    setupKnob(inputGainSlider, inputGainLabel);
    setupKnob(timeSlider, timeLabel);
    setupKnob(diffusionSlider, diffusionLabel);
    setupKnob(lpSlider, lpLabel);
}
