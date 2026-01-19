#include "PluginEditor.h"

CloudsReverbEditor::CloudsReverbEditor(CloudsReverbProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setLookAndFeel(&customLookAndFeel);

    auto setupSlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& text)
    {
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        slider.setColour(juce::Slider::textBoxTextColourId, CloudsLookAndFeel::kLightGrey);
        slider.setColour(juce::Slider::textBoxBackgroundColourId, CloudsLookAndFeel::kDarkGrey);
        slider.setColour(juce::Slider::textBoxOutlineColourId, CloudsLookAndFeel::kMediumGrey);
        addAndMakeVisible(slider);

        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, CloudsLookAndFeel::kLightGrey);
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

    setSize(550, 230);
}

CloudsReverbEditor::~CloudsReverbEditor()
{
    setLookAndFeel(nullptr);
}

void CloudsReverbEditor::paint(juce::Graphics& g)
{
    // Background gradient
    g.fillAll(CloudsLookAndFeel::kBackground);

    // Header area
    auto headerArea = getLocalBounds().removeFromTop(40);
    g.setColour(CloudsLookAndFeel::kPanelBackground);
    g.fillRect(headerArea);

    // Title
    g.setColour(CloudsLookAndFeel::kAccentColor);
    g.setFont(juce::Font(CloudsLookAndFeel::kFontName, 20.0f, juce::Font::bold));
    g.drawFittedText("CLOUDS REVERB", headerArea, juce::Justification::centred, 1);

    // Subtle border at bottom of header
    g.setColour(CloudsLookAndFeel::kMediumGrey);
    g.drawLine(0.0f, 40.0f, static_cast<float>(getWidth()), 40.0f, 1.0f);
}

void CloudsReverbEditor::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(50);  // Header space
    area = area.reduced(15);

    const int knobWidth = area.getWidth() / 5;
    const int labelHeight = 22;

    auto setupKnob = [&](juce::Slider& slider, juce::Label& label)
    {
        auto knobArea = area.removeFromLeft(knobWidth);
        label.setBounds(knobArea.removeFromTop(labelHeight));
        slider.setBounds(knobArea.reduced(5, 0));
    };

    setupKnob(amountSlider, amountLabel);
    setupKnob(inputGainSlider, inputGainLabel);
    setupKnob(timeSlider, timeLabel);
    setupKnob(diffusionSlider, diffusionLabel);
    setupKnob(lpSlider, lpLabel);
}
