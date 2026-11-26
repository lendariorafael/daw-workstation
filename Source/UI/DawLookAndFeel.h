#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Identidade visual: escuro, plano e com um único acento.
class DawLookAndFeel : public juce::LookAndFeel_V4
{
public:
    static inline const juce::Colour background { (juce::uint32) 0xff0f1115 };
    static inline const juce::Colour panel      { (juce::uint32) 0xff1a1e26 };
    static inline const juce::Colour accent     { (juce::uint32) 0xff00d1b2 };
    static inline const juce::Colour text       { (juce::uint32) 0xffe6e9ef };
    static inline const juce::Colour muted      { (juce::uint32) 0xff7d8590 };

    DawLookAndFeel()
    {
        setColour(juce::ResizableWindow::backgroundColourId, background);
        setColour(juce::TextButton::buttonColourId, panel);
        setColour(juce::TextButton::buttonOnColourId, accent);
        setColour(juce::TextButton::textColourOffId, text);
        setColour(juce::TextButton::textColourOnId, background);
        setColour(juce::Slider::thumbColourId, accent);
        setColour(juce::Slider::trackColourId, accent.withAlpha(0.55f));
        setColour(juce::Slider::backgroundColourId, panel);
        setColour(juce::Slider::textBoxTextColourId, text);
        setColour(juce::Slider::textBoxBackgroundColourId, panel);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::Label::textColourId, text);
    }
};
