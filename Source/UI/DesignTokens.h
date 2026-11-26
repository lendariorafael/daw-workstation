#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Tokens de design do Opalo. Todas as cores, raios e espaçamentos vêm daqui,
// para a interface ficar coerente e fácil de ajustar num único sítio.
namespace OpaloDesign
{
    // Superfícies em camadas: quanto mais clara, mais "perto" do utilizador.
    inline const juce::Colour background { (juce::uint32) 0xff202326 };
    inline const juce::Colour surface0   { (juce::uint32) 0xff292d30 };
    inline const juce::Colour surface1   { (juce::uint32) 0xff343a3e };
    inline const juce::Colour surface2   { (juce::uint32) 0xff41484c };
    inline const juce::Colour border     { (juce::uint32) 0xff50595e };

    inline const juce::Colour accent     { (juce::uint32) 0xff62cdbb };
    inline const juce::Colour accentDim  { (juce::uint32) 0xff367f76 };

    inline const juce::Colour text       { (juce::uint32) 0xffe5e9e8 };
    inline const juce::Colour textMuted  { (juce::uint32) 0xffaab4b2 };
    inline const juce::Colour danger     { (juce::uint32) 0xffff6b6b };

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
