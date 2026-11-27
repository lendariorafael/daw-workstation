#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Identidade visual Opalo: superfícies índigo profundas, violeta iridescente e ciano.
namespace OpaloDesign
{
    inline const juce::Colour background { (juce::uint32) 0xff0b0d1d };
    inline const juce::Colour surface0   { (juce::uint32) 0xff12152b };
    inline const juce::Colour surface1   { (juce::uint32) 0xff1b2040 };
    inline const juce::Colour surface2   { (juce::uint32) 0xff282e55 };
    inline const juce::Colour border     { (juce::uint32) 0xff393d72 };
    inline const juce::Colour accent     { (juce::uint32) 0xff963cff };
    inline const juce::Colour accentDim  { (juce::uint32) 0xff5d25b5 };
    inline const juce::Colour secondary  { (juce::uint32) 0xff16d6cf };
    inline const juce::Colour text       { (juce::uint32) 0xfff1f0ff };
    inline const juce::Colour textMuted  { (juce::uint32) 0xffa6aacb };
    inline const juce::Colour danger     { (juce::uint32) 0xffff5277 };
    constexpr float radiusSmall = 5.0f;
    constexpr float radiusMedium = 8.0f;
    constexpr float radiusLarge = 12.0f;
    constexpr int space1 = 4;
    constexpr int space2 = 8;
    constexpr int space3 = 12;
    constexpr int space4 = 16;
    constexpr float fontSmall = 11.0f;
    constexpr float fontBody = 13.0f;
    constexpr float fontTitle = 15.0f;
}
