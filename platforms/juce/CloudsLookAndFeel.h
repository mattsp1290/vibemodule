#pragma once

#include <JuceHeader.h>

// Custom LookAndFeel inspired by Mutable Instruments aesthetic
// Clean, minimalist design with teal accent colors
class CloudsLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CloudsLookAndFeel()
    {
        // Color scheme inspired by Mutable Instruments
        setColour(juce::Slider::rotarySliderFillColourId, kAccentColor);
        setColour(juce::Slider::rotarySliderOutlineColourId, kDarkGrey);
        setColour(juce::Slider::thumbColourId, kAccentColor);
        setColour(juce::Slider::textBoxTextColourId, kLightGrey);
        setColour(juce::Slider::textBoxBackgroundColourId, kDarkGrey);
        setColour(juce::Slider::textBoxOutlineColourId, kMediumGrey);

        setColour(juce::Label::textColourId, kLightGrey);

        setColour(juce::ComboBox::backgroundColourId, kDarkGrey);
        setColour(juce::ComboBox::textColourId, kLightGrey);
        setColour(juce::ComboBox::outlineColourId, kMediumGrey);
        setColour(juce::ComboBox::arrowColourId, kAccentColor);

        setColour(juce::PopupMenu::backgroundColourId, kPanelBackground);
        setColour(juce::PopupMenu::textColourId, kLightGrey);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, kAccentColor);
        setColour(juce::PopupMenu::highlightedTextColourId, kBackground);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override
    {
        const float radius = static_cast<float>(juce::jmin(width / 2, height / 2)) - 4.0f;
        const float centreX = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
        const float centreY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
        const float rx = centreX - radius;
        const float ry = centreY - radius;
        const float rw = radius * 2.0f;
        const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // Background circle
        g.setColour(kDarkGrey);
        g.fillEllipse(rx, ry, rw, rw);

        // Outer ring
        g.setColour(kMediumGrey);
        g.drawEllipse(rx, ry, rw, rw, 2.0f);

        // Value arc
        juce::Path valueArc;
        valueArc.addCentredArc(centreX, centreY, radius - 4.0f, radius - 4.0f,
                                0.0f, rotaryStartAngle, angle, true);
        g.setColour(kAccentColor);
        g.strokePath(valueArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));

        // Pointer
        juce::Path pointer;
        const float pointerLength = radius * 0.6f;
        const float pointerThickness = 3.0f;
        pointer.addRectangle(-pointerThickness * 0.5f, -radius + 6.0f, pointerThickness, pointerLength);
        pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

        g.setColour(kLightGrey);
        g.fillPath(pointer);

        // Center dot
        const float dotRadius = 4.0f;
        g.setColour(kAccentColor);
        g.fillEllipse(centreX - dotRadius, centreY - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);
    }

    void drawLabel(juce::Graphics& g, juce::Label& label) override
    {
        g.fillAll(label.findColour(juce::Label::backgroundColourId));

        if (!label.isBeingEdited()) {
            const juce::Font font(getLabelFont(label));
            g.setColour(kLightGrey);
            g.setFont(font);

            auto textArea = label.getLocalBounds();
            g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                             juce::jmax(1, static_cast<int>(static_cast<float>(textArea.getHeight()) / font.getHeight())),
                             label.getMinimumHorizontalScale());
        }
    }

    juce::Font getLabelFont(juce::Label&) override
    {
        return juce::Font(kFontName, 12.0f, juce::Font::plain);
    }

    juce::Font getTextButtonFont(juce::TextButton&, int) override
    {
        return juce::Font(kFontName, 13.0f, juce::Font::plain);
    }

    juce::Font getComboBoxFont(juce::ComboBox&) override
    {
        return juce::Font(kFontName, 13.0f, juce::Font::plain);
    }

    // Color constants
    static constexpr juce::uint32 kBackgroundValue = 0xff1a1a1a;
    static constexpr juce::uint32 kPanelBackgroundValue = 0xff2d2d2d;
    static constexpr juce::uint32 kDarkGreyValue = 0xff3d3d3d;
    static constexpr juce::uint32 kMediumGreyValue = 0xff5a5a5a;
    static constexpr juce::uint32 kLightGreyValue = 0xffc0c0c0;
    static constexpr juce::uint32 kAccentColorValue = 0xff00b4a0;  // Teal/cyan

    inline static const juce::Colour kBackground{kBackgroundValue};
    inline static const juce::Colour kPanelBackground{kPanelBackgroundValue};
    inline static const juce::Colour kDarkGrey{kDarkGreyValue};
    inline static const juce::Colour kMediumGrey{kMediumGreyValue};
    inline static const juce::Colour kLightGrey{kLightGreyValue};
    inline static const juce::Colour kAccentColor{kAccentColorValue};

    inline static const juce::String kFontName{"Arial"};
};
