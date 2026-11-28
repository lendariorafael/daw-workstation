#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// OPALO reference palette: graphite surfaces with restrained purple and spectral accents.
namespace OpaloDesign
{
    inline const juce::Colour background { (juce::uint32) 0xff070910 };
    inline const juce::Colour surface0   { (juce::uint32) 0xff10131c };
    inline const juce::Colour surface1   { (juce::uint32) 0xff171d2a };
    inline const juce::Colour surface2   { (juce::uint32) 0xff222b3b };
    inline const juce::Colour border     { (juce::uint32) 0xff303b50 };

    inline const juce::Colour accent     { (juce::uint32) 0xff9a42ff };
    inline const juce::Colour accentDim  { (juce::uint32) 0xff54218f };
    inline const juce::Colour accentBlue { (juce::uint32) 0xff27b8ff };
    inline const juce::Colour accentCyan { (juce::uint32) 0xff18d6e8 };
    inline const juce::Colour accentGreen{ (juce::uint32) 0xff43d98b };
    inline const juce::Colour accentGold { (juce::uint32) 0xffffbd48 };
    inline const juce::Colour text       { (juce::uint32) 0xffedf1fa };
    inline const juce::Colour textMuted  { (juce::uint32) 0xff8995aa };
    inline const juce::Colour danger     { (juce::uint32) 0xffff526d };

    constexpr float radiusSmall  = 4.0f;
    constexpr float radiusMedium = 6.0f;
    constexpr float radiusLarge  = 10.0f;
    constexpr int space1 = 4;
    constexpr int space2 = 8;
    constexpr int space3 = 12;
    constexpr int space4 = 16;
    constexpr float fontSmall  = 11.0f;
    constexpr float fontBody   = 13.0f;
    constexpr float fontTitle  = 15.0f;
}
